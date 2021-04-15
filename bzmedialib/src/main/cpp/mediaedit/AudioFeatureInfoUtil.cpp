//
/**
 * Created by bookzhan on 2018-09-03 10:10.
 * 说明:
 */
//
extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
}

#include "AudioFeatureInfoUtil.h"
#include "VideoUtil.h"

int AudioFeatureInfoUtil::getAudioFeatureInfo(const char *audioPath, int samples,
                                              void (*featureCallBack)(int64_t, int64_t, float)) {
    if (nullptr == audioPath || samples <= 0) {
        BZLogUtil::logE("nullptr == audioPath || samples <= 0");
        return -1;
    }
    AVFormatContext *avFormatContext = nullptr;
    int ret = VideoUtil::openInputFileForSoft(audioPath, &avFormatContext);
    if (ret < 0 || nullptr == avFormatContext) {
        BZLogUtil::logD("openInputFile %s fail", audioPath);
        return -1;
    }
    AVStream *audioStream = nullptr;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVStream *in_Stream = avFormatContext->streams[i];
        if (in_Stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = in_Stream;
        }
    }
    if (nullptr == audioStream) {
        BZLogUtil::logD("未检测到音频流");
        return -1;
    }
    SwrContext *swr_audio_ctx = swr_alloc();
    if (!swr_audio_ctx) {
        BZLogUtil::logD("Could not allocate resampler context\n");
        return -1;
    }
    int NB_SAMPLES = 1024;

    /* set options */
    av_opt_set_int(swr_audio_ctx, "out_channel_count", 1, 0);
    av_opt_set_int(swr_audio_ctx, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(swr_audio_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    av_opt_set_int(swr_audio_ctx, "in_channel_count", audioStream->codec->channels, 0);
    av_opt_set_int(swr_audio_ctx, "in_sample_rate", audioStream->codec->sample_rate, 0);
    av_opt_set_sample_fmt(swr_audio_ctx, "in_sample_fmt", audioStream->codec->sample_fmt, 0);

    /* initialize the resampling context */
    if ((ret = swr_init(swr_audio_ctx)) < 0) {
        BZLogUtil::logD("Failed to initialize the resampling context\n");
        return ret;
    }
    AVFrame *audioFrame = av_frame_alloc();
    AVCodecContext *audioCodecContext = audioStream->codec;
    AVAudioFifo *audioFifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_S16, 1,
                                                 NB_SAMPLES);


    int got_picture_ptr = 0;
    AVFrame *audioFrameTemp = VideoUtil::allocAudioFrame(AV_SAMPLE_FMT_S16,
                                                         AV_CH_LAYOUT_MONO,
                                                         44100, NB_SAMPLES);
    AVPacket *decode_pkt = nullptr;
    int errorCount = 0;
    int64_t index = 0;
    while (true) {
//        av_seek_frame(avFormatContext, audioStream->index, index, AVSEEK_FLAG_BACKWARD);
        if (decode_pkt != nullptr) {
            av_packet_free(&decode_pkt);
            decode_pkt = nullptr;
        }
        decode_pkt = av_packet_alloc();
        av_init_packet(decode_pkt);
        ret = av_read_frame(avFormatContext, decode_pkt);
        if (ret < 0) {
            break;
        }
        AVStream *avStream = avFormatContext->streams[decode_pkt->stream_index];
        if (avStream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        index++;
        if (samples > 0 && index % samples != 0) {
            continue;
        }
        ret = avcodec_decode_audio4(audioCodecContext, audioFrame, &got_picture_ptr,
                                    decode_pkt);
        if (ret < 0) {
            errorCount++;
            if (errorCount > 10) {
                break;
            }
            BZLogUtil::logD("avcodec_decode_video2 fail");
            continue;
        }
        if (!got_picture_ptr) {
            BZLogUtil::logD("VideoPlayer avcodec_decode_audio4 got_picture_ptr fail continue");
            continue;
        }
        //转换格式
        int samplesCount = swr_convert(swr_audio_ctx, audioFrameTemp->data,
                                       audioFrameTemp->nb_samples,
                                       (const uint8_t **) audioFrame->data,
                                       audioFrame->nb_samples);
//        BZLogUtil::logE("videoPlayTime=%lld--audioPlayTime=%lld", videoPlayTime, audioPlayTime);

        av_audio_fifo_write(audioFifo, (void **) &audioFrameTemp->data[0], samplesCount);
        while (av_audio_fifo_size(audioFifo) >= NB_SAMPLES) {
            av_audio_fifo_read(audioFifo, (void **) &audioFrameTemp->data[0],
                               NB_SAMPLES);

            if (nullptr != featureCallBack && 0 != callBackHandle) {
                int64_t audioPlayTime = audioCodecContext->pts_correction_last_pts * 1000 *
                                        audioStream->time_base.num /
                                        audioStream->time_base.den;

                short *pShort = reinterpret_cast<short *>(audioFrameTemp->data[0]);

                float audioFeature = (float) (*pShort) / 1000;
                featureCallBack(callBackHandle, audioPlayTime, audioFeature);
            }
        }
    }
    swr_free(&swr_audio_ctx);
    avcodec_close(audioCodecContext);
    avformat_close_input(&avFormatContext);
    av_frame_free(&audioFrame);
    av_frame_free(&audioFrameTemp);

    if (decode_pkt != nullptr) {
        av_packet_free(&decode_pkt);
        decode_pkt = nullptr;
    }
    return 0;
}

void AudioFeatureInfoUtil::setCallBackHandle(int64_t callBackHandle) {
    this->callBackHandle = callBackHandle;
}

int64_t AudioFeatureInfoUtil::getCallBackHandle() {
    return callBackHandle;
}
