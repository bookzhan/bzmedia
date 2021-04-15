//
/**
 * Created by bookzhan on 2019-02-21 10:25.
 * 说明:
 */
//
#include <common/BZLogUtil.h>
#include <common/bz_time.h>
#include "VideoFrameGetter.h"
#include "VideoUtil.h"

int VideoFrameGetter::init(const char *videoPath, bool userSoftDecode) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("nullptr==videoPath");
        return -1;
    }
    BZLogUtil::logD("VideoFrameGetter::init videoPath=%s,userSoftDecode=%d", videoPath,
                    userSoftDecode);
    this->userSoftDecode = userSoftDecode;
    int ret = 0;
    //软硬解
    if (userSoftDecode) {
        if ((ret = VideoUtil::openInputFileForSoft(videoPath, &in_fmt_ctx)) <
            0) {
            BZLogUtil::logE("openInputFile fail %s", videoPath);
            in_fmt_ctx = nullptr;
            release();
            return ret;
        }
    } else {
        if (VideoUtil::openInputFile(videoPath, &in_fmt_ctx) < 0) {
            in_fmt_ctx = nullptr;
            BZLogUtil::logE("openInputFile fail Switch to soft decoding %s", videoPath);
            if ((ret = VideoUtil::openInputFileForSoft(videoPath,
                                                       &in_fmt_ctx)) < 0) {
                BZLogUtil::logE("openInputFileForSoft fail %s", videoPath);
                in_fmt_ctx = nullptr;
                release();
                return ret;
            }
        }
    }
    if (nullptr == in_fmt_ctx) {
        BZLogUtil::logE("nullptr==in_fmt_ctx");
        release();
        return -1;
    }
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = in_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = avStream;
            videoWidth = avStream->codecpar->width;
            videoHeight = avStream->codecpar->height;
            videoCodecContext = avStream->codec;
            videoDuration = videoStream->duration * 1000 * videoStream->time_base.num /
                            videoStream->time_base.den;
            if (videoDuration <= 0) {
                int nb_frames = videoStream->nb_frames;
                if (nb_frames <= 0) {
                    nb_frames = videoStream->codec_info_nb_frames;
                }
                videoDuration = (int64_t) (videoStream->avg_frame_rate.den * 1.0f
                                           / videoStream->avg_frame_rate.num * nb_frames * 1000);
            }
            AVDictionaryEntry *entry = av_dict_get(avStream->metadata, "rotate", nullptr,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (nullptr != entry) {
                videoRotate = atoi(entry->value);
                videoRotate = videoRotate % 360;
                if (videoRotate < 0) {
                    videoRotate += 360;
                }
            }
            if (videoRotate == 90 || videoRotate == 270) {
                int temp = videoWidth;
                videoWidth = videoHeight;
                videoHeight = temp;
            }
            float avg_frame_rate = 1.0f * TARGET_FRAME_RATE;
            if (avStream->avg_frame_rate.den > 0) {
                avg_frame_rate = 1.0f * avStream->avg_frame_rate.num / avStream->avg_frame_rate.den;
            }
            if (avg_frame_rate > TARGET_FRAME_RATE) {
                //-1是去掉基数
                frameIncreaseExtent = avg_frame_rate / TARGET_FRAME_RATE - 1;
                BZLogUtil::logD("avg_frame_rate > TARGET_FRAME_RATE 需要降帧处理 spaceFrame=%f",
                                frameIncreaseExtent);
            }
            BZLogUtil::logD("videoDuration=%lld,videoRotate=%d,avg_frame_rate=%f", videoDuration,
                            videoRotate, avg_frame_rate);
            break;
        }
    }
    if (nullptr == videoStream) {
        BZLogUtil::logE("nullptr==videoStream");
        release();
        return -1;
    }
    videoFrame = av_frame_alloc();
    decodePacket = av_packet_alloc();
    //探测两个关键帧的间隔时间
    int64_t firstKeyFrameTime = -1;
    int64_t firstKeyFramePts = -1;
    while (true) {
        av_packet_unref(decodePacket);
        av_init_packet(decodePacket);
        ret = av_read_frame(in_fmt_ctx, decodePacket);
        if (ret < 0) {
            av_packet_unref(decodePacket);
            BZLogUtil::logD("av_read_frame end");
            break;
        }
        if (decodePacket->stream_index != videoStream->index) {
            continue;
        }
        if (decodePacket->flags == AV_PKT_FLAG_KEY) {
            int64_t innerTime = decodePacket->pts * 1000 *
                                videoStream->time_base.num / videoStream->time_base.den
                                + decodePacket->duration * 1000 *
                                  videoStream->time_base.num / videoStream->time_base.den;
            if (firstKeyFramePts == -1) {
                firstKeyFramePts = decodePacket->pts;
            }
            if (firstKeyFrameTime < 0) {
                firstKeyFrameTime = innerTime;
            } else {
                keyFrameSpaceTime = innerTime - firstKeyFrameTime;
                break;
            }
        }
    }
    //复位
    BZLogUtil::logD("keyFrameSpaceTime=%lld firstKeyFramePts=%lld", keyFrameSpaceTime,
                    firstKeyFramePts);
    if (nullptr != videoCodecContext)
        avcodec_flush_buffers(videoCodecContext);
    av_seek_frame(in_fmt_ctx, videoStream->index, firstKeyFramePts, AVSEEK_FLAG_BACKWARD);
    return 0;
}


int VideoFrameGetter::setStartTime(int64_t startTime) {
    if (startTime < 0 || nullptr == in_fmt_ctx || nullptr == videoStream) {
        return -1;
    }
    //转换成pts
    int64_t pts =
            static_cast<int64_t>(1.0f * startTime /
                                 (1000.0f * videoStream->time_base.num /
                                  videoStream->time_base.den));
    BZLogUtil::logD("setStartTime startTime=%lld,pts=%lld", startTime, pts);
    av_seek_frame(in_fmt_ctx, videoStream->index, pts, AVSEEK_FLAG_BACKWARD);
    return 0;
}

int VideoFrameGetter::release() {
    objectHandle = 0;
    if (nullptr != videoFrame) {
        av_frame_free(&videoFrame);
        videoFrame = nullptr;
    }
    if (nullptr != videoFrameRGBA) {
        av_frame_free(&videoFrameRGBA);
        videoFrameRGBA = nullptr;
    }
    if (nullptr != rgba_sws_video_ctx) {
        sws_freeContext(rgba_sws_video_ctx);
        rgba_sws_video_ctx = nullptr;
    }

    lastDrawAVFrame = nullptr;
    if (nullptr != decodePacket) {
        av_packet_free(&decodePacket);
        decodePacket = nullptr;
    }
    videoCodecContext = nullptr;
    videoStream = nullptr;

    if (nullptr != avFrameDrawProgram) {
        avFrameDrawProgram->releaseResource();
        delete (avFrameDrawProgram);
        avFrameDrawProgram = nullptr;
    }
    if (nullptr != frameBufferUtils) {
        frameBufferUtils->releaseFrameBuffer();
        delete (frameBufferUtils);
        frameBufferUtils = nullptr;
    }

    if (nullptr != in_fmt_ctx) {
        for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
            AVStream *avStream = in_fmt_ctx->streams[i];
            if (nullptr != avStream->codec) {
                avcodec_free_context(&avStream->codec);
                avStream->codec = nullptr;
            }
        }
        avformat_close_input(&in_fmt_ctx);
        in_fmt_ctx = nullptr;
    }

    BZLogUtil::logD("VideoFrameGetter::release Finish videoFrameIndex=%lld,skipFrameCount=%lld",
                    videoFrameIndex, skipFrameCount);
    return 0;
}

int VideoFrameGetter::getVideoWidth() {
    return videoWidth;
}

int VideoFrameGetter::getVideoHeight() {
    return videoHeight;
}

AVFrame *VideoFrameGetter::getAVideoFrame(int64_t currentTime) {
    int64_t innerTime = 0;
    if (nullptr != lastDrawAVFrame) {
        innerTime = lastDrawAVFrame->pts * 1000 *
                    videoStream->time_base.num / videoStream->time_base.den
                    + lastDrawAVFrame->pkt_duration * 1000 *
                      videoStream->time_base.num / videoStream->time_base.den;
    }
    if (nullptr != lastDrawAVFrame && currentTime + 100 < innerTime) {
        if (logCount % LOG_SPACE == 0) {
            BZLogUtil::logV("---重复绘制---");
        }
        return lastDrawAVFrame;
    }
    if (currentTime - innerTime > keyFrameSpaceTime * 2 &&
        nullptr != videoCodecContext) {//需要Seek了
        BZLogUtil::logW("currentTime - innerTime > keyFrameSpaceTime * 2 seek");
        int64_t currentVideoPts =
                currentTime * videoStream->time_base.den / (1000 * videoStream->time_base.num);
        avcodec_flush_buffers(videoCodecContext);
        av_seek_frame(in_fmt_ctx, videoStream->index, currentVideoPts, AVSEEK_FLAG_BACKWARD);
    }

    //解码一帧,如果时间不对就继续解码
    do {
        AVFrame *avFrame = decodeAnVideoFrame();
        if (nullptr != lastDrawAVFrame) {
            innerTime = lastDrawAVFrame->pts * 1000 *
                        videoStream->time_base.num / videoStream->time_base.den
                        + lastDrawAVFrame->pkt_duration * 1000 *
                          videoStream->time_base.num / videoStream->time_base.den;
        }
        lastDrawAVFrame = avFrame;
    } while (nullptr != lastDrawAVFrame && innerTime < currentTime && !hasFlush);
    return lastDrawAVFrame;
}

AVFrame *VideoFrameGetter::getVideoFrameData(int64_t currentTime) {
    if (nullptr == in_fmt_ctx || nullptr == videoStream) {
        BZLogUtil::logD("nullptr == in_fmt_ctx || nullptr == videoStream");
        return nullptr;
    }
    if (enableLoop && videoDuration > 0) {
        if (timeTag < 0) {
            timeTag = currentTime;
        }
        int64_t srcTime = currentTime;
        currentTime = currentTime - timeTag;
        if (hasFlush) {
            av_seek_frame(in_fmt_ctx, videoStream->index, 0, AVSEEK_FLAG_BACKWARD);
            if (nullptr != videoCodecContext)
                avcodec_flush_buffers(videoCodecContext);
            hasFlush = false;
            currentTime = 0;
            lastDrawAVFrame = nullptr;
            timeTag = srcTime;
        }
    }
    logCount++;
    if ((videoDuration > 0 && currentTime > videoDuration) || hasFlush) {
        BZLogUtil::logW("currentTime > videoDuration end");
        return nullptr;
    }
    int64_t innerTime = 0;
    if (nullptr != lastDrawAVFrame) {
        innerTime = lastDrawAVFrame->pts * 1000 *
                    videoStream->time_base.num / videoStream->time_base.den
                    + lastDrawAVFrame->pkt_duration * 1000 *
                      videoStream->time_base.num / videoStream->time_base.den;
    }
    if (nullptr != lastDrawAVFrame && currentTime + 100 < innerTime) {
        if (logCount % LOG_SPACE == 0) {
            BZLogUtil::logV("---重复绘制---");
        }
        return videoFrameRGBA;
    }
    return handleFrame(getAVideoFrame(currentTime));
}

AVFrame *VideoFrameGetter::handleFrame(AVFrame *avFrame) {
    if (nullptr == videoCodecContext) {
        return avFrame;
    }
    if (nullptr == avFrame || avFrame == videoFrameRGBA) {
        BZLogUtil::logW("nullptr == avFrame||avFrame==videoFrameRGBA");
        return videoFrameRGBA;
    }
    AVFrame *finalAVFrame = avFrame;
    if (nullptr == rgba_sws_video_ctx) {
        int dstWidth = finalAVFrame->width;
        int dstHeight = finalAVFrame->height;
        rgba_sws_video_ctx = sws_getContext(finalAVFrame->width, finalAVFrame->height,
                                            videoCodecContext->pix_fmt, dstWidth, dstHeight,
                                            AV_PIX_FMT_RGBA, SWS_BILINEAR,
                                            nullptr, nullptr, nullptr);
        videoFrameRGBA = VideoUtil::allocVideoFrame(AV_PIX_FMT_RGBA, dstWidth, dstHeight);
    }

    //转换为RGB
    sws_scale(rgba_sws_video_ctx,
              (const uint8_t *const *) finalAVFrame->data,
              finalAVFrame->linesize, 0, finalAVFrame->height,
              videoFrameRGBA->data,
              videoFrameRGBA->linesize);
    return videoFrameRGBA;
}


int VideoFrameGetter::getVideoFrame(int64_t currentTime) {
    if (nullptr == in_fmt_ctx || nullptr == videoStream) {
        BZLogUtil::logD("nullptr == in_fmt_ctx || nullptr == videoStream");
        return -1;
    }
    if (enableLoop && videoDuration > 0) {
        currentTime = currentTime % videoDuration;
        if (hasFlush) {
            av_seek_frame(in_fmt_ctx, videoStream->index, 0, AVSEEK_FLAG_BACKWARD);
            hasFlush = false;
            lastDrawAVFrame = nullptr;
            currentTime = 0;
        }
    }
    logCount++;
    if ((videoDuration > 0 && currentTime > videoDuration) || hasFlush) {
        BZLogUtil::logW("currentTime > videoDuration end");
        return -1000;
    }
    return drawFrame(getAVideoFrame(currentTime));
}


int VideoFrameGetter::drawFrame(AVFrame *avFrame) {
    if (nullptr == videoCodecContext) {
        return 0;
    }
    if (nullptr == avFrame) {
        BZLogUtil::logW("nullptr==avFrame");
        return nullptr != frameBufferUtils ? frameBufferUtils->getBuffersTextureId() : -1;
    }
    if (nullptr == avFrameDrawProgram) {
        YUV_Type yuv_type;
        if (videoCodecContext->pix_fmt == AV_PIX_FMT_NV12 ||
            videoCodecContext->pix_fmt == AV_PIX_FMT_NV21) {
            yuv_type = YUV_TYPE_NV21_NV12;
        } else {
            yuv_type = YUV_TYPE_YUV420P;
        }
        avFrameDrawProgram = new AVFrameDrawProgram(yuv_type);
        avFrameDrawProgram->setFlip(false, true);
        avFrameDrawProgram->setRotation(videoRotate);
    }
    if (nullptr == frameBufferUtils) {
        frameBufferUtils = new FrameBufferUtils();
        frameBufferUtils->initFrameBuffer(videoWidth, videoHeight);
    }
    frameBufferUtils->bindFrameBuffer();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    //+1是为了防止边界问题?
    if (videoRotate == 90 || videoRotate == 270) {
        glViewport(0, 0, avFrame->height,
                   avFrame->linesize[0] != videoWidth ? avFrame->linesize[0] + 1
                                                      : avFrame->linesize[0]);
    } else {
        glViewport(0, 0,
                   avFrame->linesize[0] != videoWidth ? avFrame->linesize[0] + 1
                                                      : avFrame->linesize[0],
                   avFrame->height);
    }

    avFrameDrawProgram->draw(avFrame);
    frameBufferUtils->unbindFrameBuffer();
    return frameBufferUtils->getBuffersTextureId();
}

AVFrame *VideoFrameGetter::decodeAnVideoFrame() {
    if (hasFlush) {
        return lastDrawAVFrame;
    }
    int ret = 0;
    while (true) {
        av_packet_unref(decodePacket);
        av_init_packet(decodePacket);
        ret = av_read_frame(in_fmt_ctx, decodePacket);
        if (ret < 0) {
            av_packet_unref(decodePacket);
            BZLogUtil::logD("av_read_frame end");
            break;
        }
        if (decodePacket->stream_index != videoStream->index) {
            continue;
        }
        av_frame_unref(videoFrame);
        int64_t startTime = getCurrentTime();
        avcodec_send_packet(videoCodecContext, decodePacket);
        ret = avcodec_receive_frame(videoCodecContext, videoFrame);
        av_packet_unref(decodePacket);
        if (logCount % LOG_SPACE == 0) {
            BZLogUtil::logV("解码耗时=%lld,userSoftDecode=%d", getCurrentTime() - startTime,
                            userSoftDecode);
        }
        if (ret < 0) {
            BZLogUtil::logE("decodeVideo avcodec_decode_video2 fail");
            av_frame_unref(videoFrame);
            continue;
        } else {
            videoFrameIndex++;
            if (frameIncreaseExtent > 0 && skipFrameBufferCount > 1) {
                skipFrameCount++;
                BZLogUtil::logV("skip a frame skipFrameCount=%lld", skipFrameCount);
                skipFrameBufferCount -= 1;
                continue;
            }
            skipFrameBufferCount += frameIncreaseExtent;
            if (logCount % LOG_SPACE == 0) {
                BZLogUtil::logV("skipFrameBufferCount=%f", skipFrameBufferCount);
            }
            break;
        }
    }
    if (ret < 0 && !hasFlush) {
        //刷新缓存
        while (true) {
            av_frame_unref(videoFrame);
            av_packet_unref(decodePacket);
            av_init_packet(decodePacket);
            decodePacket->data = nullptr;
            decodePacket->size = 0;
            avcodec_send_packet(videoCodecContext, decodePacket);
            ret = avcodec_receive_frame(videoCodecContext, videoFrame);
            av_packet_unref(decodePacket);
            if (ret < 0) {
                hasFlush = true;
                av_frame_unref(videoFrame);
                break;
            } else {
                videoFrameIndex++;
                BZLogUtil::logV("----flushDecodeVideo----pts=%lld", videoFrame->pts);
                if (frameIncreaseExtent > 0 &&
                    videoFrameIndex == (int64_t) (skipFrameCount * frameIncreaseExtent)) {
                    skipFrameCount++;
                    BZLogUtil::logV("skip a frame skipFrameCount=%lld", skipFrameCount);
                    continue;
                }
                break;
            }
        }
    }
    return ret >= 0 ? videoFrame : nullptr;
}

void VideoFrameGetter::setObjectHandle(int64_t objectHandle) {
    this->objectHandle = objectHandle;
}

int64_t VideoFrameGetter::getObjectHandle() {
    return objectHandle;
}

int64_t VideoFrameGetter::getVideoDuration() {
    return videoDuration;
}

int VideoFrameGetter::setEnableLoop(bool enableLoop) {
    this->enableLoop = enableLoop;
    return 0;
}
