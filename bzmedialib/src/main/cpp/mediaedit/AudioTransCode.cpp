//
/**
*Created by bookzhan on 2022−04-09 15:16.
*description:
*/
//

#include "AudioTransCode.h"
#include "BZLogUtil.h"
#include "VideoUtil.h"

int AudioTransCode::startTransCode(const char *inputPath, const char *outputPath,
                                   int64_t methodInfoHandle,
                                   const char *(*pcmCallBack)(int64_t, const char *, int),
                                   void (*progressCallBack)(int64_t, float)) {
    if (nullptr == inputPath || nullptr == outputPath) {
        BZLogUtil::logE("nullptr==inputPath|| nullptr==outputPath");
        return -1;
    }
    if (VideoUtil::openInputFileForSoft(inputPath, &in_fmt_ctx, false, true) < 0) {
        BZLogUtil::logE("openInputFile fail %s", inputPath);
        return -1;
    }
    if (nullptr == in_fmt_ctx) {
        BZLogUtil::logE("nullptr==in_fmt_ctx");
        return -1;
    }
    AVStream *audioStream = nullptr;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = in_fmt_ctx->streams[i];
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = in_stream;
            break;
        }
    }
    if (nullptr == audioStream) {
        BZLogUtil::logE("nullptr==audioStream");
        return -1;
    }
    int ret = 0;
    BZLogUtil::logD("---------------------------------------");
    if ((ret = openOutputFile(outputPath)) < 0) {
        BZLogUtil::logE("openOutputFile fail");
        releaseResource();
        return ret;
    }
    AVPacket *decode_pkt = nullptr, *encode_pkt = av_packet_alloc(), *final_pkt = nullptr;
    AVStream *in_stream;
    int got_picture_ptr = 0, videoFrameCount = 0;
    while (true) {
        if (nullptr != decode_pkt) {
            av_packet_free(&decode_pkt);
        }
        decode_pkt = av_packet_alloc();
        av_init_packet(decode_pkt);
        av_init_packet(encode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);
        if (ret < 0) {
            BZLogUtil::logD("av_read_frame end");
            break;
        }
        in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];
        if (in_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        ret = avcodec_decode_audio4(in_stream->codec, audioFrame, &got_picture_ptr,
                                    decode_pkt);

        if (ret < 0) {
            BZLogUtil::logD("avcodec_decode_audio4 fail");
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        if (!got_picture_ptr) {
            BZLogUtil::logD("avcodec_decode_audio4 got_picture_ptr fail continue");
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        //转换格式
        int samplesCount = swr_convert(swr_audio_ctx_2_pcm, audioFramePcm->data,
                                       audioFramePcm->nb_samples,
                                       (const uint8_t **) audioFrame->data,
                                       audioFrame->nb_samples);


        av_audio_fifo_write(audioFifo, (void **) &audioFramePcm->data[0], samplesCount);
        const char *pcmData = NULL;
        if (av_audio_fifo_size(audioFifo) >= NB_SAMPLES) {
            av_audio_fifo_read(audioFifo, (void **) &audioFramePcm->data[0],
                               NB_SAMPLES);
            if (nullptr != pcmCallBack && methodInfoHandle != 0) {
                pcmData = pcmCallBack(methodInfoHandle, (const char *) audioFramePcm->data[0],
                                      audioFramePcm->linesize[0]);
            }
        } else {
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        if (nullptr == pcmData) {
            BZLogUtil::logE("pcmCallBack NULL==pcmData");
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        audioFramePcmTemp->data[0] = (uint8_t *) pcmData;
        ret = swr_convert(swr_audio_ctx_2_aac, audioFrameAac->data,
                          avAudioEncodeCodecContext->frame_size,
                          (const uint8_t **) audioFramePcmTemp->data,
                          avAudioEncodeCodecContext->frame_size);
        if (ret < 0) {
            BZLogUtil::logE("swr_convert ret<0 break");
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        //编码
        audioFrameAac->pts = decode_pkt->pts;
        audioFrameAac->pkt_dts = decode_pkt->dts;

        ret = avcodec_encode_audio2(avAudioEncodeCodecContext, encode_pkt,
                                    audioFrameAac,
                                    &got_picture_ptr);
        if (ret < 0) {
            BZLogUtil::logE("avcodec_encode_audio2 Failed to encode!\n");
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        if (got_picture_ptr != 1) {
            BZLogUtil::logD("got_packet_ptr!=1");
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        final_pkt = encode_pkt;
        final_pkt->pts = decode_pkt->pts;
        final_pkt->duration = decode_pkt->duration;
        final_pkt->dts = decode_pkt->dts;
        final_pkt->pos = -1;
        free((void *) pcmData);

        float tempProgress =1.0f * final_pkt->pts / audioStream->duration;
        if (nullptr != progressCallBack && methodInfoHandle != 0) {//防止频繁回调
            progressCallBack(methodInfoHandle, tempProgress);
        }

        final_pkt->stream_index = 0;
        ret = av_interleaved_write_frame(out_fmt_ctx, final_pkt);
        if (ret < 0) {
            BZLogUtil::logE("Error muxing packet\n");
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        av_packet_unref(decode_pkt);
        av_packet_unref(encode_pkt);
    }
    av_packet_free(&encode_pkt);
    if (nullptr != out_fmt_ctx) {
        ret = av_write_trailer(out_fmt_ctx);
        if (ret != 0) {
            BZLogUtil::logE("av_write_trailer fail");
        }
    }
    BZLogUtil::logD("VideoTransCoding end");
    releaseResource();
    return ret;
}

int AudioTransCode::openOutputFile(const char *outPath) {
    if (nullptr == in_fmt_ctx) {
        BZLogUtil::logE("nullptr==in_fmt_ctx");
        return -1;
    }
    int ret = 0;
    ret = avformat_alloc_output_context2(&out_fmt_ctx, nullptr, nullptr, outPath);
    if (ret < 0) {
        BZLogUtil::logE("alloc_output_context2 fail");
        return ret;
    }
    if (nullptr == out_fmt_ctx) {
        BZLogUtil::logE("nullptr==out_fmt_ctx");
        return -1;
    }
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = in_fmt_ctx->streams[i];
        if (in_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        AVStream *out_stream = avformat_new_stream(out_fmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            BZLogUtil::logD("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return ret;
        }
        out_stream->time_base = (AVRational) {in_stream->time_base.num,
                                              in_stream->time_base.den};

        out_stream->duration = in_stream->duration;
        out_stream->nb_frames = in_stream->nb_frames;
        out_stream->id = in_stream->id;
        out_fmt_ctx->duration = in_fmt_ctx->duration;

        swr_audio_ctx_2_pcm = swr_alloc();
        if (!swr_audio_ctx_2_pcm) {
            BZLogUtil::logD("Could not allocate resampler context\n");
            return -1;
        }

        /* set options */
        av_opt_set_int(swr_audio_ctx_2_pcm, "out_channel_count", 1, 0);
        av_opt_set_int(swr_audio_ctx_2_pcm, "out_sample_rate", 44100, 0);
        av_opt_set_sample_fmt(swr_audio_ctx_2_pcm, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

        av_opt_set_int(swr_audio_ctx_2_pcm, "in_channel_count", in_stream->codec->channels, 0);
        av_opt_set_int(swr_audio_ctx_2_pcm, "in_sample_rate", in_stream->codec->sample_rate, 0);
        av_opt_set_sample_fmt(swr_audio_ctx_2_pcm, "in_sample_fmt",
                              in_stream->codec->sample_fmt, 0);

        /* initialize the resampling context */
        if ((ret = swr_init(swr_audio_ctx_2_pcm)) < 0) {
            BZLogUtil::logD("Failed to initialize the resampling context\n");
            return ret;
        }
        /* create resampler context */
        swr_audio_ctx_2_aac = swr_alloc();
        if (!swr_audio_ctx_2_aac) {
            BZLogUtil::logD("Could not allocate resampler context\n");
            return -1;
        }
        /* set options */
        av_opt_set_int(swr_audio_ctx_2_aac, "in_channel_count", 1, 0);
        av_opt_set_int(swr_audio_ctx_2_aac, "in_sample_rate", 44100, 0);
        av_opt_set_sample_fmt(swr_audio_ctx_2_aac, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);

        av_opt_set_int(swr_audio_ctx_2_aac, "out_channel_count",
                       av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO), 0);
        av_opt_set_int(swr_audio_ctx_2_aac, "out_sample_rate", 44100,
                       0);
        av_opt_set_sample_fmt(swr_audio_ctx_2_aac, "out_sample_fmt",
                              in_stream->codec->sample_fmt, 0);
        /* initialize the resampling context */
        if ((ret = swr_init(swr_audio_ctx_2_aac)) < 0) {
            BZLogUtil::logD("Failed to initialize the resampling context\n");
            return ret;
        }

        /* find the encoder */
        AVCodec *avCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (!avCodec) {
            BZLogUtil::logD("Could not find encoder for '%s'\n",
                            avcodec_get_name(AV_CODEC_ID_AAC));
            return -1;
        }
        avAudioEncodeCodecContext = avcodec_alloc_context3(avCodec);
        if (!avAudioEncodeCodecContext) {
            BZLogUtil::logD("Could not alloc an encoding context\n");
            return -1;
        }
        avAudioEncodeCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
        avAudioEncodeCodecContext->bit_rate = 128000;
        avAudioEncodeCodecContext->sample_rate = 44100;
        avAudioEncodeCodecContext->channel_layout = AV_CH_LAYOUT_MONO;
        avAudioEncodeCodecContext->channels = av_get_channel_layout_nb_channels(
                avAudioEncodeCodecContext->channel_layout);

        if (out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            avAudioEncodeCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        ret = avcodec_open2(avAudioEncodeCodecContext, avCodec, NULL);
        if (ret < 0) {
            BZLogUtil::logD("Could not open audio codec: %s\n", av_err2str(ret));
            return -1;
        }
        avcodec_parameters_from_context(out_stream->codecpar, avAudioEncodeCodecContext);
        out_stream->codec = avAudioEncodeCodecContext;
        audioFrame = av_frame_alloc();
        audioFramePcm = VideoUtil::allocAudioFrame(AV_SAMPLE_FMT_S16,
                                                   AV_CH_LAYOUT_MONO,
                                                   44100, NB_SAMPLES);

        audioFramePcmTemp = av_frame_alloc();
        audioFramePcmTemp->linesize[0] = audioFramePcm->linesize[0];
        audioFramePcmTemp->format = audioFramePcm->format;
        audioFramePcmTemp->channel_layout = audioFramePcm->channel_layout;
        audioFramePcmTemp->sample_rate = audioFramePcm->sample_rate;
        audioFramePcmTemp->nb_samples = audioFramePcm->nb_samples;

        audioFrameAac = VideoUtil::allocAudioFrame(AV_SAMPLE_FMT_FLTP,
                                                   AV_CH_LAYOUT_MONO,
                                                   44100, NB_SAMPLES);
        audioFifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_S16, 1,
                                        NB_SAMPLES);
        break;
    }
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_fmt_ctx->pb, outPath, AVIO_FLAG_WRITE);
        if (ret < 0) {
            BZLogUtil::logD("Could not open '%s': %s\n", outPath,
                            av_err2str(ret));
            return ret;
        }
    }
    /* Write the stream header, if any. */
    ret = avformat_write_header(out_fmt_ctx, nullptr);
    if (ret < 0) {
        BZLogUtil::logD("Error occurred when opening output file: %s\n",
                        av_err2str(ret));
        return ret;
    }
    return 0;
}

int AudioTransCode::releaseResource() {
    BZLogUtil::logD("AudioTransCode releaseResource");
    if (nullptr != in_fmt_ctx) {
        for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
            avcodec_close(in_fmt_ctx->streams[i]->codec);
        }
        avformat_close_input(&in_fmt_ctx);
        in_fmt_ctx = nullptr;
    }
    if (nullptr != out_fmt_ctx) {
        for (int i = 0; i < out_fmt_ctx->nb_streams; i++) {
            avcodec_close(out_fmt_ctx->streams[i]->codec);
        }
        avformat_close_input(&out_fmt_ctx);
        out_fmt_ctx = nullptr;
    }
    if (nullptr != audioFrame) {
        av_frame_free(&audioFrame);
        audioFrame = nullptr;
    }
    if (nullptr != audioFrameAac) {
        av_frame_free(&audioFrameAac);
        audioFrameAac = nullptr;
    }
    if (nullptr != audioFramePcm) {
        av_frame_free(&audioFramePcm);
        audioFramePcm = nullptr;
    }
    if (nullptr != audioFramePcmTemp) {
        av_frame_free(&audioFramePcmTemp);
        audioFramePcmTemp = nullptr;
    }
    if (nullptr != swr_audio_ctx_2_pcm) {
        swr_free(&swr_audio_ctx_2_pcm);
        swr_audio_ctx_2_pcm = nullptr;
    }
    if (nullptr != audioFifo) {
        av_audio_fifo_free(audioFifo);
        audioFifo = nullptr;
    }
    return 0;
}

