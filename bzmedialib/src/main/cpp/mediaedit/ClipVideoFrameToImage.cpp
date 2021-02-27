
/**
 * Created by zhandalin on 2017-06-12 17:27.
 * 说明:
 */

#include "ClipVideoFrameToImage.h"
#include "glutils/FrameBufferUtils.h"
#include "../mediaedit/VideoUtil.h"
#include "BZLogUtil.h"
#include <common/GLUtil.h>
#include <glutils/EGLContextUtil.h>

int ClipVideoFrameToImage::getVideoFrame(int imageCount, int scale2Width) {
    if (imageCount <= 0 || scale2Width <= 0) {
        BZLogUtil::logE("imageCount<=0||scale2Width<=0");
        return -1;
    }
    int ret = 0, got_picture_ptr;
    AVPacket *decode_pkt = av_packet_alloc();
    av_init_packet(decode_pkt);
    int width = 480, height = 480, videoIndex = -1;
    int64_t seekStep = 1;

    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = in_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            input_video_stream = avStream;
            videoCodecContext = avStream->codec;
            width = avStream->codecpar->width;
            height = avStream->codecpar->height;
            //保证不是单数
            outWidth = std::min(scale2Width, width) / 2 * 2;
            outHeight = outWidth * height / width / 2 * 2;
            seekStep = (int64_t) (avStream->duration / imageCount);//*0.9是防止截图不够
            AVDictionaryEntry *entry = av_dict_get(avStream->metadata, "rotate", NULL,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (NULL != entry) {
                video_rotate = atoi(entry->value);
                if (video_rotate >= 360) {
                    video_rotate %= 360;
                }
            }
            BZLogUtil::logD("video_rotate=%d", video_rotate);
            if (video_rotate == 90 || video_rotate == 270) {
                int temp = outWidth;
                outWidth = outHeight;
                outHeight = temp;
            }
            break;
        }
    }
    if (videoIndex < 0) {
        BZLogUtil::logE("InputFile no video stream or is not AV_PIX_FMT_YUV420P");
        return -1;
    }
    EGLContextUtil *eglContextUtilNative = new EGLContextUtil();
    eglContextUtilNative->initEGLContext(outWidth, outHeight);

    AVFrame *videoFrameOut = VideoUtil::allocVideoFrame(AV_PIX_FMT_RGBA,
                                                        outWidth, outHeight);
    AVFrame *videoFrame = av_frame_alloc();
    AVStream *in_stream = NULL;
    int errorCount = 0, getFrameCount = 0, seekIndex = 0;
    while (true) {
        if (getFrameCount >= imageCount) {
            BZLogUtil::logD("getVideoFrame end");
            break;
        }
        av_init_packet(decode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0) {
            if (NULL == videoFrame->data[0]) {
                BZLogUtil::logE("最后一帧补齐 NULL==videoFrame->data[0]");
                return -1;
            }
            if (errorCount > 35) {
                BZLogUtil::logE("errorCount>30");
                return -1;
            }
            //数量不够拿最后一帧补齐
            BZLogUtil::logD("最后一帧补齐--index=%d", getFrameCount);
            if (NULL != outImageParentPath) {
                char finalImagePath[512] = {0};
                sprintf(finalImagePath, "%simage_%02d.jpg", outImageParentPath, getFrameCount);
                BZLogUtil::logV("index=%d--finalImagePath=%s", getFrameCount, finalImagePath);

                ret = saveFrame(finalImagePath, videoFrameOut);
                if (ret < 0) {
                    BZLogUtil::logE("error save frame");
                    return -1;
                }
                if (NULL != getImageFromVideoCallBack)
                    getImageFromVideoCallBack(callBackHandle, getFrameCount, finalImagePath);
            }

            if (NULL != getImageDataFromVideoCallBack) {
                getImageDataFromVideoCallBack(callBackHandle, getFrameCount, videoFrameOut->data[0],
                                              videoFrameOut->width,
                                              videoFrameOut->height);
            }
            ++getFrameCount;
            errorCount++;
            continue;
        }
        in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];

        if (in_stream->codec != NULL && in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            int64_t time = seekStep * (++seekIndex);
            av_seek_frame(in_fmt_ctx, videoIndex, time, AVSEEK_FLAG_BACKWARD);
//            BZLogUtil::logD("seek time=%lld, pts=%lld", time, decode_pkt->pts);
            ret = avcodec_decode_video2(in_stream->codec, videoFrame, &got_picture_ptr,
                                        decode_pkt);
            if (ret < 0) {
                BZLogUtil::logE("avcodec_decode_video2 fail");
                return -1;
            }
            if (got_picture_ptr <= 0) {
                errorCount++;
                BZLogUtil::logD("got_picture_ptr<=0");
                continue;
            }
            dealWidthAVFrame(videoFrame, videoFrameOut);
            if (NULL != outImageParentPath) {
                char finalImagePath[512] = {0};
                sprintf(finalImagePath, "%simage_%02d.jpg", outImageParentPath, getFrameCount);
                BZLogUtil::logV("index=%d--finalImagePath=%s", getFrameCount, finalImagePath);
                ret = saveFrame(finalImagePath, videoFrameOut);
                if (ret < 0) {
                    BZLogUtil::logE("saveFrame fail");
                    return -1;
                }
                if (NULL != getImageFromVideoCallBack)
                    getImageFromVideoCallBack(callBackHandle, getFrameCount, finalImagePath);

            }

            if (NULL != getImageDataFromVideoCallBack) {
                getImageDataFromVideoCallBack(callBackHandle, getFrameCount, videoFrameOut->data[0],
                                              videoFrameOut->width,
                                              videoFrameOut->height);
            }
            ++getFrameCount;
        }
    }

    if (NULL != videoFrame) {
        av_frame_free(&videoFrame);
        videoFrame = NULL;
    }
    if (NULL != videoFrameOut) {
        av_frame_free(&videoFrameOut);
        videoFrameOut = NULL;
    }

    //只需要关闭VideoCodec
    avcodec_close(in_fmt_ctx->streams[videoIndex]->codec);

    if (NULL != in_fmt_ctx) {
        avformat_close_input(&in_fmt_ctx);
        in_fmt_ctx = NULL;
    }
    if (nullptr != frameBufferUtils) {
        frameBufferUtils->releaseFrameBuffer();
        delete (frameBufferUtils);
        frameBufferUtils = nullptr;
    }
    if (nullptr != avFrameDrawProgram) {
        avFrameDrawProgram->releaseResource();
        delete (avFrameDrawProgram);
        avFrameDrawProgram = nullptr;
    }
    if (nullptr != sws_nv_2_yuv420p) {
        sws_freeContext(sws_nv_2_yuv420p);
        sws_nv_2_yuv420p = NULL;
    }
    if (NULL != nv_2_yuv420pFrame) {
        av_frame_free(&nv_2_yuv420pFrame);
        nv_2_yuv420pFrame = NULL;
    }
    eglContextUtilNative->releaseEGLContext();
    delete (eglContextUtilNative);
    return 0;
}

int
ClipVideoFrameToImage::clipVideoFrameToImage(const char *videoPath, const char *outImageParentPath,
                                             int imageCount, int scale2Width,
                                             int64_t callBackHandle,
                                             void (*getImageFromVideoCallBack)(
                                                     int64_t, int,
                                                     const char *)) {
    this->outImageParentPath = outImageParentPath;
    this->getImageFromVideoCallBack = getImageFromVideoCallBack;
    this->callBackHandle = callBackHandle;

    if (openInputFile(videoPath) < 0) {
        BZLogUtil::logE("openInputFile fail for %s", videoPath);
        return -1;
    }

    BZLogUtil::logD("ClipVideoFrameToImage success");
    return getVideoFrame(imageCount, scale2Width);
}

int ClipVideoFrameToImage::clipVideoFrameToImage(const char *videoPath, int imageCount,
                                                 int scale2Width, int64_t callBackHandle,
                                                 void (*getImageDataFromVideoCallBack)(int64_t, int,
                                                                                       void *,
                                                                                       int, int)) {
    this->getImageDataFromVideoCallBack = getImageDataFromVideoCallBack;
    this->callBackHandle = callBackHandle;

    if (openInputFile(videoPath) < 0) {
        BZLogUtil::logE("openInputFile fail for %s", videoPath);
        return -1;
    }
    return getVideoFrame(imageCount, scale2Width);
}


AVFrame *
ClipVideoFrameToImage::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height) {
    BZLogUtil::logD("alloc_picture");
    AVFrame *picture = NULL;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;
    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 1);
    if (ret < 0) {
        BZLogUtil::logE("Could not allocate frame data.\n");
    }
    return picture;
}

int
ClipVideoFrameToImage::saveFrame(const char *fileName, AVFrame *videoFrame) {
    if (NULL == fileName)
        return -1;

    if (videoFrame->format != AV_PIX_FMT_YUV420P) {
        if (NULL == sws_nv_2_yuv420p) {
            sws_nv_2_yuv420p = sws_getContext(videoFrame->width,
                                              videoFrame->height,
                                              (AVPixelFormat) videoFrame->format,
                                              videoFrame->width,
                                              videoFrame->height,
                                              AV_PIX_FMT_YUV420P,
                                              SWS_FAST_BILINEAR, NULL, NULL, NULL);
        }
        if (NULL == nv_2_yuv420pFrame) {
            nv_2_yuv420pFrame = VideoUtil::allocVideoFrame(AV_PIX_FMT_YUV420P, videoFrame->width,
                                                           videoFrame->height);
        }
        sws_scale(sws_nv_2_yuv420p,
                  (const uint8_t *const *) videoFrame->data,
                  videoFrame->linesize, 0, videoFrame->height,
                  nv_2_yuv420pFrame->data,
                  nv_2_yuv420pFrame->linesize);

        videoFrame = nv_2_yuv420pFrame;
    }

    int ret;
    avformat_alloc_output_context2(&out_fmt_ctx, NULL, NULL, fileName);
    if (NULL == out_fmt_ctx) {
        BZLogUtil::logE("avformat_alloc_output_context2 fail");
        return -1;
    }
    if (avio_open(&out_fmt_ctx->pb, fileName, AVIO_FLAG_READ_WRITE) < 0) {
        BZLogUtil::logE("输出文件打开失败");
        return -1;
    }
    AVStream *out_stream = avformat_new_stream(out_fmt_ctx, 0);
    if (out_stream == NULL) {
        BZLogUtil::logE("avformat_new_stream fail");
        return -1;
    }
    AVCodecContext *pCodecCtx;
    pCodecCtx = out_stream->codec;
    pCodecCtx->codec_id = out_fmt_ctx->oformat->video_codec;
    pCodecCtx->bit_rate = videoFrame->width * videoFrame->height;
    pCodecCtx->bit_rate_tolerance = videoFrame->width * videoFrame->height;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    //这个可以控制质量
    pCodecCtx->qmin = 1;
    pCodecCtx->qmax = 5;

    pCodecCtx->width = videoFrame->width;
    pCodecCtx->height = videoFrame->height;

    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    //输出格式信息
    av_dump_format(out_fmt_ctx, 0, fileName, 1);

    AVCodec *pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec) {
        BZLogUtil::logE("avcodec_find_encoder fail");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        BZLogUtil::logE("avcodec_open2 fail");
        return -1;
    }
    avformat_write_header(out_fmt_ctx, NULL);

    AVPacket pkt;
    int y_size = pCodecCtx->width * pCodecCtx->height;
    ret = av_new_packet(&pkt, y_size * 3);
    if (ret < 0) {
        BZLogUtil::logE("av_new_packet fail");
        return -1;
    }

    int got_picture = 0;
    //编码
    ret = avcodec_encode_video2(pCodecCtx, &pkt, videoFrame, &got_picture);
    if (ret < 0) {
        BZLogUtil::logE("avcodec_encode_video2 fail！\n");
        return -1;
    }
    if (got_picture == 1) {
        pkt.stream_index = out_stream->index;
        av_write_frame(out_fmt_ctx, &pkt);
    }
    av_free_packet(&pkt);

    if (NULL != out_stream) {
        avcodec_close(out_stream->codec);
    }
    //写文件尾
    av_write_trailer(out_fmt_ctx);

    if (NULL != out_fmt_ctx) {
        if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE))
            avio_closep(&out_fmt_ctx->pb);
        avformat_free_context(out_fmt_ctx);
        out_fmt_ctx = NULL;
    }
    return 0;
}

int
ClipVideoFrameToImage::clipVideoFrameToImageAtTime(const char *videoPath, const char *outImagePath,
                                                   int64_t time) {
    if (openInputFile(videoPath) < 0) {
        BZLogUtil::logE("openInputFile fail for %s", videoPath);
        return -1;
    }
    int ret = 0, got_picture_ptr;
    AVPacket *decode_pkt = av_packet_alloc();
    av_init_packet(decode_pkt);
    int videoIndex = -1;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = in_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            input_video_stream = avStream;
            videoCodecContext = avStream->codec;
            outWidth = avStream->codecpar->width / 2 * 2;
            outHeight = avStream->codecpar->height / 2 * 2;
            AVDictionaryEntry *entry = av_dict_get(avStream->metadata, "rotate", NULL,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (NULL != entry) {
                video_rotate = atoi(entry->value);
                if (video_rotate >= 360) {
                    video_rotate %= 360;
                }
            }
            BZLogUtil::logD("video_rotate=%d", video_rotate);
            if (video_rotate == 90 || video_rotate == 270) {
                int temp = outWidth;
                outWidth = outHeight;
                outHeight = temp;
            }
            break;
        }
    }
    if (videoIndex < 0) {
        BZLogUtil::logE("InputFile no video stream or is not AV_PIX_FMT_YUV420P");
        return -1;
    }
    EGLContextUtil *eglContextUtilNative = new EGLContextUtil();
    eglContextUtilNative->initEGLContext(outWidth, outHeight);

    AVFrame *videoFrameOut = VideoUtil::allocVideoFrame(AV_PIX_FMT_RGBA,
                                                        outWidth, outHeight);
    AVFrame *videoFrame = av_frame_alloc();
    AVStream *in_stream = NULL;

    av_seek_frame(in_fmt_ctx, videoIndex,
                  time * input_video_stream->time_base.den / input_video_stream->time_base.num /
                  1000, AVSEEK_FLAG_BACKWARD);
    while (true) {
        BZLogUtil::logD("duration=%lld seek time=%lld", input_video_stream->duration,
                        time * input_video_stream->time_base.den /
                        input_video_stream->time_base.num /
                        1000);
        av_init_packet(decode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0 && decode_pkt->stream_index == videoIndex) {
            time -= 1000;
            av_seek_frame(in_fmt_ctx, videoIndex,
                          time * input_video_stream->time_base.den /
                          input_video_stream->time_base.num /
                          1000, AVSEEK_FLAG_BACKWARD);
            BZLogUtil::logD("到最后了还是没有截取到图片--向前找");
            if (time < 0) {
                break;
            }
            continue;
        }
        in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];

        if (in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            ret = avcodec_decode_video2(in_stream->codec, videoFrame, &got_picture_ptr,
                                        decode_pkt);
            if (ret < 0) {
                BZLogUtil::logE("avcodec_decode_video2 fail");
                return -1;
            }
            if (got_picture_ptr <= 0) {
                BZLogUtil::logD("got_picture_ptr<=0");
                continue;
            }
            dealWidthAVFrame(videoFrame, videoFrameOut);
            ret = saveFrame(outImagePath, videoFrameOut);
            if (ret < 0) {
                BZLogUtil::logE("saveFrame fail");
                return -1;
            }
            break;
        }
    }

    if (NULL != videoFrame) {
        av_frame_free(&videoFrame);
        videoFrame = NULL;
    }
    //只需要关闭VideoCodec
    avcodec_close(in_fmt_ctx->streams[videoIndex]->codec);

    if (NULL != in_fmt_ctx) {
        avio_close(in_fmt_ctx->pb);
        avformat_free_context(in_fmt_ctx);
        in_fmt_ctx = NULL;
    }

    if (nullptr != frameBufferUtils) {
        frameBufferUtils->releaseFrameBuffer();
        delete (frameBufferUtils);
        frameBufferUtils = nullptr;
    }
    if (nullptr != avFrameDrawProgram) {
        avFrameDrawProgram->releaseResource();
        delete (avFrameDrawProgram);
        avFrameDrawProgram = nullptr;
    }
    if (nullptr != sws_nv_2_yuv420p) {
        sws_freeContext(sws_nv_2_yuv420p);
        sws_nv_2_yuv420p = NULL;
    }
    if (NULL != nv_2_yuv420pFrame) {
        av_frame_free(&nv_2_yuv420pFrame);
        nv_2_yuv420pFrame = NULL;
    }
    av_frame_free(&videoFrameOut);

    eglContextUtilNative->releaseEGLContext();
    delete (eglContextUtilNative);

    BZLogUtil::logD("clipVideoFrameToImageAtTime success");
    return 0;
}

AVFrame *ClipVideoFrameToImage::clipVideoFrameAtTime(const char *videoPath, int64_t time) {
    if (openInputFile(videoPath) < 0) {
        BZLogUtil::logE("openInputFile fail for %s", videoPath);
        return nullptr;
    }
    int ret = 0, got_picture_ptr;
    AVPacket *decode_pkt = av_packet_alloc();
    av_init_packet(decode_pkt);
    int videoIndex = -1;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = in_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            input_video_stream = avStream;
            videoCodecContext = avStream->codec;
            outWidth = avStream->codecpar->width / 2 * 2;
            outHeight = avStream->codecpar->height / 2 * 2;
            AVDictionaryEntry *entry = av_dict_get(avStream->metadata, "rotate", NULL,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (NULL != entry) {
                video_rotate = atoi(entry->value);
                if (video_rotate >= 360) {
                    video_rotate %= 360;
                }
            }
            BZLogUtil::logD("video_rotate=%d", video_rotate);
            if (video_rotate == 90 || video_rotate == 270) {
                int temp = outWidth;
                outWidth = outHeight;
                outHeight = temp;
            }
            break;
        }
    }
    if (videoIndex < 0) {
        BZLogUtil::logE("InputFile no video stream or is not AV_PIX_FMT_YUV420P");
        return nullptr;
    }
    EGLContextUtil *eglContextUtilNative = new EGLContextUtil();
    eglContextUtilNative->initEGLContext(outWidth, outHeight);

    AVFrame *videoFrameOut = VideoUtil::allocVideoFrame(AV_PIX_FMT_RGBA,
                                                        outWidth, outHeight);
    AVFrame *videoFrame = av_frame_alloc();
    AVStream *in_stream = NULL;

    av_seek_frame(in_fmt_ctx, videoIndex,
                  time * input_video_stream->time_base.den / input_video_stream->time_base.num /
                  1000, AVSEEK_FLAG_BACKWARD);
    while (true) {
        av_init_packet(decode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0 && decode_pkt->stream_index == videoIndex) {
            time -= 1000;
            av_seek_frame(in_fmt_ctx, videoIndex,
                          time * input_video_stream->time_base.den /
                          input_video_stream->time_base.num /
                          1000, AVSEEK_FLAG_BACKWARD);
            BZLogUtil::logD("到最后了还是没有截取到图片--向前找");
            if (time < 0) {
                break;
            }
            continue;
        }
        in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];

        if (in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            ret = avcodec_decode_video2(in_stream->codec, videoFrame, &got_picture_ptr,
                                        decode_pkt);
            if (ret < 0) {
                BZLogUtil::logE("avcodec_decode_video2 fail");
                break;
            }
            if (got_picture_ptr <= 0) {
                BZLogUtil::logD("got_picture_ptr<=0");
                continue;
            }
            dealWidthAVFrame(videoFrame, videoFrameOut);
            if (ret < 0) {
                BZLogUtil::logE("saveFrame fail");
                break;
            }
            break;
        }
    }

    if (NULL != videoFrame) {
        av_frame_free(&videoFrame);
        videoFrame = NULL;
    }
    av_packet_free(&decode_pkt);

    //只需要关闭VideoCodec
    avcodec_close(in_fmt_ctx->streams[videoIndex]->codec);

    if (NULL != in_fmt_ctx) {
        avio_close(in_fmt_ctx->pb);
        avformat_free_context(in_fmt_ctx);
        in_fmt_ctx = NULL;
    }

    if (nullptr != frameBufferUtils) {
        frameBufferUtils->releaseFrameBuffer();
        delete (frameBufferUtils);
        frameBufferUtils = nullptr;
    }
    if (nullptr != avFrameDrawProgram) {
        avFrameDrawProgram->releaseResource();
        delete (avFrameDrawProgram);
        avFrameDrawProgram = nullptr;
    }
    if (nullptr != sws_nv_2_yuv420p) {
        sws_freeContext(sws_nv_2_yuv420p);
        sws_nv_2_yuv420p = NULL;
    }
    if (NULL != nv_2_yuv420pFrame) {
        av_frame_free(&nv_2_yuv420pFrame);
        nv_2_yuv420pFrame = NULL;
    }

    eglContextUtilNative->releaseEGLContext();
    delete (eglContextUtilNative);

    BZLogUtil::logD("clipVideoFrameToImageAtTime success");
    return videoFrameOut;
}


int ClipVideoFrameToImage::openInputFile(const char *filename) {
    if (NULL == filename)
        return -1;
    int ret;
    unsigned int i;

    if ((ret = avformat_open_input(&in_fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    for (i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        //只打开视频
        if (stream->codec->codec_type != AVMEDIA_TYPE_VIDEO) {
            continue;
        }
        AVCodec *avCodec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (NULL == avCodec) {
            BZLogUtil::logE("can't find_decoder");
            return -1;
        }
        AVCodecContext *codec_ctx = avcodec_alloc_context3(avCodec);
        if (NULL == codec_ctx) {
            BZLogUtil::logE("can't avcodec_alloc_context3");
            return -1;
        }
        avcodec_parameters_to_context(codec_ctx, stream->codecpar);

        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
            || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                                NULL, NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
        stream->codec = codec_ctx;
    }
    av_dump_format(in_fmt_ctx, 0, filename, 0);
    return 0;
}

void ClipVideoFrameToImage::dealWidthAVFrame(AVFrame *srcAVFrame, AVFrame *targetAVFrame) {
    if (nullptr == srcAVFrame || nullptr == srcAVFrame->data || nullptr == srcAVFrame->data[0] ||
        nullptr == targetAVFrame || nullptr == targetAVFrame->data ||
        nullptr == targetAVFrame->data[0]) {
        BZLogUtil::logE("dealWidthAVFrame data is error");
        return;
    }

    if (nullptr == avFrameDrawProgram) {
        YUV_Type yuv_type;
        if (srcAVFrame->format == AV_PIX_FMT_NV12 ||
            srcAVFrame->format == AV_PIX_FMT_NV21) {
            yuv_type = YUV_TYPE_NV21_NV12;
        } else {
            yuv_type = YUV_TYPE_YUV420P;
        }
        avFrameDrawProgram = new AVFrameDrawProgram(yuv_type);
        avFrameDrawProgram->setRotation(video_rotate);
        //这个和其他的不同意估计是由于glReadPixels影响的
        if (video_rotate == 270 || video_rotate == 90) {
            avFrameDrawProgram->setFlip(true, true);
        } else {
            avFrameDrawProgram->setFlip(false, false);
        }
    }
    if (nullptr == frameBufferUtils) {
        frameBufferUtils = new FrameBufferUtils();
        frameBufferUtils->initFrameBuffer(outWidth, outHeight);
        frameBufferUtils->bindFrameBuffer();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        frameBufferUtils->unbindFrameBuffer();
    }
    frameBufferUtils->bindFrameBuffer();
    glViewport(0, 0, outWidth, outHeight);

    avFrameDrawProgram->draw(srcAVFrame);
    glReadPixels(0, 0, outWidth, outHeight, GL_RGBA, GL_UNSIGNED_BYTE, targetAVFrame->data[0]);
    frameBufferUtils->unbindFrameBuffer();
}
