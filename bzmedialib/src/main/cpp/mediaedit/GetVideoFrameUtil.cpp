//
/**
 * Created by bookzhan on 2021-10-16 12:49.
 *description:
 */
//

#include <common/BZLogUtil.h>
#include <glutils/EGLContextUtil.h>
#include <common/bz_time.h>
#include "GetVideoFrameUtil.h"
#include "VideoUtil.h"
#include <libyuv.h>

int GetVideoFrameUtil::getVideoFrame(const char *videoPath, int imageCount, int maxWidth,
                                     int64_t startTime, int64_t endTime, int64_t callBackHandle,
                                     void (*callBack)(int64_t, int, void *, int, int)) {
    if (nullptr == videoPath || imageCount <= 0 || maxWidth <= 0 || nullptr == callBack) {
        BZLogUtil::logE(
                "nullptr==videoPath||imageCount <= 0 || maxWidth <= 0 || nullptr == callBack");
        return -1;
    }
    BZLogUtil::logD(
            "getVideoFrame videoPath=%s,imageCount=%d,maxWidth=%d,startTime=%lld,endTime=%lld",
            videoPath, imageCount, maxWidth, startTime, endTime);
    int ret = initContext(videoPath, imageCount, maxWidth, startTime, endTime);
    if (ret < 0) {
        BZLogUtil::logE("initContext fail videoPath=%s", videoPath);
        return -1;
    }

    EGLContextUtil *eglContextUtilNative = new EGLContextUtil();
    eglContextUtilNative->initEGLContext(outWidth, outHeight);

    AVFrame *videoFrameOut = VideoUtil::allocVideoFrame(AV_PIX_FMT_RGBA,
                                                        outWidth, outHeight);
    AVFrame *videoFrame = av_frame_alloc();
    int getFrameCount = 0;
    AVPacket *decode_pkt = av_packet_alloc();
    int decodeCount = 0;
    while (true) {
        if (getFrameCount >= imageCount) {
            BZLogUtil::logD("getVideoFrame end");
            break;
        }
        av_init_packet(decode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0) {
            break;
        }
        AVStream *in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];

        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            int64_t timeTemp = getCurrentTime();
            avcodec_send_packet(in_stream->codec, decode_pkt);
            ret = avcodec_receive_frame(in_stream->codec, videoFrame);
            av_packet_unref(decode_pkt);
            decodeCount++;
            if (ret < 0) {
                BZLogUtil::logE("avcodec_receive_frame fail");
                av_frame_unref(videoFrame);
                continue;
            }
            if (!targetVideoTimeList.empty() && videoFrame->pts >= targetVideoTimeList.front()) {
                dealWidthAVFrame(videoFrame, videoFrameOut);
                callBack(callBackHandle, getFrameCount, videoFrameOut->data[0],
                         videoFrameOut->width,
                         videoFrameOut->height);
                ++getFrameCount;
                targetVideoTimeList.pop_front();
                if (!targetVideoTimeList.empty()) {
                    //seek到下一个时间节点,节省时间
                    int64_t seekPts = getSeekPts(videoFrame->pts);
                    if (seekPts > 0) {
                        av_seek_frame(in_fmt_ctx, videoIndex, seekPts, AVSEEK_FLAG_BACKWARD);
                    }
                }
                BZLogUtil::logV("decode time cost=%lld", getCurrentTime() - timeTemp);
            }
        }
    }
    if (getFrameCount < imageCount && nullptr != input_video_stream) {
        BZLogUtil::logD("start flush buffer");
        //刷新缓存
        while (true) {
            if (getFrameCount >= imageCount) {
                BZLogUtil::logD("getVideoFrame flush buffer end");
                break;
            }
            av_init_packet(decode_pkt);
            decode_pkt->data = nullptr;
            decode_pkt->size = 0;
            avcodec_send_packet(input_video_stream->codec, decode_pkt);
            ret = avcodec_receive_frame(input_video_stream->codec, videoFrame);
            av_packet_unref(decode_pkt);
            BZLogUtil::logD("--- flush buffer ---");
            if (ret < 0) {
                av_frame_unref(videoFrame);
                break;
            } else {
                if (!targetVideoTimeList.empty() &&
                    videoFrame->pts >= targetVideoTimeList.front()) {
                    dealWidthAVFrame(videoFrame, videoFrameOut);
                    callBack(callBackHandle, getFrameCount, videoFrameOut->data[0],
                             videoFrameOut->width,
                             videoFrameOut->height);
                    ++getFrameCount;
                    targetVideoTimeList.pop_front();
                }
            }
        }
    }

    BZLogUtil::logD("decodeCount=%d imageCount=%d", decodeCount, imageCount);
    if (nullptr != videoFrame) {
        av_frame_free(&videoFrame);
        videoFrame = nullptr;
    }
    if (nullptr != videoFrameOut) {
        av_frame_free(&videoFrameOut);
        videoFrameOut = nullptr;
    }
    releaseContext();
    eglContextUtilNative->releaseEGLContext();
    delete (eglContextUtilNative);
    return 0;
}

AVFrame *GetVideoFrameUtil::getVideoFrameAtTime(const char *videoPath, int64_t time, int maxWidth) {
    if (nullptr == videoPath || maxWidth <= 0) {
        BZLogUtil::logE("nullptr == videoPath || maxWidth <= 0");
        return nullptr;
    }
    int ret = initContext(videoPath, 1, maxWidth, time, -1);
    if (ret < 0) {
        BZLogUtil::logE("initContext fail videoPath=%s", videoPath);
        return nullptr;
    }
    EGLContextUtil *eglContextUtilNative = new EGLContextUtil();
    eglContextUtilNative->initEGLContext(outWidth, outHeight);

    AVFrame *videoFrameOut = VideoUtil::allocVideoFrame(AV_PIX_FMT_RGBA,
                                                        outWidth, outHeight);
    AVFrame *videoFrame = av_frame_alloc();
    AVPacket *decode_pkt = av_packet_alloc();
    int decodeCount = 0;
    while (true) {
        av_init_packet(decode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0) {
            break;
        }
        AVStream *in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];

        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            int64_t timeTemp = getCurrentTime();
            avcodec_send_packet(in_stream->codec, decode_pkt);
            ret = avcodec_receive_frame(in_stream->codec, videoFrame);
            decodeCount++;
            if (ret < 0) {
                av_packet_unref(decode_pkt);
                BZLogUtil::logE("avcodec_receive_frame fail");
                av_frame_unref(videoFrame);
                continue;
            }
            if (!targetVideoTimeList.empty() && videoFrame->pts >= targetVideoTimeList.front()) {
                dealWidthAVFrame(videoFrame, videoFrameOut);
                BZLogUtil::logV("decode time cost=%lld", getCurrentTime() - timeTemp);
                break;
            }
        }
        av_packet_unref(decode_pkt);
    }
    BZLogUtil::logD("decodeCount=%d", decodeCount);
    if (nullptr != videoFrame) {
        av_frame_free(&videoFrame);
        videoFrame = nullptr;
    }
    releaseContext();
    eglContextUtilNative->releaseEGLContext();
    delete (eglContextUtilNative);
    return videoFrameOut;
}


int
GetVideoFrameUtil::initContext(const char *path, int imageCount, int maxWidth, int64_t startTime,
                               int64_t endTime) {
    int ret;
    if ((ret = VideoUtil::openInputFileForSoft(path,
                                               &in_fmt_ctx, true, false)) < 0) {
        BZLogUtil::logE("openInputFileForSoft fail %s", path);
        in_fmt_ctx = nullptr;
        return ret;
    }

    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = in_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            input_video_stream = avStream;
            width = avStream->codecpar->width;
            height = avStream->codecpar->height;
            //保证不是单数
            outWidth = std::min(maxWidth, width) / 2 * 2;
            outHeight = outWidth * height / width / 2 * 2;
            AVDictionaryEntry *entry = av_dict_get(avStream->metadata, "rotate", nullptr,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (nullptr != entry) {
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
    if (nullptr == input_video_stream || videoIndex < 0 || width <= 0 || height <= 0) {
        BZLogUtil::logE(
                "nullptr == input_video_stream || videoIndex < 0 || width <= 0 || height <= 0");
        return -1;
    }
    duration = in_fmt_ctx->duration / 1000;
    //统计关键帧数量并计算要解码的帧的时间戳
    list<int64_t> allVideoTimeList;
    int64_t videoKeyFrameCount = 0;
    int64_t totalFrameCount = 0;
    int64_t lastPts = 0;
    AVPacket *decode_pkt = av_packet_alloc();
    while (true) {
        av_init_packet(decode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0) {
            break;
        }
        AVStream *in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            int64_t temp = decode_pkt->pts * 1000 * in_stream->time_base.num /
                           in_stream->time_base.den;
            lastPts = decode_pkt->pts;
            if (startTime >= 0 && temp < startTime) {
                av_packet_unref(decode_pkt);
                continue;
            }
            if (endTime >= 0 && endTime - startTime > 0 && temp > endTime) {
                break;
            }
            if (decode_pkt->flags == AV_PKT_FLAG_KEY) {
                videoKeyFrameCount++;
                keyVideoTimeList.push_back(decode_pkt->pts);
            }
            totalFrameCount++;
            allVideoTimeList.push_back(decode_pkt->pts);
        }
        av_packet_unref(decode_pkt);
    }
    //到最后了一帧也不满足条件,取最后一帧
    if (totalFrameCount <= 0) {
        totalFrameCount++;
        allVideoTimeList.push_back(lastPts);
    }
    BZLogUtil::logD("video startTime=%d endTime=%d", startTime, endTime);
    BZLogUtil::logD("video width=%d height=%d", width, height);
    BZLogUtil::logD("imageCount=%d videoKeyFrameCount=%lld totalFrameCount=%lld", imageCount,
                    videoKeyFrameCount,
                    totalFrameCount);
    av_packet_free(&decode_pkt);

    if (imageCount <= videoKeyFrameCount) {
        BZLogUtil::logD("use keyVideoTimeList");
        for (int i = 0; i < imageCount; ++i) {
            int64_t index = videoKeyFrameCount / imageCount * i;
            targetVideoTimeList.push_back(getListValue(keyVideoTimeList, index));
        }
    } else {
        BZLogUtil::logW("use allVideoTimeList");
        for (int i = 0; i < imageCount; ++i) {
            int64_t index = totalFrameCount / imageCount * i;
            targetVideoTimeList.push_back(getListValue(allVideoTimeList, index));
        }
    }
    av_seek_frame(in_fmt_ctx, videoIndex, targetVideoTimeList.front(), AVSEEK_FLAG_BACKWARD);
    return 0;
}

int64_t GetVideoFrameUtil::getSeekPts(int64_t currentFramePts) {
    if (targetVideoTimeList.empty() || keyVideoTimeList.empty()) {
        return -1;
    }
    auto iteratorPts = keyVideoTimeList.begin();
    int64_t needKeyPts = 0;
    while (iteratorPts != keyVideoTimeList.end()) {
        if (targetVideoTimeList.front() >= *iteratorPts) {
            needKeyPts = *iteratorPts;
        }
        iteratorPts++;
    }
    bool neeSeek = currentFramePts < needKeyPts;
    BZLogUtil::logV("currentFramePts=%lld,targetVideoTime=%lld,needKeyPts=%lld,neeSeek=%d",
                    currentFramePts, targetVideoTimeList.front(), needKeyPts, neeSeek);
    if (neeSeek) {
        return needKeyPts;
    }
    return -1;
}

void GetVideoFrameUtil::dealWidthAVFrame(AVFrame *srcAVFrame, AVFrame *targetAVFrame) {
    if (nullptr == srcAVFrame || nullptr == srcAVFrame->data[0] ||
        nullptr == targetAVFrame || nullptr == targetAVFrame->data[0]) {
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
    if (avFrameDrawProgram->getYUVType() == YUV_TYPE_YUV420P &&
        srcAVFrame->format != AV_PIX_FMT_YUV420P) {
        if (nullptr == sws_video_to_YUV) {
            sws_video_to_YUV = sws_getContext(srcAVFrame->width, srcAVFrame->height,
                                              (AVPixelFormat) srcAVFrame->format,
                                              srcAVFrame->width, srcAVFrame->height,
                                              AV_PIX_FMT_YUV420P,
                                              1, nullptr, nullptr, nullptr);
            yuv420Frame = VideoUtil::allocVideoFrame(AV_PIX_FMT_YUV420P,
                                                     srcAVFrame->width,
                                                     srcAVFrame->height);
        }
        sws_scale(sws_video_to_YUV,
                  (const uint8_t *const *) srcAVFrame->data,
                  srcAVFrame->linesize, 0, yuv420Frame->height,
                  yuv420Frame->data,
                  yuv420Frame->linesize);
        srcAVFrame = yuv420Frame;
    }
    if (nullptr == outFrameBufferUtils) {
        outFrameBufferUtils = new FrameBufferUtils();
        outFrameBufferUtils->initFrameBuffer(outWidth, outHeight);
        outFrameBufferUtils->bindFrameBuffer();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        outFrameBufferUtils->unbindFrameBuffer();
    }
    if (nullptr == baseProgram) {
        baseProgram = new BaseProgram();
    }
    if (nullptr == srcFrameBufferUtils) {
        srcFrameBufferUtils = new FrameBufferUtils();
        srcFrameBufferUtils->initFrameBuffer(width, height);
        srcFrameBufferUtils->bindFrameBuffer();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        srcFrameBufferUtils->unbindFrameBuffer();
    }
    srcFrameBufferUtils->bindFrameBuffer();
    glViewport(0, 0, srcAVFrame->linesize[0], height);
    avFrameDrawProgram->draw(srcAVFrame);
    srcFrameBufferUtils->unbindFrameBuffer();

    outFrameBufferUtils->bindFrameBuffer();
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, outWidth, outHeight);
    baseProgram->setTextureId(srcFrameBufferUtils->getBuffersTextureId());
    baseProgram->draw();
    glReadPixels(0, 0, outWidth, outHeight, GL_RGBA, GL_UNSIGNED_BYTE, targetAVFrame->data[0]);
    outFrameBufferUtils->unbindFrameBuffer();
}

void GetVideoFrameUtil::releaseContext() {
    if (nullptr != in_fmt_ctx) {
        if (videoIndex >= 0) {
            //只需要关闭VideoCodec
            avcodec_close(in_fmt_ctx->streams[videoIndex]->codec);
        }
        avformat_close_input(&in_fmt_ctx);
        in_fmt_ctx = nullptr;
    }
    if (nullptr != srcFrameBufferUtils) {
        srcFrameBufferUtils->releaseFrameBuffer();
        delete (srcFrameBufferUtils);
        srcFrameBufferUtils = nullptr;
    }
    if (nullptr != outFrameBufferUtils) {
        outFrameBufferUtils->releaseFrameBuffer();
        delete (outFrameBufferUtils);
        outFrameBufferUtils = nullptr;
    }
    if (nullptr != avFrameDrawProgram) {
        avFrameDrawProgram->releaseResource();
        delete (avFrameDrawProgram);
        avFrameDrawProgram = nullptr;
    }
    if (nullptr != yuv420Frame) {
        av_frame_free(&yuv420Frame);
        yuv420Frame = nullptr;
    }
    if (nullptr != sws_video_to_YUV) {
        sws_freeContext(sws_video_to_YUV);
        sws_video_to_YUV = nullptr;
    }
}

int64_t GetVideoFrameUtil::getListValue(std::list<int64_t> ls, int64_t index) {
    std::list<int64_t>::iterator listIterator;
    listIterator = ls.begin();
    int64_t result = *listIterator;
    for (int64_t i = 0; i < index + 1; ++i) {
        if (listIterator == ls.end()) {
            break;
        }
        result = *listIterator;
        listIterator++;
    }
    return result;
}
