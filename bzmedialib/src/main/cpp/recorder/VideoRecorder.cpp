/**
 * Created by zhandalin on 2017-03-29 17:55.
 * 说明:视频录制
 */
#include <common/BZLogUtil.h>
#include <bean/FilteringContext.h>
#include <common/bzcommon.h>
#include "VideoRecorder.h"
#include "h264_util.h"


int VideoRecorder::getVideoOutputWidth() {
    return targetWidth;
}

int VideoRecorder::getVideoOutputHeight() {
    return targetHeight;
}


int64_t VideoRecorder::getRecordTime() {
    return recorderTime;
}

VideoRecorder::VideoRecorder() {
    video_st = new BZOutputStream();
    audio_st = new BZOutputStream();
    videoWriteMutex = new mutex();
    recorderTime = 0;
}

int VideoRecorder::startRecord(VideoRecordParams videoRecordParams) {
    BZLogUtil::logD(
            "startRecord output_path=%s--srcWidth=%d--srcHeight=%d--targetWidth=%d--targetHeight=%d \n"
            "rate=%d--nb_samples=%d--sampleRate=%d--videoRotate=%d--pixelFormat=%d\n"
            "allFrameIsKey=%d,synEncode=%d,avPacketFromMediaCodec=%d",
            videoRecordParams.output_path, videoRecordParams.inputWidth,
            videoRecordParams.inputHeight,
            videoRecordParams.targetWidth, videoRecordParams.targetHeight,
            videoRecordParams.videoFrameRate,
            videoRecordParams.nbSamples,
            videoRecordParams.sampleRate, videoRecordParams.videoRotate,
            videoRecordParams.pixelFormat,
            videoRecordParams.allFrameIsKey, videoRecordParams.synEncode,
            videoRecordParams.avPacketFromMediaCodec);

    videoRecordParams.targetWidth = videoRecordParams.targetWidth / 16 * 16;
    videoRecordParams.inputWidth = videoRecordParams.inputWidth / 16 * 16;

    videoRecordParams.targetHeight = videoRecordParams.targetHeight / 16 * 16;
    videoRecordParams.inputHeight = videoRecordParams.inputHeight / 16 * 16;

    BZLogUtil::logD("VideoRecorder 对齐后targetWidth=%d--targetHeight=%d",
                    videoRecordParams.targetWidth,
                    videoRecordParams.targetHeight);
    if (videoRecordParams.targetHeight % 2 != 0)
        videoRecordParams.targetHeight -= 1;

    if (videoRecordParams.inputHeight % 2 != 0)
        videoRecordParams.inputHeight -= 1;

    this->srcWidth = videoRecordParams.inputWidth;
    this->srcHeight = videoRecordParams.inputHeight;
    this->targetWidth = videoRecordParams.targetWidth;

    this->targetHeight = videoRecordParams.targetHeight;
    this->videoRate = videoRecordParams.videoFrameRate;
    this->nbSamples = videoRecordParams.nbSamples;
    this->sampleRate = videoRecordParams.sampleRate;
    this->videoRotate = videoRecordParams.videoRotate;
    this->pixelFormat = videoRecordParams.pixelFormat;
    this->allFrameIsKey = videoRecordParams.allFrameIsKey;
    this->bit_rate = videoRecordParams.bit_rate;
    this->videoPts = videoRecordParams.videoPts;
    this->audioPts = videoRecordParams.audioPts;
    this->synEncode = videoRecordParams.synEncode;
    this->avPacketFromMediaCodec = videoRecordParams.avPacketFromMediaCodec;

    y_size = targetWidth * targetHeight;

    output_path = new string();
    output_path->append(videoRecordParams.output_path);

    int ret = 0;
    ret = avformat_alloc_output_context2(&avFormatContext, NULL, NULL,
                                         videoRecordParams.output_path);
    if (ret < 0) {
        BZLogUtil::logE("VideoRecorder alloc_output_context2 fail");
//        endRecordAndReleaseResource();
        return ret;
    }
    if (!avFormatContext)
        return -1;
    AVOutputFormat *oformat = avFormatContext->oformat;
    if (oformat->video_codec != AV_CODEC_ID_NONE) {
        ret = addStream(video_st, AVMEDIA_TYPE_VIDEO, AV_CODEC_ID_H264);
        if (ret < 0) {
            BZLogUtil::logE("VideoRecorder addStream video_st fail");
            endRecordAndReleaseResource();
            return ret;
        }
    }
    if (oformat->audio_codec != AV_CODEC_ID_NONE && videoRecordParams.hasAudio) {
        ret = addStream(audio_st, AVMEDIA_TYPE_AUDIO, AV_CODEC_ID_AAC);
        if (ret < 0) {
            BZLogUtil::logE("VideoRecorder addStream audio_st fail");
            endRecordAndReleaseResource();
            return ret;
        }
    }
    if (video_st->avCodecContext) {
        ret = openVideo(video_st, nullptr);
        if (ret < 0) {
            BZLogUtil::logE("VideoRecorder open_video fail");
            endRecordAndReleaseResource();
            return ret;
        }
    }
    if (audio_st->avCodecContext && videoRecordParams.hasAudio) {
        ret = openAudio(audio_st);
        if (ret < 0) {
            BZLogUtil::logE("VideoRecorder open_audio fail");
            endRecordAndReleaseResource();
            return ret;
        }
    }
    av_dump_format(avFormatContext, 0, videoRecordParams.output_path, 1);

    /* open the output file, if needed */
    if (!(oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&avFormatContext->pb, videoRecordParams.output_path, AVIO_FLAG_WRITE);
        if (ret < 0) {
            BZLogUtil::logD("VideoRecorder Could not open '%s': %s\n",
                            videoRecordParams.output_path,
                            av_err2str(ret));
            endRecordAndReleaseResource();
            return ret;
        }
    }
    /* Write the stream header, if any. */
    ret = avformat_write_header(avFormatContext, NULL);
    if (ret < 0) {
        BZLogUtil::logD("VideoRecorder Error occurred when opening output file: %s\n",
                        av_err2str(ret));
        endRecordAndReleaseResource();
        return ret;
    }

    recorderStartTime = getCurrentTime();
    isStopRecorder = false;
    if (pixelFormat == PixelFormat::TEXTURE) {
        textureConvertYUVUtil = new TextureConvertYUVUtil();
        textureConvertYUVUtil->init(targetWidth, targetHeight);
    }
    if (!synEncode) {
        encodeThreadIsRunning = true;
        thread threadEncode(&VideoRecorder::encodeThread, this);
        threadEncode.detach();
    }
    BZLogUtil::logD("VideoRecorder start startRecord finish");
    return 0;
}

void VideoRecorder::flushBuffer() {
    BZLogUtil::logV("VideoRecorder ------flush_video start-------");
    int got_picture = 0, ret = 0;
    int64_t startTime = 0;
    while (true) {
        av_init_packet(video_st->avPacket);
        startTime = getCurrentTime();
        //Encode
        ret = avcodec_encode_video2(video_st->avCodecContext, video_st->avPacket, NULL,
                                    &got_picture);
        if (ret < 0 || !got_picture) {
            BZLogUtil::logV("VideoRecorder -----flush_video end----");
            break;
        }
        int64_t time = getCurrentTime() - startTime;
        encodeTotalTime += time;
        BZLogUtil::logV("VideoRecorder flush_video avcodec_encode_video2 time cost=%lld", time);
        //防止最后保存的时候帧率大于30帧,只是适合外部传入PTS的情况,一般录制不会超过30帧
        video_st->avPacket->pts = 1;
        video_st->avPacket->dts = 1;
        writeVideoPacket(video_st->avPacket, got_picture, -1);
    }

//    BZLogUtil::logV("VideoRecorder ------flush_audio start-------");
//    while (true) {
//        if (NULL == audio_st)
//            break;
//
//        av_init_packet(audio_st->avPacket);
//        samples_count += audio_st->avCodecContext->frame_size;
//        //音频编码
//        ret = avcodec_encode_audio2(audio_st->avCodecContext, audio_st->avPacket,
//                                    NULL,
//                                    &got_picture);
//        if (ret < 0 || !got_picture) {
//            BZLogUtil::logV("VideoRecorder -----flush_audio end----");
//            break;
//        }
//        BZLogUtil::logV("VideoRecorder -----flush_audio----");
//        audio_st->avPacket->stream_index = audio_st->avStream->index;
//        av_packet_rescale_ts(audio_st->avPacket, audio_st->avCodecContext->time_base,
//                             audio_st->avStream->time_base);
//
//        videoWriteMutex->lock();
//        ret = av_interleaved_write_frame(avFormatContext, audio_st->avPacket);
//        videoWriteMutex->unlock();
//
//        if (ret < 0) {
//            BZLogUtil::logE("VideoRecorder Failed to av_interleaved_write_frame!\n");
//            break;
//        }
//        av_packet_unref(audio_st->avPacket);
//    }
    endRecordAndReleaseResource();
    BZLogUtil::logD("VideoRecorder record --end--");

    VideoRecorder::recorderTime = 0;
}


int VideoRecorder::stopRecord() {
    BZLogUtil::logD("VideoRecorder start stopRecord");
    isStopRecorder = true;
    std::chrono::milliseconds dura(30);
    while (isAddAudioData || isAddVideoData) {
        BZLogUtil::logD("VideoRecorder stopRecord isAddAudioData || isAddVideoData wait...");
        std::this_thread::sleep_for(dura);
    }
    flushBuffer();
    BZLogUtil::logD("VideoRecorder stopRecord end");
    return 0;
}

VideoRecorder::~VideoRecorder() {
    BZLogUtil::logD("VideoRecorder ~VideoRecorderBase");
//    endRecordAndReleaseResource();
}

int VideoRecorder::endRecordAndReleaseResource() {
    if (yuvBufferTotalCount > 0) {
        int64_t totalTime = getCurrentTime() - recorderStartTime;
        BZLogUtil::logD(
                "endRecordAndReleaseResource--平均编码耗时=%lld--录制总耗时=%lld--传入总帧数=%d--帧率=%f--updateTextureCount=%d",
                encodeTotalTime / yuvBufferTotalCount, totalTime,
                yuvBufferTotalCount, 1.0f * yuvBufferTotalCount / (totalTime / 1000.0f),
                updateTextureCount);
    } else {
        BZLogUtil::logE("VideoRecorder yuvBufferTotalCount<=0");
    }

    if (NULL != videoWriteMutex) {
        videoWriteMutex->lock();
        av_interleaved_write_frame(avFormatContext, NULL);
        int ret = 0;
        if (NULL != avFormatContext && NULL != avFormatContext->pb) {
            ret = av_write_trailer(avFormatContext);
            if (ret != 0) {
                BZLogUtil::logE("VideoRecorder av_write_trailer fail");
            }
        }
        addVideoDataLock.lock();
        if (NULL != video_st) {
            closeStream(video_st);
            video_st = NULL;
        }
        addVideoDataLock.unlock();

        addAudioDataLock.lock();
        if (NULL != audio_st) {
            closeStream(audio_st);
            audio_st = NULL;
        }
        addAudioDataLock.unlock();

        if (NULL != avFormatContext) {
            /* Close the output file. */
            if (!(avFormatContext->oformat->flags & AVFMT_NOFILE))
                avio_closep(&avFormatContext->pb);
            //打开会报错，原因不明 是因为stream->avStream->codec = avCodecContext; 导致重复释放，新版本不需要这么指定了
            avformat_free_context(avFormatContext);
            avFormatContext = NULL;
        }

        if (NULL != audioFifo) {
            av_audio_fifo_free(audioFifo);
            audioFifo = NULL;
        }
        videoWriteMutex->unlock();
        delete videoWriteMutex;
        videoWriteMutex = NULL;
    }
    videoFrameCount = 0;
    if (nullptr != audioData) {
        delete (audioData);
        audioData = nullptr;
    }
    if (nullptr != videoPtsBuffer) {
        videoPtsBuffer->clear();
        delete (videoPtsBuffer);
        videoPtsBuffer = nullptr;
    }
    BZLogUtil::logD("VideoRecorder endRecordAndReleaseResource --success");
    return 0;
}

int VideoRecorder::closeStream(BZOutputStream *stream) {
    if (NULL == stream)return -1;
    if (NULL != stream->avStream)
        BZLogUtil::logD("VideoRecorder closeStream index=%d", stream->avStream->index);

    if (NULL != stream->avCodecContext) {
        avcodec_free_context(&stream->avCodecContext);
        stream->avCodecContext = NULL;
    }
    if (NULL != stream->frame) {
        av_frame_free(&stream->frame);
        stream->frame = NULL;
    }
    //只有视频的tmp_frame 才释放,可能是由于音频swr_audio_ctx 处理影响的,导致指针交换了
    if (NULL != stream->tmp_frame && NULL == stream->swr_audio_ctx) {
        av_frame_free(&stream->tmp_frame);
        stream->tmp_frame = NULL;
    }
    if (NULL != stream->filter_frame) {
        av_frame_free(&stream->filter_frame);
        stream->filter_frame = NULL;
    }
    //有的手机会崩溃
    if (NULL != stream->swr_audio_ctx) {
        swr_free(&stream->swr_audio_ctx);
        stream->swr_audio_ctx = NULL;
    }
    return 0;
}

int VideoRecorder::addStream(BZOutputStream *stream, AVMediaType mediaType, AVCodecID codec_id) {
    BZLogUtil::logD("VideoRecorder addStream mediaType=%d", mediaType);
    AVCodecContext *avCodecContext;
    /* find the encoder */
    stream->avCodec = avcodec_find_encoder(codec_id);
    if (!stream->avCodec) {
        BZLogUtil::logD("VideoRecorder Could not find encoder for '%s'\n",
                        avcodec_get_name(codec_id));
        return -1;
    }
    stream->avStream = avformat_new_stream(avFormatContext, NULL);
    if (!stream->avStream) {
        BZLogUtil::logD("VideoRecorder Could not allocate stream\n");
        return -1;
    }
    stream->avStream->id = avFormatContext->nb_streams - 1;
    avCodecContext = avcodec_alloc_context3(stream->avCodec);
    if (!avCodecContext) {
        BZLogUtil::logD("VideoRecorder Could not alloc an encoding context\n");
        return -1;
    }
    stream->avCodecContext = avCodecContext;

    switch (mediaType) {
        case AVMEDIA_TYPE_AUDIO:
            avCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
            avCodecContext->bit_rate = 128000;
            avCodecContext->sample_rate = sampleRate;
            avCodecContext->channel_layout = AV_CH_LAYOUT_MONO;
            avCodecContext->channels = av_get_channel_layout_nb_channels(
                    avCodecContext->channel_layout);
            stream->avStream->time_base = (AVRational) {1, avCodecContext->sample_rate};
            break;
        case AVMEDIA_TYPE_VIDEO:
            BZLogUtil::logD("VideoRecorder video bit_rate=%f", bit_rate / 1024.0f / 1024 / 8);

            avCodecContext->codec_id = codec_id;
            avCodecContext->bit_rate = bit_rate;
            avCodecContext->bit_rate_tolerance = (int) (bit_rate * 2);
            avCodecContext->width = this->targetWidth;
            avCodecContext->height = this->targetHeight;
            avCodecContext->time_base = (AVRational) {1,
                                                      videoRate};
            stream->avStream->time_base = AV_TIME_BASE_Q;

//            //这个不设置第一帧会花
//            avCodecContext->qmin = 5;
//            avCodecContext->qmax = 10;
//            avCodecContext->max_b_frames = 3;
            if (allFrameIsKey) {
                avCodecContext->gop_size = 1; /* emit one intra frame every twelve frames at most */
            } else {
                avCodecContext->gop_size = 30;
            }
            avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
            if (avCodecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                /* just for testing, we also add B-frames */
                avCodecContext->max_b_frames = 2;
            }
            if (avCodecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
                /* Needed to avoid using macroblocks in which some coeffs overflow.
                 * This does not happen with normal video, it just happens here as
                 * the motion of the chroma plane does not match the luma plane. */
                avCodecContext->mb_decision = 2;
            }
            if (videoRotate != 0) {
                char rotate_char[10] = {0};
                sprintf(rotate_char, "%d", videoRotate);
                av_dict_set(&stream->avStream->metadata, "rotate", rotate_char, 0);
            }
            break;
        default:
            break;
    }
//    stream->avStream->codec = avCodecContext;

    /* Some formats want stream headers to be separate. */
    if (avFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        avCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


    stream->avPacket = av_packet_alloc();
    stream->avPacket->dts = 0;
    stream->avPacket->dts = 0;
    stream->avPacket->size = 0;
    return 0;
}

int VideoRecorder::openVideo(BZOutputStream *stream, const char *extraFilterParam) {
    BZLogUtil::logD("VideoRecorder openVideo");
    int ret = 0;
    AVCodecContext *avCodecContext = stream->avCodecContext;

    //好像有用的
//    avCodecContext->thread_count = 2;

    AVDictionary *param = 0;
    av_dict_set(&param, "preset", "ultrafast", 0);
    av_dict_set(&param, "tune", "film", 0);
    av_dict_set(&param, "no-cabac", "1", 0);
    av_dict_set(&param, "no-deblock", "1", 0);
    //零延迟编码,但是有的播放器播放不了,直播适合打开
    //现在由于是自己打时间戳,所以如果有缓存的话就导致,视频开始时间就延后了,导致音视频不同步
    //这个参数会导致编码速度大幅降低,时间会加大4倍左右
//    av_dict_set(&param, "tune", "zerolatency", 0);

    stream->avCodec->capabilities = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_AUTO_THREADS;

    /* open the codec */
    ret = avcodec_open2(avCodecContext, stream->avCodec, &param);
    if (ret < 0) {
        BZLogUtil::logD("VideoRecorder avCodecContext Could not open video codec: %s\n",
                        av_err2str(ret));
        return -1;
    }
    av_dict_free(&param);

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(stream->avStream->codecpar, avCodecContext);
    if (ret < 0) {
        BZLogUtil::logD("VideoRecorder Could not copy the stream parameters\n");
        return -1;
    }


    /* allocate and init a re-usable frame */
    stream->frame = alloc_picture(avCodecContext->pix_fmt, this->targetWidth,
                                  this->targetHeight);

    if (!stream->frame) {
        BZLogUtil::logD("VideoRecorder Could not allocate video frame\n");
        return -1;
    }
    stream->tmp_frame = alloc_picture(avCodecContext->pix_fmt, this->targetWidth,
                                      this->targetHeight);
    if (!stream->tmp_frame) {
        BZLogUtil::logD("VideoRecorder Could not allocate temporary picture\n");
        return -1;
    }
    return ret;
}

int VideoRecorder::openAudio(BZOutputStream *stream) {
    BZLogUtil::logD("VideoRecorder openAudio");

    AVCodecContext *avCodecContext = stream->avCodecContext;
    int nb_samples;
    int ret;
    ret = avcodec_open2(avCodecContext, stream->avCodec, NULL);
    if (ret < 0) {
        BZLogUtil::logD("VideoRecorder Could not open audio codec: %s\n", av_err2str(ret));
        return -1;
    }
    if (avCodecContext->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = avCodecContext->frame_size;
    stream->frame = alloc_audio_frame(avCodecContext->sample_fmt, avCodecContext->channel_layout,
                                      avCodecContext->sample_rate, nb_samples);


    stream->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_MONO,
                                          this->sampleRate, this->nbSamples);

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(stream->avStream->codecpar, avCodecContext);
    if (ret < 0) {
        BZLogUtil::logD("VideoRecorder Could not copy the stream parameters\n");
        return -1;
    }

    /* create resampler context */
    stream->swr_audio_ctx = swr_alloc();
    if (!stream->swr_audio_ctx) {
        BZLogUtil::logD("VideoRecorder Could not allocate resampler context\n");
        return -1;
    }

    /* set options */
    av_opt_set_int(stream->swr_audio_ctx, "in_channel_count", 1, 0);
    av_opt_set_int(stream->swr_audio_ctx, "in_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(stream->swr_audio_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    av_opt_set_int(stream->swr_audio_ctx, "out_channel_count", avCodecContext->channels, 0);
    av_opt_set_int(stream->swr_audio_ctx, "out_sample_rate", avCodecContext->sample_rate, 0);
    av_opt_set_sample_fmt(stream->swr_audio_ctx, "out_sample_fmt", avCodecContext->sample_fmt, 0);

    /* initialize the resampling context */
    if ((ret = swr_init(stream->swr_audio_ctx)) < 0) {
        BZLogUtil::logD("VideoRecorder Failed to initialize the resampling context\n");
        return ret;
    }
    audioFifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_S16, 1,
                                    this->nbSamples);
    return 0;
}

AVFrame *VideoRecorder::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height) {
    BZLogUtil::logD("VideoRecorder alloc_picture");
    AVFrame *picture = NULL;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;
    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 4);
    if (ret < 0) {
        BZLogUtil::logE("VideoRecorder Could not allocate frame data.\n");
    }
    return picture;
}

AVFrame *VideoRecorder::alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                          uint64_t channel_layout,
                                          int sample_rate, int nb_samples) {
    BZLogUtil::logD("VideoRecorder alloc_audio_frame");
    AVFrame *frame = av_frame_alloc();
    int ret;
    if (!frame) {
        BZLogUtil::logD("VideoRecorder Error allocating an audio frame\n");
        return NULL;
    }
    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            BZLogUtil::logD("VideoRecorder Error allocating an audio buffer\n");
        }
    }
    return frame;
}


int64_t VideoRecorder::addAudioData(unsigned char *pIn, int frameSize, int64_t audioPts) {
    if (isStopRecorder || nullptr == audioFifo || NULL == audio_st || nullptr == audioData)
        return -1;
    if (avPacketFromMediaCodec && !isWriteHeaderSuccess) {
        return 0;
    }

//    BZLogUtil::logD("VideoRecorder start to encode_pcm_data..., frameSize is:%d", frameSize);
//    先缓存起来,直到够1024
    int ret = 0;
    addAudioDataLock.lock();
    isAddAudioData = true;
    audioData[0] = pIn;
    av_audio_fifo_write(audioFifo, reinterpret_cast<void **>(audioData), frameSize / 2);
    while (av_audio_fifo_size(audioFifo) >= audio_st->avCodecContext->frame_size) {
        av_audio_fifo_read(audioFifo, (void **) audio_st->tmp_frame->data,
                           audio_st->avCodecContext->frame_size);
        ret = swr_convert(audio_st->swr_audio_ctx, audio_st->frame->data,
                          audio_st->avCodecContext->frame_size,
                          (const uint8_t **) audio_st->tmp_frame->data,
                          audio_st->avCodecContext->frame_size);
        if (ret < 0) {
            BZLogUtil::logE("VideoRecorder swr_convert ret<0 break");
            break;
        }
        writeAudioFrame(audio_st->frame, audioPts);
    }
    isAddAudioData = false;
    addAudioDataLock.unlock();
    return recorderTime;
}

int VideoRecorder::writeAudioFrame(AVFrame *frame, int64_t audioPtsInput) {
    if (NULL == frame || NULL == audio_st || NULL == audio_st->avCodecContext)
        return -1;

    av_init_packet(audio_st->avPacket);
    int encode_ret = 0;
    int got_packet_ptr = 0;
    frame->pts = av_rescale_q(samples_count,
                              (AVRational) {1, audio_st->avCodecContext->sample_rate},
                              audio_st->avCodecContext->time_base);

    samples_count += frame->nb_samples;
    //音频编码
    encode_ret = avcodec_encode_audio2(audio_st->avCodecContext, audio_st->avPacket,
                                       frame,
                                       &got_packet_ptr);
    if (encode_ret < 0) {
        BZLogUtil::logE("VideoRecorder Failed to encode!\n");
        return encode_ret;
    }
    if (got_packet_ptr != 1) {
        BZLogUtil::logD("VideoRecorder got_packet_ptr!=1");
        return got_packet_ptr;
    }
    audio_st->avPacket->stream_index = audio_st->avStream->index;
    if (audioPtsInput >= 0) {
        audio_st->avPacket->pts = audioPtsInput;
        audio_st->avPacket->dts = audioPtsInput;
    } else {
        av_packet_rescale_ts(audio_st->avPacket, audio_st->avCodecContext->time_base,
                             audio_st->avStream->time_base);

        if (nullptr != audioPts && !audioPts->empty()) {
            audio_st->avPacket->pts = audioPts->front();
            audio_st->avPacket->dts = audioPts->front();
            audioPts->pop_front();
        }
    }
    recorderTime = (long) (audio_st->avPacket->pts * av_q2d(audio_st->avStream->time_base) * 1000);
//    BZLogUtil::logE("VideoRecorder recordTime=%ld", recordTime);

    audio_st->last_pts = audio_st->avPacket->pts;

    videoWriteMutex->lock();
    encode_ret = av_interleaved_write_frame(avFormatContext, audio_st->avPacket);
    videoWriteMutex->unlock();

    if (encode_ret < 0) {
        BZLogUtil::logE("VideoRecorder Failed to av_interleaved_write_frame!\n");
        return encode_ret;
    }
    av_packet_unref(audio_st->avPacket);
    return encode_ret;
}

int64_t VideoRecorder::addVideoData(AVFrame *avFrameData, int64_t videoPts) {
    addVideoDataLock.lock();
    isAddVideoData = true;
    yuvBufferTotalCount++;
//    BZLogUtil::logE("VideoRecorder addVideoData for AVFrame--yuvBufferTotalCount=%d", yuvBufferTotalCount);
    int ret = 0;
    if (synEncode) {
        encodeFrame(avFrameData, videoPts);
    } else {
        mutexAVFrameDeque.lock();
        AVFrame *avFrame = av_frame_clone(avFrameData);
        avFrame->pts = videoPts;
        avFrameDeque.push_back(avFrame);
        mutexAVFrameDeque.unlock();
    }
    isAddVideoData = false;
    addVideoDataLock.unlock();
    return recorderTime;
}

int64_t VideoRecorder::addVideoData(unsigned char *data, int64_t videoPts) {
    if (isStopRecorder || nullptr == data)
        return -1;

    addVideoDataLock.lock();
    yuvBufferTotalCount++;
//    BZLogUtil::logV("VideoRecorder addVideoData for data");
    isAddVideoData = true;
    AVFrame *frame = video_st->tmp_frame;
    uint8_t *buffer = data;
    if (this->pixelFormat == PixelFormat::YUVI420) {
        frame->data[0] = buffer;
        frame->data[1] = buffer + y_size;
        frame->data[2] = buffer + y_size * 5 / 4;
    } else if (this->pixelFormat == PixelFormat::YV12) {
        frame->data[0] = buffer;
        frame->data[1] = buffer + y_size * 5 / 4;
        frame->data[2] = buffer + y_size;
    } else {
        BZLogUtil::logE("Unsupported pixelFormat");
        return -1;
    }
    if (synEncode) {
        encodeFrame(frame, videoPts);
    } else {
        mutexAVFrameDeque.lock();
        AVFrame *avFrame = av_frame_clone(frame);
        avFrame->pts = videoPts;
        avFrameDeque.push_back(avFrame);
        mutexAVFrameDeque.unlock();
    }
    isAddVideoData = false;
    addVideoDataLock.unlock();
    return recorderTime;
}

int VideoRecorder::encodeFrame(AVFrame *avFrame, int64_t videoPts) {
    if (nullptr == avFrame) {
        return -1;
    }
    int64_t startTime = 0;
    int ret = 0, got_picture = 0;
    av_init_packet(video_st->avPacket);
    if (videoPts >= 0) {
        if (nullptr == videoPtsBuffer)
            videoPtsBuffer = new list<int64_t>();
        videoPtsBuffer->push_back(videoPts);
    }
    avFrame->pts = videoFrameCount++;
    startTime = getCurrentTime();
    //Encode
    ret = avcodec_encode_video2(video_st->avCodecContext, video_st->avPacket,
                                avFrame,
                                &got_picture);
    if (ret < 0) {
        BZLogUtil::logE("VideoRecorder Failed to encode! \n");
        return ret;
    }
    if (got_picture != 1) {
        BZLogUtil::logD("VideoRecorder avcodec_encode_video2 got_picture!=1");
        return -1;
    }
    int64_t time = getCurrentTime() - startTime;
    encodeTotalTime += time;
    if (logIndex % 30 == 0) {
        BZLogUtil::logV("VideoRecorder avcodec_encode_video2 time cost=%lld", time);
    }
    if (videoPts >= 0 && !videoPtsBuffer->empty()) {
        videoPts = videoPtsBuffer->front();
        videoPtsBuffer->pop_front();
    }
    writeVideoPacket(video_st->avPacket, got_picture, videoPts);
    logIndex++;
    return ret;
}

int VideoRecorder::writeVideoPacket(AVPacket *avPacket, int got_picture, int64_t videoPtsInput) {
    if (nullptr == avPacket || avPacket->size <= 0 || avPacket->data == nullptr) {
        BZLogUtil::logV(
                "VideoRecorder avPacket is NULL or avPacket->size <= 0 || avPacket->data ==nullptr");
        return -1;
    }
    if (!avPacketFromMediaCodec && videoCacheFramePts == 0) {
        videoCacheFramePts =
                video_st->avCodecContext->frame_number * (video_st->avStream->time_base.den /
                                                          videoRate);
    }

    int ret = 0;
    if (got_picture) {
        if (videoPtsInput >= 0) {
            avPacket->pts = videoPtsInput;
            avPacket->dts = videoPtsInput;
        } else {
            //先用自身的pts
            if (avPacketFromMediaCodec) {
                avPacket->pts = video_st->avCodecContext->frame_number *
                                (video_st->avStream->time_base.den / videoRate);
                avPacket->dts = avPacket->pts;

            } else {
                av_packet_rescale_ts(avPacket, video_st->avCodecContext->time_base,
                                     video_st->avStream->time_base);
            }

            int64_t tempPts = audio_st->last_pts;
            if (tempPts < 0)tempPts = 0;
            //还要祛除缓冲区里面的pts
            if (NULL != audio_st && NULL != audio_st->avStream) {
                tempPts = tempPts * video_st->avStream->time_base.den /
                          audio_st->avStream->time_base.den -
                          videoCacheFramePts;
            }

//            BZLogUtil::logD(
//                    "tempPts=%lld--avPacket->pts=%lld--audio_st->last_pts=%lld--vlast_pts=%lld--videoCacheFramePts=%lld",
//                    tempPts,
//                    avPacket->pts,
//                    audio_st->last_pts, video_st->last_pts, videoCacheFramePts);

            //前5帧用自增的方式,为什么是5呢?,现在改成0,如果有的手机反应慢,会导致第6帧停留时间过长
            if (tempPts > avPacket->pts && video_st->avStream->nb_frames > 0) {//追齐音频时间戳
                if (showDebugLog) {
                    BZLogUtil::logV(
                            "---追齐音频时间戳-- temp pts=%lld--avPacket->pts=%lld--audio_st->last_pts=%lld--vlast_pts=%lld--videoCacheFramePts=%lld",
                            tempPts,
                            avPacket->pts,
                            audio_st->last_pts, video_st->last_pts, videoCacheFramePts);
                }
                avPacket->pts = tempPts;
                avPacket->dts = avPacket->pts;
            }
        }

        //保证第一帧有数据,并检测时间戳是否是递增的
        while (avPacket->pts <= video_st->last_pts && video_st->avStream->nb_frames > 0) {
            avPacket->pts += video_st->avStream->time_base.den / videoRate;
            avPacket->dts = avPacket->pts;
        }
        if (nullptr != videoPts && !videoPts->empty()) {
            avPacket->pts = videoPts->front();
            avPacket->dts = videoPts->front();
            videoPts->pop_front();
        }
        if (showDebugLog) {
            int64_t temp = static_cast<int64_t>((avPacket->pts - video_st->last_pts) * 1000.0 *
                                                video_st->avStream->time_base.num /
                                                video_st->avStream->time_base.den);
            BZLogUtil::logD("VideoRecorder pts=%lld--dts=%lld--duration=%lld",
                            avPacket->pts,
                            avPacket->dts,
                            temp);
        }
        video_st->last_pts = avPacket->pts;
        avPacket->stream_index = video_st->avStream->index;
        videoWriteMutex->lock();
        if (nullptr == audio_st || nullptr == audio_st->avCodec) {
            recorderTime = (long) (video_st->avPacket->pts * av_q2d(video_st->avStream->time_base) *
                                   1000);
        }
        ret = av_interleaved_write_frame(avFormatContext, avPacket);
        if (ret < 0) {
            BZLogUtil::logE("av_interleaved_write_frame fail ret=%d", ret);
        }
        videoWriteMutex->unlock();
        av_packet_unref(avPacket);
    } else {
        BZLogUtil::logE("VideoRecorder got_picture--=%d don't write_frame", got_picture);
    }
    return ret;
}

//这个是在gl线程中调用的
void VideoRecorder::setStopRecordFlag() {
    BZLogUtil::logD("setStopRecordFlag encodeThreadIsRunning=" + encodeThreadIsRunning);
//    if (NULL != textureConvertYUVUtil) {
//        //刷新一帧缓存,最开始的那一帧是返回的NULL
//        mutexAVFrameList.lock();
//        AVFrame *result = textureConvertYUVUtil->textureConvertYUV(0);
//        if (NULL != result) {
//            avFrameList.push_back(result);
//        } else {
//            BZLogUtil::logD("VideoRecorder textureConvertYUV NULL == result");
//        }
//        mutexAVFrameList.unlock();
//    }
    isStopRecorder = true;
    std::chrono::milliseconds dura(10);
    while (encodeThreadIsRunning) {
        BZLogUtil::logD("VideoRecorder setStopRecordFlag encodeThreadIsRunning sleep_for 10");
        std::this_thread::sleep_for(dura);
        continue;
    }

    if (NULL != textureConvertYUVUtil) {
        textureConvertYUVUtil->destroyResource();
        delete (textureConvertYUVUtil);
        textureConvertYUVUtil = NULL;
    }
}

int VideoRecorder::updateTexture(int textureId, int64_t videoPts) {
    updateTextureCount++;
//    BZLogUtil::logD("VideoRecorder updateTexture");
    if (!isStopRecorder && NULL != textureConvertYUVUtil) {
        AVFrame *result = textureConvertYUVUtil->textureConvertYUV(textureId);
        if (NULL != result) {
            if (synEncode) {
                encodeFrame(result, videoPts);
                av_frame_free(&result);
            } else {
                mutexAVFrameDeque.lock();
                result->pts = videoPts;
                avFrameDeque.push_back(result);
                mutexAVFrameDeque.unlock();
            }
        } else {
            BZLogUtil::logD("VideoRecorder textureConvertYUV NULL == result");
        }
    } else {
        BZLogUtil::logE("VideoRecorder isStopRecorder =%d textureConvertYUVUtil is NULL=%d",
                        isStopRecorder == true,
                        NULL == textureConvertYUVUtil);
    }
    return 0;
}

void VideoRecorder::encodeThread() {
    BZLogUtil::logD("VideoRecorder encodeThread start");
    std::chrono::milliseconds dura(10);
    bool listIsEmpty;
    while (true) {
        mutexAVFrameDeque.lock();
        listIsEmpty = avFrameDeque.empty();
        mutexAVFrameDeque.unlock();
        if (isStopRecorder && listIsEmpty) {
            break;
        } else if (listIsEmpty) {
//            BZLogUtil::logV("VideoRecorder VideoRecorder listIsEmpty sleep_for 10");
            std::this_thread::sleep_for(dura);
            continue;
        }
        mutexAVFrameDeque.lock();
        AVFrame *avFrame = NULL;
        long avFrameListSize = avFrameDeque.size();
        if (avFrameListSize > 5) {
            do {
                avFrame = avFrameDeque.front();
                avFrameDeque.pop_front();
                av_frame_free(&avFrame);
                avFrameListSize = avFrameDeque.size();
                BZLogUtil::logW("avFrameDeque size=%d pop_front", avFrameListSize);
            } while (avFrameListSize > 5);
        }
        avFrame = avFrameDeque.front();
        avFrameDeque.pop_front();
        mutexAVFrameDeque.unlock();
        if (nullptr != avFrame) {
            encodeFrame(avFrame, avFrame->pts);
            av_frame_free(&avFrame);
        }
    }
    encodeThreadIsRunning = false;
    BZLogUtil::logD("VideoRecorder encodeThread end");
}

int64_t
VideoRecorder::addVideoPacketData(const unsigned char *outputData, int64_t size, int64_t pts) {
    if (isStopRecorder || nullptr == outputData || size <= 0 || nullptr == avFormatContext ||
        nullptr == video_st ||
        nullptr == video_st->avStream || isStopRecorder || nullptr == video_st->avCodec) {
        return -1;
    }
    //这种明显速度过快了,纠正一下
    int64_t timeSpace = pts - addVideoPacketDataLatsPts;
    if (addVideoPacketDataLatsPts > 0 && timeSpace > 0 && timeSpace < 30 && videoRate <= 30) {
        if (showDebugLog) {
            BZLogUtil::logV(
                    "VideoRecorder pts - addVideoPacketDataLatsPts < 30 LatsPts=%lld,pts=%lld,差值=%lld",
                    addVideoPacketDataLatsPts, pts, pts - addVideoPacketDataLatsPts);
        }

        if (video_st->avCodecContext->frame_number > 0) {
            pts = addVideoPacketDataLatsPts + pts / video_st->avCodecContext->frame_number;
            if (showDebugLog) {
                BZLogUtil::logD("VideoRecorder 时间差值=%lld",
                                pts / video_st->avCodecContext->frame_number);
            }
        }
    }
    if (showDebugLog) {
        BZLogUtil::logV("VideoRecorder addVideoPacketData size=%lld pts=%lld", size, pts);
    }
    int nalu_type = (outputData[4] & 0x1F);
    if (nalu_type == H264_NALU_TYPE_SEQUENCE_PARAMETER_SET) {
        headerSize = size;
        if (nullptr != bufferHeader) {
            free(bufferHeader);
            bufferHeader = nullptr;
        }
        bufferHeader = static_cast<unsigned char *>(malloc(
                static_cast<size_t>(size * sizeof(char))));
        memcpy(bufferHeader, outputData, size);
        isWriteHeaderSuccess = true;
        BZLogUtil::logD("VideoRecorder WriteHeaderSuccess");
        return 0;
    }
    if (!isWriteHeaderSuccess) {
        BZLogUtil::logE("VideoRecorder !isWriteHeaderSuccess");
        return -1;
    }
    size_t finalSize = static_cast<size_t>(size + headerSize);
    unsigned char *buffer = static_cast<unsigned char *>(malloc(finalSize));
    AVPacket *avPacket = av_packet_alloc();
    av_init_packet(avPacket);
    avPacket->size = finalSize;
    avPacket->stream_index = video_st->avStream->index;
    avPacket->side_data = nullptr;
    avPacket->pts = 0;
    avPacket->dts = 0;
    avPacket->data = buffer;
    if (nalu_type == H264_NALU_TYPE_IDR_PICTURE || nalu_type == H264_NALU_TYPE_SEI) {
        avPacket->flags = AV_PKT_FLAG_KEY;
    } else {
        avPacket->flags = 0;
    }
    if (nullptr != bufferHeader)
        memcpy(buffer, bufferHeader, static_cast<size_t>(headerSize));

//这个编码是有缓存的,不能直接去同步音频时间戳
//    if (pts < recorderTime) {
//        pts = recorderTime;
//        BZLogUtil::logV("VideoRecorder ---追齐音频时间戳--");
//    }
//    if (pts <= addVideoPacketDataLatsPts && video_st->avCodecContext->frame_number > 0) {
//        pts += pts / video_st->avCodecContext->frame_number;
//    }

    memcpy(buffer + headerSize, outputData, static_cast<size_t>(size));
    int64_t videoPts = static_cast<int64_t>(pts * video_st->avStream->time_base.den /
                                            (1000.0 * video_st->avStream->time_base.num));
    writeVideoPacket(avPacket, 1, videoPts);
    video_st->avCodecContext->frame_number++;

    avPacket->buf = nullptr;
    avPacket->size = 0;

    av_packet_free(&avPacket);
    free(buffer);
    addVideoPacketDataLatsPts = pts;
    return recorderTime;
}



