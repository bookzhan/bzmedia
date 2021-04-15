//
/**
 * Created by bookzhan on 2020-08-10 15:47.
 *description:
 */
//

#include <mediaedit/VideoUtil.h>
#include <thread>
#include <common/bz_time.h>
#include "AudioPlayer.h"
#include "PCMPlayerNative.h"

int AudioPlayer::init(const char *audioPath, int64_t methodHandle,
                      void (*progressCallBack)(int64_t, float)) {
    if (nullptr == audioPath) {
        return -1;
    }
    BZLogUtil::logD("AudioPlayer::init audioPath=%s", audioPath);
    this->methodHandle = methodHandle;
    this->progressCallBack = progressCallBack;
    int ret = VideoUtil::openInputFileForSoft(audioPath, &in_fmt_ctx, false, true);
    if (ret < 0) {
        BZLogUtil::logE("openInputFile fail %s", audioPath);
        in_fmt_ctx = nullptr;
        release();
        return ret;
    }
    if (nullptr == in_fmt_ctx) {
        BZLogUtil::logE("nullptr==in_fmt_ctx");
        return -1;
    }
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = in_fmt_ctx->streams[i];
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
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
            av_opt_set_int(swr_audio_ctx, "out_channel_count", in_stream->codecpar->channels, 0);
            av_opt_set_int(swr_audio_ctx, "out_sample_rate", in_stream->codecpar->sample_rate, 0);
            av_opt_set_sample_fmt(swr_audio_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

            av_opt_set_int(swr_audio_ctx, "in_channel_count", in_stream->codecpar->channels, 0);
            av_opt_set_int(swr_audio_ctx, "in_sample_rate", in_stream->codecpar->sample_rate,
                           0);
            av_opt_set_sample_fmt(swr_audio_ctx, "in_sample_fmt",
                                  static_cast<AVSampleFormat>(in_stream->codecpar->format),
                                  0);
            if (nullptr != pcmPlayer) {
                pcmPlayer->init(in_stream->codecpar->sample_rate, in_stream->codecpar->channels);
            }
            /* initialize the resampling context */
            if ((ret = swr_init(swr_audio_ctx)) < 0) {
                BZLogUtil::logD("Failed to initialize the resampling context\n");
                return ret;
            }
            audioFrame = av_frame_alloc();
            audioCodecContext = in_stream->codec;
        }
    }
    if (nullptr == audioStream) {
        BZLogUtil::logE("nullptr==audioStream");
        release();
        return -1;
    }


    BZLogUtil::logD("audioStreamTotalTime=%lld", audioStreamTotalTime);

    audioPlayThreadIsEnd = false;
    thread threadPlayAudio(&AudioPlayer::audioPlayThread, this);
    threadPlayAudio.detach();
    return 0;
}

void AudioPlayer::audioPlayThread() {
    BZLogUtil::logD("audioPlayThread start");
    std::chrono::milliseconds dura(30);
    int ret = 0, got_picture_ptr = 0;
    int bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

    AVFrame *audioFrameTemp = VideoUtil::allocAudioFrame(
            AV_SAMPLE_FMT_S16,
            static_cast<uint64_t>(audioStream->codecpar->channels),
            audioStream->codecpar->sample_rate, FRAME_SIZE);
    int errorCount = 0;
    AVPacket *decode_pkt = av_packet_alloc();
    while (true) {
        if (videoPlayerIsRelease) {
            BZLogUtil::logD("audioPlayThread videoPlayerIsRelease break");
            break;
        }
        logCount++;
        if (playerIsPause) {
            if (logCount % LOG_SPACE == 0) {
                BZLogUtil::logV("---playerIsPause---");
            }
            std::this_thread::sleep_for(dura);
            continue;
        }
        av_init_packet(decode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0) {
            av_packet_free(&decode_pkt);
            if (isLoop && nullptr != in_fmt_ctx) {
                av_seek_frame(in_fmt_ctx, audioStream->index, 0, AVSEEK_FLAG_BACKWARD);
                continue;
            }
            break;
        }
        if (nullptr == decode_pkt) {
            continue;
        }
        if (requestSetVolume && nullptr != pcmPlayer) {
            pcmPlayer->setVideoPlayerVolume(volume);
            requestSetVolume = false;
        }
        if (requestSeek) {
            if (nullptr != audioCodecContext) {
                audioCodecLock.lock();
                avcodec_flush_buffers(audioCodecContext);
                audioCodecLock.unlock();
            }
            if (nullptr != in_fmt_ctx) {
                av_seek_frame(in_fmt_ctx, audioStream->index, seekAudioPts, AVSEEK_FLAG_BACKWARD);
            }
            requestSeek = false;
            continue;
        }
        //回调进度
        if (audioStream->duration > 0) {
            float progress = 1.0f * audioCodecContext->pts_correction_last_pts /
                             audioStream->duration;
            callBackProgress(progress);
        }
//        int64_t startTime = getMicrosecondTime();
        ret = avcodec_decode_audio4(audioCodecContext, audioFrame, &got_picture_ptr,
                                    decode_pkt);
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
        int samplesCount = swr_convert(swr_audio_ctx, audioFrameTemp->data,
                                       audioFrameTemp->nb_samples,
                                       (const uint8_t **) audioFrame->data,
                                       audioFrame->nb_samples);
//        BZLogUtil::logD("samplesCount=%d", samplesCount);
//        BZLogUtil::logD("avcodec_decode_audio4 耗时=%lld", getMicrosecondTime() - startTime);
        onPCMDataAvailable((const char *) audioFrameTemp->data[0],
                           audioFrameTemp->linesize[0] * bytes_per_sample);
        audioPlayTime = audioCodecContext->pts_correction_last_pts * 1000 *
                        audioStream->time_base.num /
                        audioStream->time_base.den;
        av_frame_unref(audioFrame);
//        BZLogUtil::logE("videoPlayTime=% lld--audioPlayTime=%lld", videoPlayTime, audioPlayTime);
    }
    av_packet_free(&decode_pkt);
    av_frame_free(&audioFrameTemp);
    if (audioStream->duration > 0) {
        callBackProgress(1);
    }
    audioPlayThreadIsEnd = true;
    BZLogUtil::logD("audioPlayThread end");
}

int AudioPlayer::seek(int64_t time) {
    BZLogUtil::logD("seek time=%lld", time);
    if (nullptr != audioStream) {
        seekAudioPts = time * audioStream->time_base.den / (1000 * audioStream->time_base.num);
    }
    requestSeek = true;
    return 0;
}

int AudioPlayer::setPlayLoop(bool isLoop) {
    this->isLoop = isLoop;
    return 0;
}

void AudioPlayer::setAudioVolume(float volume) {
    this->volume = volume;
    requestSetVolume = true;
}

int64_t AudioPlayer::getDuration() {
    return audioStreamTotalTime;
}

int AudioPlayer::pause() {
    playerIsPause = true;
    if (nullptr != pcmPlayer) {
        pcmPlayer->pause();
    }
    return 0;
}

int AudioPlayer::start() {
    playerIsPause = false;
    if (nullptr != pcmPlayer) {
        pcmPlayer->start();
    }
    return 0;
}

int AudioPlayer::release() {
    videoPlayerIsRelease = true;
    std::chrono::milliseconds dura(30);
    while (!audioPlayThreadIsEnd) {
        BZLogUtil::logV("!audioPlayThreadIsEnd");
        std::this_thread::sleep_for(dura);
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
    audioCodecContext = nullptr;
    audioStream = nullptr;

    if (nullptr != audioFifo) {
        av_audio_fifo_free(audioFifo);
        audioFifo = nullptr;
    }

    if (nullptr != audioFrame) {
        av_frame_free(&audioFrame);
        audioFrame = nullptr;
    }
    if (nullptr != swr_audio_ctx) {
        swr_free(&swr_audio_ctx);
        swr_audio_ctx = nullptr;
    }
    return 0;
}


void AudioPlayer::onPCMDataAvailable(const char *pcmData, int length) {
    if (nullptr != pcmPlayer) {
        pcmPlayer->onPCMDataAvailable(pcmData, length);
    }
}

void AudioPlayer::callBackProgress(float progress) {
    if (nullptr != progressCallBack) {
        progressCallBack(methodHandle, progress);
    }
}

AudioPlayer::~AudioPlayer() {
    BZLogUtil::logD("~AudioPlayer");
    if (nullptr != pcmPlayer) {
        pcmPlayer->stopAudioTrack();
        delete pcmPlayer;
        pcmPlayer = nullptr;
    }
}

AudioPlayer::AudioPlayer() {
    pcmPlayer = new PCMPlayerNative();
}
