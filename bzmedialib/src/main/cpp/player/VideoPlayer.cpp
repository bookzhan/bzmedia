//
/**
 * Created by bookzhan on 2019-02-13 10:38.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <common/bz_time.h>
#include "VideoPlayer.h"

int VideoPlayer::init(VideoPlayerInitParams *videoPlayerInitParams) {
    if (nullptr == videoPlayerInitParams || nullptr == videoPlayerInitParams->videoPath) {
        BZLogUtil::logE("VideoPlayerInitParams Error");
        return -1;
    }
    innerInitSuccessFlag = false;
    videoDecodeSuccess = false;
//    videoPlayerInitParams->userSoftDecode = true;

    BZLogUtil::logD(
            "VideoPlayer init videoPath=%s,methodHandle=%lld,userSoftDecode=%d,synDecodeCache=%d",
            videoPlayerInitParams->videoPath, videoPlayerInitParams->methodHandle,
            videoPlayerInitParams->userSoftDecode, videoPlayerInitParams->prepareSyn);
    this->videoPlayerInitParams = videoPlayerInitParams;

    int ret = 0;
    //软硬解
    if (videoPlayerInitParams->userSoftDecode) {
        if ((ret = VideoUtil::openInputFileForSoft(videoPlayerInitParams->videoPath, &in_fmt_ctx)) <
            0) {
            BZLogUtil::logE("openInputFile fail %s", videoPlayerInitParams->videoPath);
            in_fmt_ctx = nullptr;
            release();
            return ret;
        }
    } else {
        if (VideoUtil::openInputFile(videoPlayerInitParams->videoPath, &in_fmt_ctx) < 0) {
            in_fmt_ctx = nullptr;
            BZLogUtil::logE("openInputFile fail 切换到软解 %s", videoPlayerInitParams->videoPath);
            if ((ret = VideoUtil::openInputFileForSoft(videoPlayerInitParams->videoPath,
                                                       &in_fmt_ctx)) < 0) {
                BZLogUtil::logE("openInputFileForSoft fail %s", videoPlayerInitParams->videoPath);
                in_fmt_ctx = nullptr;
                release();
                return ret;
            }
        }
    }
    //分配必要的东西
    if ((ret = allocResource()) < 0) {
        BZLogUtil::logE("allocResource fail %s", videoPlayerInitParams->videoPath);
        release();
        return ret;
    }
    //先做个缓存
    if (videoPlayerInitParams->prepareSyn) {
        decodeVideo();
    }
    videoDecodeThreadIsEnd = false;
    thread threadDecodeVideo(&VideoPlayer::decodeVideoThread, this);
    threadDecodeVideo.detach();

    //音频播放线程,如果没有音频就不开线程处理
    if (nullptr != audioCodecContext) {
        audioPlayThreadIsEnd = false;
        thread threadPlayAudio(&VideoPlayer::audioPlayThread, this);
        threadPlayAudio.detach();
    }

    if (nullptr != videoStream) {
        float tempDuration = videoStream->duration * 1.0f / videoStream->nb_frames;
        averageDuration = static_cast<int>(tempDuration * 1000 *
                                           videoStream->time_base.num /
                                           videoStream->time_base.den);
        BZLogUtil::logD("averageDuration=%d--normal_duration=33",
                        averageDuration);
    }
    innerInitSuccessFlag = true;
    return averageDuration;

}

int64_t VideoPlayer::onDraw(int64_t time) {
    logCount++;
    if (videoPlayerIsRelease || nullptr == videoPlayerInitParams || nullptr == videoStream) {
        BZLogUtil::logE(
                "videoPlayerIsRelease || nullptr == videoPlayerInitParams|| nullptr==videoStream");
        return -1;
    }
    if (!innerInitSuccessFlag) {
        BZLogUtil::logE("!innerInitSuccessFlag");
        return -1;
    }
    //取新数据绘制
    if (videoAVFrameDeque.isEmpty() && nullptr == lastDrawAVFrame) {
        return -1;
    }
    if (!videoAVFrameDeque.isEmpty() && videoPlayTime + 100 < audioPlayTime) {
        //开始内部跳帧
        BZLogUtil::logD("~~~开始内部跳帧~~~videoPlayTime=%lld,audioPlayTime=%lld videoPath=%s",
                        videoPlayTime, audioPlayTime, videoPlayerInitParams->videoPath);
        while (true) {
            AVFrame *avFrame = videoAVFrameDeque.getFirst();
            if (nullptr == avFrame) {
                BZLogUtil::logD("~~~跳帧结束~~~ nullptr == avFrame");
                break;
            }
            int64_t videoPlayTimeTemp =
                    avFrame->pts * 1000 * videoStream->time_base.num /
                    videoStream->time_base.den;
            av_frame_free(&avFrame);
            videoPlayTime = videoPlayTimeTemp;
            if (videoPlayTimeTemp >= audioPlayTime) {
                BZLogUtil::logD("~~~跳帧结束~~~ videoPlayTimeTemp>=audioPlayTime");
                break;
            }
        }
    }
    if (logCount > 10000) {
        logCount = 0;
    }
    AVFrame *tempFrame = getFinalAVFrame(time);
    drawFrame(tempFrame);
    if (logCount % LOG_SPACE == 0) {
        BZLogUtil::logV("VideoPlayer::onDraw time=%lld", time);
    }
    return getPlayTime();
}

AVFrame *VideoPlayer::getFinalAVFrame(int64_t currentTime) {
    AVFrame *tempFrame = lastDrawAVFrame;
    if (nullptr != tempFrame && nullptr != videoStream) {
        int64_t ptsTime =
                tempFrame->pts * 1000 *
                videoStream->time_base.num /
                videoStream->time_base.den;
        if (currentTime <= ptsTime) {
            return tempFrame;
        }
    }
    if (!videoAVFrameDeque.isEmpty()) {
        tempFrame = videoAVFrameDeque.getFirst();
    } else {
        if (!videoPlayCompletionHasCallBacked) {
            BZLogUtil::logV("getFinalAVFrame videoAVFrameDeque.empty() 重复绘制");
        }
    }
    return tempFrame;
}

int VideoPlayer::reStart() {
    BZLogUtil::logD("VideoPlayer::reStart");
    pause();
    release();
    videoPlayerIsRelease = false;
    if (nullptr != videoPlayerInitParams) {
        init(videoPlayerInitParams);
        start();
    }
    return 0;
}

int VideoPlayer::start() {
//    BZLogUtil::logD("VideoPlayer::start");
    videoPlayerIsPause = false;
    pcmPlayerStart();
    return 0;
}

int VideoPlayer::onPause() {
    BZLogUtil::logD("VideoPlayer::onPause");
    if (nullptr != avFrameDrawProgram) {
        avFrameDrawProgram->releaseResource();
        delete (avFrameDrawProgram);
        avFrameDrawProgram = nullptr;
    }
    return 0;
}

int VideoPlayer::pause() {
    videoPlayerIsPause = true;
    pcmPlayerPause();
    BZLogUtil::logD("VideoPlayer::pause");
    return 0;
}

int VideoPlayer::release() {
    BZLogUtil::logD("VideoPlayer::release");
    videoPlayerIsRelease = true;
    std::chrono::milliseconds dura(30);
    while (!videoDecodeThreadIsEnd || !audioPlayThreadIsEnd) {
        BZLogUtil::logV("!videoDecodeThreadIsEnd || !audioPlayThreadIsEnd sleep");
        std::this_thread::sleep_for(dura);
    }
    releaseResource();
    return 0;
}

VideoPlayer::~VideoPlayer() {
    BZLogUtil::logD("VideoPlayer::~VideoPlayer");
    if (nullptr != videoPlayerInitParams) {
        if (nullptr != videoPlayerInitParams->videoPath) {
            free((void *) videoPlayerInitParams->videoPath);
            videoPlayerInitParams->videoPath = nullptr;
        }
        delete (videoPlayerInitParams);
        videoPlayerInitParams = nullptr;
    }
}

int VideoPlayer::pcmPlayerInit() {
    BZLogUtil::logD("pcmPlayerInit");
    pcmPlayerRelease();
    pcmPlayerLock.lock();
    pcmPlayer = new PCMPlayerNative();
    pcmPlayer->init(SAMPLE_RATE, 1);
    pcmPlayer->setVideoPlayerVolume(volume);
    pcmPlayerLock.unlock();
    return 0;
}

int VideoPlayer::pcmPlayerStart() {
//    BZLogUtil::logD("pcmPlayerStart");
    if (nullptr != pcmPlayer && audioPlayIsPause)
        pcmPlayer->start();
    audioPlayIsPause = false;
    return 0;
}

void VideoPlayer::onPCMDataAvailable(const char *pcmData, int length) {
//    BZLogUtil::logD("onPCMDataAvailable");
    if (nullptr != pcmPlayer)
        pcmPlayer->onPCMDataAvailable(pcmData, length);
}

int VideoPlayer::pcmPlayerPause() {
//    BZLogUtil::logD("pcmPlayerPause");
    if (nullptr != pcmPlayer && !audioPlayIsPause)
        pcmPlayer->pause();
    audioPlayIsPause = true;
    return 0;
}

void VideoPlayer::setAudioVolume(float volume) {
    BZLogUtil::logD("setAudioVolume volume=%f", volume);
    this->volume = volume;
    if (nullptr != pcmPlayer)
        pcmPlayer->setVideoPlayerVolume(volume);
}


int VideoPlayer::pcmPlayerRelease() {
    BZLogUtil::logD("pcmPlayerRelease");
    pcmPlayerLock.lock();
    if (nullptr != pcmPlayer) {
        pcmPlayer->pause();
        pcmPlayer->stopAudioTrack();
        delete (pcmPlayer);
        pcmPlayer = nullptr;
    }
    pcmPlayerLock.unlock();
    return 0;
}

void VideoPlayer::decodeVideoThread() {
    BZLogUtil::logD("videoDecodeThread start");
    std::chrono::milliseconds dura(30);
    int64_t errorCount = 0;
    int ret = 0;
    bool decodeFail = false;
    while (true) {
        if (videoPlayerIsRelease) {
            break;
        }
        if (videoAVFrameDeque.getSize() > VIDEO_AVFRAME_DEQUE_MAX_SIZE) {
            std::this_thread::sleep_for(dura);
            if (logCount > 10 && logCount % 30 == 0) {
                BZLogUtil::logV(
                        "decodeVideoThread videoFrameDeque.size() > VIDEO_AVFRAME_DEQUE_MAX_SIZE");
            }
        } else {
            if (requestSeekToStart) {
                requestSeekToStart = false;
                BZLogUtil::logD("requestSeekToStart");
                seek(0);
            }
            ret = decodeVideo();
            if (ret < 0) {
                errorCount++;
                if (errorCount > 30 && videoAVFrameDeque.isEmpty()) {
                    decodeFail = true;
                    break;
                }
            } else {
                errorCount = 0;
            }
        }
    }
    videoDecodeThreadIsEnd = true;
    if (decodeFail && !videoPlayerIsRelease) {
        BZLogUtil::logE("---解码失败启动软解--");
        videoPlayerInitParams->userSoftDecode = true;
        videoPlayerInitParams->prepareSyn = true;
        reStart();
    }
    BZLogUtil::logD("videoDecodeThread end");
}

int VideoPlayer::decodeVideo() {
    if (videoPlayerIsRelease || nullptr == videoPlayerInitParams || nullptr == videoStream) {
        return -1;
    }
    int ret = 0;
    if (videoPacketDeque.isEmpty()) {
        readPacket();
    }
    //刷新缓存
    if (videoPacketDeque.isEmpty()) {
        //先刷新缓存,再处理seek的情况
        flushDecodeVideo();
        //处理没有音频的情况
        if (nullptr == audioCodecContext && videoAVFrameDeque.isEmpty()) {
            if (!videoPlayCompletionHasCallBacked) {
                videoPlayCompletionHasCallBacked = true;
                onVideoPlayCompletion(true);
                if (isLoop) {
                    BZLogUtil::logD("decodeVideo isLoop seek 0");
                    requestSeekToStart = true;
                }
            }
            callBackProgress(1);
        }
        return 0;
    } else {
        onVideoPlayCompletion(false);
    }
    if (nullptr == audioCodecContext) {
        videoPlayCompletionHasCallBacked = false;
    }
    AVPacket *decode_pkt = videoPacketDeque.getFirst();
    if (nullptr == decode_pkt) {
        return -1;
    }

    int64_t startTime = getCurrentTime();
    AVFrame *videoFrame = av_frame_alloc();
    videoCodecLock.lock();
    avcodec_send_packet(videoCodecContext, decode_pkt);
    ret = avcodec_receive_frame(videoCodecContext, videoFrame);

    videoCodecLock.unlock();
    if (logCount % LOG_SPACE == 0 && !videoPlayCompletionHasCallBacked) {
        BZLogUtil::logV("VideoPlayer 解码耗时=%lld userSoftDecode=%d", getCurrentTime() - startTime,
                        videoPlayerInitParams->userSoftDecode);
    }
    av_packet_free(&decode_pkt);
    if (ret < 0) {
        BZLogUtil::logE("decodeVideo avcodec_decode_video2 fail");
        av_frame_unref(videoFrame);
        //解码失败这个时候不要调用av_packet_free,可能是由于硬解造成的
//        av_packet_unref(decode_pkt);
        return -1;
    }
    videoDecodeSuccess = true;
    videoAVFrameDeque.pushBack(videoFrame);
    return ret;
}

int VideoPlayer::flushDecodeVideo() {
    int ret = 0;
//    int64_t tempTime = getCurrentTime();
    while (true) {
        AVPacket *decode_pkt = av_packet_alloc();
        decode_pkt->data = nullptr;
        decode_pkt->size = 0;
        AVFrame *videoFrame = av_frame_alloc();
        videoCodecLock.lock();
        avcodec_send_packet(videoCodecContext, decode_pkt);

        ret = avcodec_receive_frame(videoCodecContext, videoFrame);

        videoCodecLock.unlock();
        av_packet_free(&decode_pkt);
        if (ret < 0) {
//            BZLogUtil::logE("flushDecodeVideo avcodec_decode_video2 fail");
            av_frame_free(&videoFrame);
            break;
        }
        BZLogUtil::logV("----flushDecodeVideo----pts=%lld", videoFrame->pts);
        videoAVFrameDeque.pushBack(videoFrame);
    }
//    BZLogUtil::logD("flushDecodeVideo 耗时=%lld", getCurrentTime() - tempTime);
    return 0;
}


void VideoPlayer::audioPlayThread() {
    BZLogUtil::logD("audioPlayThread start");
    std::chrono::milliseconds dura(30);
    while (nullptr != videoStream) {
        if (logCount % 30 == 0)
            BZLogUtil::logV("waiting decodeVideoSuccess this=%lld", (int64_t) this);
        if (videoPlayTime > 0) {
            break;
        }
        if (videoPlayerIsRelease) {
            break;
        }
        logCount++;
        std::this_thread::sleep_for(dura);
    }

    int ret = 0, got_picture_ptr = 0;
    AVFrame *audioFrameTemp = VideoUtil::allocAudioFrame(AV_SAMPLE_FMT_S16,
                                                         AV_CH_LAYOUT_MONO,
                                                         SAMPLE_RATE, NB_SAMPLES);
    pcmPlayerInit();
    int errorCount = 0;
    while (true) {
        if (videoPlayerIsRelease) {
            BZLogUtil::logD("audioPlayThread videoPlayerIsRelease break");
            break;
        }
        if (videoPlayerIsPause || audioPlayIsPause) {
            if (logCount % LOG_SPACE == 0) {
                BZLogUtil::logV(
                        "decodeAndPlayAudioThread isPause || isVideoSeek sleep||isPauseAudioPlay");
                logCount++;
            }
            std::this_thread::sleep_for(dura);
            continue;
        }
        if (requestSetVolume) {
            setAudioVolume(volume);
            requestSetVolume = false;
        }

        if (audioPacketDeque.isEmpty()) {
            readPacket();
        }
        AVPacket *decode_pkt = audioPacketDeque.getFirst();
        lastAudioPlayPts = decode_pkt->pts;

        //回调进度
        if (nullptr != decode_pkt && audioStream->duration > 0) {
            float progress = 1.0f * lastAudioPlayPts /
                             audioStream->duration;
            callBackProgress(progress);
        }

        if (audioPacketDeque.isEmpty()) {
            if (videoAVFrameDeque.isEmpty() && !videoPlayCompletionHasCallBacked) {
                videoPlayCompletionHasCallBacked = true;
                onVideoPlayCompletion(true);
                if (isLoop) {
                    BZLogUtil::logD("audioPlayThread isLoop seek 0");
                    seek(0);
                }
            }
            callBackProgress(1);
//            BZLogUtil::logV("audioPacketList.size()<=0 sleep");
            std::this_thread::sleep_for(dura);
            continue;
        } else {
            onVideoPlayCompletion(false);
        }
        if (nullptr == decode_pkt) {
            continue;
        }
        videoPlayCompletionHasCallBacked = false;

        audioCodecLock.lock();
        ret = avcodec_decode_audio4(audioCodecContext, audioFrame, &got_picture_ptr,
                                    decode_pkt);
        av_packet_free(&decode_pkt);
        audioCodecLock.unlock();

        if (ret < 0) {
            errorCount++;
            if (errorCount > 10) {
                break;
            }
            av_frame_unref(audioFrame);
            BZLogUtil::logD("avcodec_decode_video2 fail");
            continue;
        }
        if (!got_picture_ptr) {
            av_frame_unref(audioFrame);
            BZLogUtil::logD("VideoPlayer avcodec_decode_audio4 got_picture_ptr fail continue");
            continue;
        }
        errorCount = 0;
        //转换格式
        int samplesCount = swr_convert(swr_audio_ctx, audioFrameTemp->data,
                                       audioFrameTemp->nb_samples,
                                       (const uint8_t **) audioFrame->data,
                                       audioFrame->nb_samples);
//            BZLogUtil::logD("samplesCount=%d", samplesCount);

        av_audio_fifo_write(audioFifo, (void **) &audioFrameTemp->data[0], samplesCount);
        while (av_audio_fifo_size(audioFifo) >= NB_SAMPLES) {
            av_audio_fifo_read(audioFifo, (void **) &audioFrameTemp->data[0],
                               NB_SAMPLES);
            if (nullptr != videoStream) {
                onPCMDataAvailable((const char *) audioFrameTemp->data[0],
                                   audioFrameTemp->linesize[0]);
            }
        }
        setPlayTime();

        av_frame_unref(audioFrame);
//        BZLogUtil::logE("videoPlayTime=% lld--audioPlayTime=%lld", videoPlayTime, audioPlayTime);
    }
    av_frame_free(&audioFrameTemp);
    pcmPlayerRelease();

    audioPlayThreadIsEnd = true;
    BZLogUtil::logD("audioPlayThread end");
}

void VideoPlayer::seek(int64_t time) {
    int64_t currentVideoPts = 0;
    if (nullptr != videoStream) {
        currentVideoPts = time * videoStream->time_base.den / (1000 * videoStream->time_base.num);
    }
    int64_t currentAudioPts = 0;
    if (nullptr != audioStream) {
        currentAudioPts = time * audioStream->time_base.den / (1000 * audioStream->time_base.num);
    }
    BZLogUtil::logD("videoPlayerSeek seek time=%lld", time);
    seekInner(currentVideoPts, currentAudioPts);
}


void VideoPlayer::seekInner(int64_t videoPts, int64_t audioPts) {
    if (nullptr == in_fmt_ctx || videoPlayerIsRelease || nullptr == videoCodecContext ||
        !innerInitSuccessFlag) {
        return;
    }
    if (nullptr == videoStream && nullptr == audioStream) {
        BZLogUtil::logE("nullptr==videoStream&&nullptr==audioStream");
        return;
    }
    BZLogUtil::logD("videoPlayerSeek videoPts=%lld audioPts=%lld", videoPts, audioPts);
    while (true) {
        AVFrame *avFrame = videoAVFrameDeque.front();
        if (nullptr == avFrame) {
            break;
        }
        if (avFrame->pts >= videoPts) {
            BZLogUtil::logD("seekInner use cache");
            return;
        }
        avFrame = videoAVFrameDeque.getFirst();
        if (nullptr == avFrame) {
            break;
        }
        av_frame_free(&avFrame);
    }

    videoAVFrameDeque.clear();

    //解码还没有成功,就不做处理了
    if (nullptr != lastDrawAVFrame) {
        videoCodecLock.lock();
        avcodec_flush_buffers(videoCodecContext);
        videoCodecLock.unlock();
    }
    if (nullptr != audioCodecContext) {
        audioCodecLock.lock();
        avcodec_flush_buffers(audioCodecContext);
        audioCodecLock.unlock();
    }
    fmt_ctxLock.lock();
    //没有视频流的时候以音频流为准
    if (nullptr != videoStream) {
        av_seek_frame(in_fmt_ctx, videoStream->index, videoPts, AVSEEK_FLAG_BACKWARD);
    } else if (nullptr != audioStream) {
        av_seek_frame(in_fmt_ctx, audioStream->index, audioPts, AVSEEK_FLAG_BACKWARD);
    }
    fmt_ctxLock.unlock();
    videoPacketDeque.clear();
    audioPacketDeque.clear();

    if (nullptr != videoStream) {
        if (!videoAVFrameDeque.isEmpty()) {
            videoPlayTime = videoAVFrameDeque.front()->pts * 1000 * videoStream->time_base.num /
                            videoStream->time_base.den;
        } else if (!videoPacketDeque.isEmpty()) {
            videoPlayTime = videoPacketDeque.front()->pts * 1000 * videoStream->time_base.num /
                            videoStream->time_base.den;
        }
    }
    if (!audioPacketDeque.isEmpty() && nullptr != audioStream) {
        audioPlayTime = audioPacketDeque.front()->pts * 1000 * audioStream->time_base.num /
                        audioStream->time_base.den;
    }
    BZLogUtil::logD("videoPlayerSeek finish");
}

int VideoPlayer::allocResource() {
    BZLogUtil::logD("allocResource");
    if (nullptr == in_fmt_ctx || nullptr == videoPlayerInitParams) {
        BZLogUtil::logD(
                "allocResource nullptr==in_fmt_ctx || nullptr == videoPlayerInitParams");
        return -1;
    }
    int ret = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = in_fmt_ctx->streams[i];
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoWidth = in_stream->codecpar->width / 8 * 8;
            videoHeight = in_stream->codecpar->height / 8 * 8;
            videoStream = in_stream;
            videoStreamTotalTime = in_stream->duration * 1000 * in_stream->time_base.num /
                                   in_stream->time_base.den;
            videoCodecContext = in_stream->codec;


            AVDictionaryEntry *entry = av_dict_get(in_stream->metadata, "rotate", nullptr,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (nullptr != entry) {
                videoRotate = atoi(entry->value);
                videoRotate = videoRotate % 360;
                if (videoRotate < 0) {
                    videoRotate += 360;
                }
            }
            BZLogUtil::logD("videoRotate=%d", videoRotate);
            videoDuration = videoStream->duration * 1000 * videoStream->time_base.num /
                            videoStream->time_base.den;

            if (videoHeight > 0 && videoWidth > 0 &&
                nullptr != videoPlayerInitParams->onVideoInfoAvailableCallBack) {
                float avg_frame_rate =
                        1.0f * in_stream->avg_frame_rate.num / in_stream->avg_frame_rate.den;
                videoPlayerInitParams->onVideoInfoAvailableCallBack(
                        videoPlayerInitParams->methodHandle, videoWidth, videoHeight,
                        videoRotate,
                        videoDuration, avg_frame_rate);
            }
        } else if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = in_stream;
            audioStreamTotalTime = in_stream->duration * 1000 * in_stream->time_base.num /
                                   in_stream->time_base.den;
            /* create resampler context */
            swr_audio_ctx = swr_alloc();
            if (!swr_audio_ctx) {
                BZLogUtil::logD("Could not allocate resampler context\n");
                return -1;
            }
            /* set options */
            av_opt_set_int(swr_audio_ctx, "out_channel_count", 1, 0);
            av_opt_set_int(swr_audio_ctx, "out_sample_rate", SAMPLE_RATE, 0);
            av_opt_set_sample_fmt(swr_audio_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

            av_opt_set_int(swr_audio_ctx, "in_channel_count", in_stream->codecpar->channels, 0);
            av_opt_set_int(swr_audio_ctx, "in_sample_rate", in_stream->codecpar->sample_rate,
                           0);
            av_opt_set_sample_fmt(swr_audio_ctx, "in_sample_fmt",
                                  static_cast<AVSampleFormat>(in_stream->codecpar->format),
                                  0);

            /* initialize the resampling context */
            if ((ret = swr_init(swr_audio_ctx)) < 0) {
                BZLogUtil::logD("Failed to initialize the resampling context\n");
                return ret;
            }
            audioFrame = av_frame_alloc();

            audioCodecContext = in_stream->codec;
        }
    }
    audioFifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_S16, 1,
                                    NB_SAMPLES);

    BZLogUtil::logD("videoStreamTotalTime=%lld,audioStreamTotalTime=%lld", videoStreamTotalTime,
                    audioStreamTotalTime);
    if (nullptr == videoCodecContext) {
        return -1;
    }
    return 0;
}

int64_t VideoPlayer::getPlayTime() {
    if (nullptr != audioCodecContext && nullptr != audioStream) {
        if (audioPlayTime)
            return audioPlayTime;
    }
    return videoPlayTime;
}

int64_t VideoPlayer::drawFrame(AVFrame *avFrame) {
    if (nullptr == avFrame) {
        BZLogUtil::logE("VideoPlayer::drawFrame nullptr==avFrame");
        return -1;
    }
    if (videoPlayerIsRelease || !innerInitSuccessFlag) {
        BZLogUtil::logE("drawFrame videoPlayerIsRelease || !innerInitSuccessFlag");
        return -1;
    }
//    BZLogUtil::logV("drawFrame pts=%lld", avFrame->pts);
    AVFrame *finalDrawFrame = avFrame;
    int ret = 0;
    if (nullptr != videoCodecContext) {
        setPlayTime();
        if (nullptr == audioCodecContext) {
            if (videoStream->duration > 0) {
                float progress = 1.0f * avFrame->pts /
                                 videoStream->duration;
                callBackProgress(progress);
            }
        }
        //在用的时候再分配,mediacode 的pix_fmt 设置有延迟
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
        ret = avFrameDrawProgram->draw(avFrame);
        if (ret < 0) {
            BZLogUtil::logW("avFrameDrawProgram->draw fail retry lastDrawAVFrame");
            if (lastAVFrameEnable()) {
                avFrameDrawProgram->draw(lastDrawAVFrame);
            }
        }
    }
    if (nullptr != lastDrawAVFrame && lastDrawAVFrame != finalDrawFrame) {
        av_frame_free(&lastDrawAVFrame);
    }
    lastVideoDrawPts = finalDrawFrame->pts;
    lastDrawAVFrame = finalDrawFrame;
    return ret >= 0 ? finalDrawFrame->pts : ret;
}

void VideoPlayer::callBackProgress(float progress) {
    if (!videoPlayerIsPause && progress > 0 && nullptr != videoPlayerInitParams &&
        videoPlayerInitParams->progressCallBack != nullptr) {
        if (progress < 0) {
            BZLogUtil::logD("callBackProgress progress < 0 progress=%f", progress);
            progress = 0;
        }
        if (progress > 1) {
            BZLogUtil::logD("callBackProgress progress > 1 progress=%f", progress);
            progress = 1;
        }
        videoPlayerInitParams->progressCallBack(videoPlayerInitParams->methodHandle, progress);
    }
}

int VideoPlayer::readPacket() {
    if (videoPlayerIsRelease) {
        return -1;
    }
//    BZLogUtil::logV("start readPacket");
    AVStream *in_stream;
    int ret = 0;
//    int64_t startTime = getCurrentTime();

    long videoPacketSize = videoPacketDeque.getSize();
    long audioPacketSize = audioPacketDeque.getSize();

    if (logCount % LOG_SPACE == 0 && !videoPlayCompletionHasCallBacked)
        BZLogUtil::logV("videoPacketSize=%d,audioPacketSize=%d", videoPacketSize, audioPacketSize);
    while (videoPacketSize < PACKET_BUFFER_SIZE || audioPacketSize < PACKET_BUFFER_SIZE) {
        AVPacket *decode_pkt = av_packet_alloc();
        av_init_packet(decode_pkt);
        fmt_ctxLock.lock();
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0) {
            av_packet_free(&decode_pkt);
//            BZLogUtil::logV("read packet finish");
            fmt_ctxLock.unlock();
            break;
        }
        in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];
        fmt_ctxLock.unlock();

        if (in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoPacketDeque.pushBack(decode_pkt);
            videoPacketSize++;
        } else if (in_stream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioPacketDeque.pushBack(decode_pkt);
            audioPacketSize++;
        }
    }
//    BZLogUtil::logV("read packet 耗时=%lld, this=%lld", getCurrentTime() - startTime,
//                    (int64_t) this);
    return ret;
}

void VideoPlayer::setPlayLoop(bool isLoop) {
    BZLogUtil::logD("setPlayLoop=%d", isLoop);
    this->isLoop = isLoop;
}

int64_t VideoPlayer::getCurrentAudioPts() {
    return lastAudioPlayPts;
}

int VideoPlayer::releaseResource() {
    if (nullptr != videoPlayerInitParams)
        BZLogUtil::logD("VideoPlayer releaseResource videoPath=%s",
                        videoPlayerInitParams->videoPath);
//    BZLogUtil::logD("VideoPlayer releaseResource lock");
    resourceReleaseLock.lock();

    if (nullptr != lastDrawAVFrame) {
        av_frame_unref(lastDrawAVFrame);
        lastDrawAVFrame = nullptr;
    }
    videoAVFrameDeque.clear();

//    BZLogUtil::logD("VideoPlayer releaseResource lock success");
    if (nullptr != videoStream && nullptr != videoStream->codec) {
        videoCodecLock.lock();
        avcodec_close(videoStream->codec);
        videoStream->codec = nullptr;
        videoCodecContext = nullptr;
        videoCodecLock.unlock();
    }
    if (nullptr != audioStream && nullptr != audioStream->codec) {
        audioCodecLock.lock();
        avcodec_close(audioStream->codec);
        audioStream->codec = nullptr;
        audioCodecContext = nullptr;
        audioCodecLock.unlock();
    }
    if (nullptr != in_fmt_ctx) {
        avformat_close_input(&in_fmt_ctx);
        in_fmt_ctx = nullptr;
    }
    videoCodecContext = nullptr;
    audioCodecContext = nullptr;
    videoStream = nullptr;
    audioStream = nullptr;

    videoPacketDeque.clear();
    audioPacketDeque.clear();

    if (nullptr != audioFifo) {
        av_audio_fifo_free(audioFifo);
        audioFifo = nullptr;
    }

    if (nullptr != audioFrame) {
        av_frame_free(&audioFrame);
        audioFrame = nullptr;
    }

    if (nullptr != avFrameDrawProgram) {
        avFrameDrawProgram->releaseResource();
        delete (avFrameDrawProgram);
        avFrameDrawProgram = nullptr;
    }
    if (nullptr != swr_audio_ctx) {
        swr_free(&swr_audio_ctx);
        swr_audio_ctx = nullptr;
    }
    resourceReleaseLock.unlock();

//    releaseResourceFinish = true;
    BZLogUtil::logD("VideoPlayer releaseResource finish");
    return 0;
}

void VideoPlayer::setPlayTime() {
    if (videoPlayerIsRelease) {
        return;
    }
    playTimeLock.lock();
    if (nullptr != videoStream) {
        videoPlayTime =
                lastVideoDrawPts * 1000 *
                videoStream->time_base.num /
                videoStream->time_base.den;
    }
    if (nullptr != audioCodecContext && nullptr != audioStream) {
        audioPlayTime = lastAudioPlayPts * 1000 *
                        audioStream->time_base.num /
                        audioStream->time_base.den;
    }
    playTimeLock.unlock();
}

int VideoPlayer::getVideoWidth() {
    if (videoRotate == 90 || videoRotate == 270) {
        return videoHeight;
    }
    return videoWidth;
}

int VideoPlayer::getVideoHeight() {
    if (videoRotate == 90 || videoRotate == 270) {
        return videoWidth;
    }
    return videoHeight;
}

void VideoPlayer::onVideoPlayCompletion(bool isPlayCompletion) {
    if (nullptr != videoPlayerInitParams &&
        nullptr != videoPlayerInitParams->onVideoPlayCompletionCallBack) {
        videoPlayerInitParams->onVideoPlayCompletionCallBack(
                videoPlayerInitParams->methodHandle, isPlayCompletion ? 1 : 0);

    }
}

bool VideoPlayer::lastAVFrameEnable() {
    return nullptr != lastDrawAVFrame && nullptr != lastDrawAVFrame->linesize
           && nullptr != lastDrawAVFrame->data[0]
           && nullptr != lastDrawAVFrame->data[1];
}

VideoPlayerInitParams *VideoPlayer::getVideoPlayerInitParams() {
    return videoPlayerInitParams;
}

bool VideoPlayer::isPlaying() {
    return !videoPlayerIsPause && !audioPlayIsPause;
}


