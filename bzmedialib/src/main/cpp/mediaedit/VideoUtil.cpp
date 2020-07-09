
/**
 * Created by zhandalin on 2017-08-09 20:46.
 * 说明: 视频工具类
 */
#include "VideoUtil.h"
#include "BackgroundMusicUtil.h"

#include <string>
#include <common/bzcommon.h>
#include <common/bz_time.h>

extern "C" {
#include "../common/ffmpeg_cmd.h"
}


AVFrame *VideoUtil::allocVideoFrame(enum AVPixelFormat pix_fmt, int width, int height) {
//    BZLogUtil::logD("alloc_picture");
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
        BZLogUtil::logE("Could not allocate frame data.\n");
    }
    return picture;
}

AVFrame *VideoUtil::allocAudioFrame(enum AVSampleFormat sample_fmt,
                                    uint64_t channel_layout,
                                    int sample_rate, int nb_samples) {
//    BZLogUtil::logD("alloc_audio_frame");
    AVFrame *frame = av_frame_alloc();
    int ret;
    if (!frame) {
        BZLogUtil::logD("Error allocating an audio frame\n");
        return NULL;
    }
    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            BZLogUtil::logD("Error allocating an audio buffer\n");
        }
    }
    return frame;
}


int VideoUtil::getVideoWidth(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("nullptr == videoPath");
        return -1;
    }
    AVFormatContext *in_fmt_ctx = NULL;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    int temp = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            temp = stream->codecpar->width;
            break;
        }
    }
    if (NULL != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return temp;
}

int VideoUtil::getVideoHeight(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("getVideoHeight nullptr == videoPath");
        return -1;
    }
    AVFormatContext *in_fmt_ctx = NULL;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    int temp = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            temp = stream->codecpar->height;
            break;
        }
    }
    if (NULL != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return temp;
}

int VideoUtil::getVideoRotate(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("getVideoRotate nullptr == videoPath");
        return 0;
    }
    AVFormatContext *in_fmt_ctx = NULL;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    int videoRotate = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVDictionaryEntry *entry = av_dict_get(stream->metadata, "rotate", NULL,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (NULL != entry) {
                videoRotate = atoi(entry->value);
            }
            break;
        }
    }
    if (NULL != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return videoRotate;
}

int64_t VideoUtil::getMediaDuration(const char *mediaPath) {
    if (nullptr == mediaPath) {
        BZLogUtil::logE("getMediaDuration nullptr == videoPath");
        return -1;
    }
    AVFormatContext *in_fmt_ctx = NULL;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, mediaPath, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    int64_t videoDuration = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        int64_t temp = stream->duration * 1000 * stream->time_base.num /
                       stream->time_base.den;
        if (temp > videoDuration)
            videoDuration = temp;
    }
    if (NULL != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);

    return videoDuration;
}


int VideoUtil::getVideoInfo(const char *videoPath, long handle, void (*sendMediaInfoCallBack)(
        long, int, int)) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("nullptr == videoPath");
        return -1;
    }
    if (NULL == sendMediaInfoCallBack)
        return -1;
    AVFormatContext *in_fmt_ctx = NULL;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    long videoDuration = 0;
    int height = 0, width = 0, rotate = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        long temp = (long) (stream->duration * 1000 * stream->time_base.num /
                            stream->time_base.den);
        if (temp > videoDuration)
            videoDuration = temp;
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            width = stream->codecpar->width;
            height = stream->codecpar->height;
            AVDictionaryEntry *entry = av_dict_get(stream->metadata, "rotate", NULL,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (NULL != entry) {
                rotate = atoi(entry->value);
            }
        }

    }
    if (NULL != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    sendMediaInfoCallBack(handle, MEDIA_INFO_WHAT_VIDEO_DURATION, (int) videoDuration);
    sendMediaInfoCallBack(handle, MEDIA_INFO_WHAT_VIDEO_ROTATE, rotate);
    sendMediaInfoCallBack(handle, MEDIA_INFO_WHAT_VIDEO_WIDTH, width);
    sendMediaInfoCallBack(handle, MEDIA_INFO_WHAT_VIDEO_HEIGHT, height);

    return 0;
}

int VideoUtil::printVideoTimeStamp(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("nullptr == videoPath");
        return -1;
    }
    AVFormatContext *in_fmt_ctx = NULL;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    int videoIndex = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            BZLogUtil::logD("video index=%d", i);
            videoIndex = i;
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
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            BZLogUtil::logD("--audio-- index=%d", i);
        }
    }

    AVStream *videoInStream = in_fmt_ctx->streams[videoIndex];
    AVFrame *videoFrame = VideoUtil::allocVideoFrame(videoInStream->codec->pix_fmt,
                                                     videoInStream->codecpar->width,
                                                     videoInStream->codecpar->height);

//    FILE *yuvFile = fopen("/sdcard/Filter/out.yuv", "wb");
    int got_picture_ptr = 0;
    size_t ySize = (size_t)(videoInStream->codecpar->width * videoInStream->codecpar->height);
    AVPacket *decode_pkt = av_packet_alloc();
    int videoKeyFrameCount = 0;
    int videoFrameCount = 0;
    while (av_read_frame(in_fmt_ctx, decode_pkt) >= 0) {
        if (decode_pkt->stream_index == videoIndex) {
            videoFrameCount++;
            if (decode_pkt->flags == AV_PKT_FLAG_KEY) {
                videoKeyFrameCount++;
            }
            BZLogUtil::logD(
                    "video stream_index=%d\tpts=%lld\tdts=%lld\tduration=%lld\t durationTime=%lld isKeyFrame=%d",
                    decode_pkt->stream_index,
                    decode_pkt->pts, decode_pkt->dts, decode_pkt->duration,
                    (decode_pkt->duration * 1000 *
                     videoInStream->time_base.num /
                     videoInStream->time_base.den), decode_pkt->flags == AV_PKT_FLAG_KEY);
        } else {
//            BZLogUtil::logD("audio stream_index=%d\tpts=%lld\tdts=%lld\tduration=%lld",
//                            decode_pkt->stream_index,
//                            decode_pkt->pts, decode_pkt->dts, decode_pkt->duration);
        }


//        if (decode_pkt->stream_index == videoIndex) {
//            ret = avcodec_decode_video2(in_stream->codec, videoFrame, &got_picture_ptr,
//                                        decode_pkt);
//            if (ret < 0) {
//                BZLogUtil::logD("avcodec_decode_video2 fail");
//            }
//            BZLogUtil::logD("stream_index=%d\tpts=%lld\tdts=%lld\tduration=%lld", videoIndex,
//                 videoFrame->pts, videoFrame->pkt_dts, videoFrame->pkt_duration);
//
//            fwrite(videoFrame->data[0], ySize, 1, yuvFile);
//
//            fwrite(videoFrame->data[1], ySize / 4, 1, yuvFile);
//            fwrite(videoFrame->data[2], ySize / 4, 1, yuvFile);
//        }
        av_init_packet(decode_pkt);
    }
//    fflush(yuvFile);
//    fclose(yuvFile);

    BZLogUtil::logD("videoKeyFrameCount=%d,videoFrameCount=%d", videoKeyFrameCount,
                    videoFrameCount);
    if (NULL != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return 0;
}

int
VideoUtil::openInputFile(const char *filename, AVFormatContext **in_fmt_ctx) {
    return openInputFileForHard(filename, in_fmt_ctx, true, true);
}

int
VideoUtil::openInputFileForHard(const char *filename, AVFormatContext **in_fmt_ctx,
                                bool openVideoDecode,
                                bool openAudioDecode) {
    if (NULL == filename)
        return -1;
    int ret;
    unsigned int i;

    if ((ret = avformat_open_input(in_fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(*in_fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    for (i = 0; i < (*in_fmt_ctx)->nb_streams; i++) {
        AVStream *stream = (*in_fmt_ctx)->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (openVideoDecode) {
                ret = openAVCodecContext(stream);
                if (ret < 0) {
                    BZLogUtil::logE("openAVCodecContext AVMEDIA_TYPE_AUDIO fail");
                    break;
                }
            } else {
                stream->codec = NULL;
            }
        }
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (openAudioDecode) {
                ret = openAVCodecContext(stream);
                if (ret < 0) {
                    BZLogUtil::logE("openAVCodecContext AVMEDIA_TYPE_AUDIO fail");
                    break;
                }
            } else {
                stream->codec = NULL;
            }
        }
    }
    //如果有一个流没有打开, 那么dump就会有问题
//    av_dump_format((*in_fmt_ctx), 0, filename, 0);
    return ret;
}

int VideoUtil::openInputFileForSoft(const char *filename, AVFormatContext **in_fmt_ctx,
                                    bool openVideoDecode,
                                    bool openAudioDecode) {
    if (NULL == filename)
        return -1;
    int ret;
    unsigned int i;

    if ((ret = avformat_open_input(in_fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info((*in_fmt_ctx), NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    for (i = 0; i < (*in_fmt_ctx)->nb_streams; i++) {
        AVStream *inStream = (*in_fmt_ctx)->streams[i];
        AVCodec *avCodec = avcodec_find_decoder(inStream->codecpar->codec_id);
        if (NULL == avCodec) {
            BZLogUtil::logE("can't find_decoder");
            return -1;
        }
        AVCodecContext *codec_ctx = avcodec_alloc_context3(avCodec);
        if (NULL == codec_ctx) {
            BZLogUtil::logE("can't avcodec_alloc_context3");
            return -1;
        }
        avcodec_parameters_to_context(codec_ctx, inStream->codecpar);
        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && openVideoDecode) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                                NULL, NULL);
            if (ret < 0) {
                BZLogUtil::logE("Failed to open decoder for stream");
                return ret;
            }
        }
        if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && openAudioDecode) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                                NULL, NULL);
            if (ret < 0) {
                BZLogUtil::logE("Failed to open decoder for stream");
                return ret;
            }
        }
        if (nullptr != inStream->codec)
            avcodec_free_context(&inStream->codec);
        inStream->codec = codec_ctx;
    }
//    av_dump_format((*in_fmt_ctx), 0, filename, 0);
    return ret;
}


int VideoUtil::openAVCodecContext(AVStream *inStream) {
    int ret = 0;
    AVCodecID codec_id = inStream->codecpar->codec_id;
    AVCodec *avCodec = NULL;
    if (codec_id == AV_CODEC_ID_H264) {//换成硬解码
        avCodec = avcodec_find_decoder_by_name("h264_mediacodec");
    } else if (codec_id == AV_CODEC_ID_MPEG4) {
        avCodec = avcodec_find_decoder_by_name("mpeg4_mediacodec");
    } else {
        avCodec = avcodec_find_decoder(codec_id);
    }

    if (NULL == avCodec) {
        BZLogUtil::logE("can't find_decoder");
        return -1;
    }
    AVCodecContext *codec_ctx = avcodec_alloc_context3(avCodec);
    if (NULL == codec_ctx) {
        BZLogUtil::logE("can't avcodec_alloc_context3");
        return -1;
    }
    avcodec_parameters_to_context(codec_ctx, inStream->codecpar);

    /* Reencode video & audio and remux subtitles etc. */
    if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
        || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        /* Open decoder */
        ret = avcodec_open2(codec_ctx,
                            NULL, NULL);
        if (ret < 0) {
            BZLogUtil::logE("Failed to open decoder for stream openAVCodecContext");
            return ret;
        }
    }
    if (nullptr != inStream->codec)
        avcodec_free_context(&inStream->codec);
    inStream->codec = codec_ctx;
    return ret;
}

int64_t VideoUtil::getBitRate(int videoWidth, int videoHeight, bool allFrameIsKey) {
    //bit_rate 设置,
    int bitrateP = videoWidth < videoHeight ? videoHeight : videoWidth;
    int64_t bit_rate;
    if (bitrateP >= 1920) {
        bit_rate = 7552;
    } else if (bitrateP >= 1080) {
        bit_rate = 4992;
    } else if (bitrateP >= 720) {
        bit_rate = 2496;
    } else if (bitrateP >= 576) {
        bit_rate = 1856;
    } else if (bitrateP >= 480) {
        bit_rate = 1216;
    } else if (bitrateP >= 432) {
        bit_rate = 1088;
    } else if (bitrateP >= 360) {
        bit_rate = 896;
    } else if (bitrateP >= 240) {
        bit_rate = 576;
    } else {
        bit_rate = 2496;
    }
    bit_rate *= 1000;

    if (allFrameIsKey) {//如果是关键帧,那么码率升高
        bit_rate = bit_rate / 2 * 3;
    }
    BZLogUtil::logD("getBitRate mRecordWidth=%d mRecordHeight=%d bit_rate=%lld", videoWidth,
                    videoHeight, bit_rate);
    return static_cast<int64_t>(bit_rate * 4);
}

int VideoUtil::openOutputFile(AVFormatContext *in_fmt_ctx, AVFormatContext **out_fmt_ctx,
                              const char *output_path, bool needAudio) {
    if (nullptr == output_path) {
        BZLogUtil::logE("nullptr == output_path");
        return -1;
    }
    int ret = 0;
    ret = avformat_alloc_output_context2(out_fmt_ctx, NULL, NULL, output_path);
    if (ret < 0) {
        BZLogUtil::logE("alloc_output_context2 fail");
        return ret;
    }
    if (!out_fmt_ctx) {
        return -1;
    }
    int out_stream_index = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = in_fmt_ctx->streams[i];
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && !needAudio) {
            continue;
        }
        AVStream *out_stream = avformat_new_stream(*out_fmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            BZLogUtil::logD("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return ret;
        }
        ret = av_dict_copy(&out_stream->metadata, in_stream->metadata, AV_DICT_IGNORE_SUFFIX);
        if (ret < 0) {
            BZLogUtil::logE("Failed allocating output stream\n");
            return ret;
        }
        out_stream->time_base = in_stream->time_base;

        avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0) {
            BZLogUtil::logD("Failed to copy context from input to output stream codec context\n");
            return ret;
        }
        ret = avcodec_parameters_from_context(out_stream->codecpar, in_stream->codec);
        if (ret < 0) {
            BZLogUtil::logD("Could not copy the stream parameters\n");
            return -1;
        }
        out_stream->index = out_stream_index;
        out_stream_index++;
        (*out_fmt_ctx)->oformat->codec_tag = 0;
        out_stream->codec->codec_tag = 0;
        if ((*out_fmt_ctx)->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

//    av_dump_format(*out_fmt_ctx, 0, output_path, 1);

    /* open the output file, if needed */
    if (!((*out_fmt_ctx)->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&(*out_fmt_ctx)->pb, output_path, AVIO_FLAG_WRITE);
        if (ret < 0) {
            BZLogUtil::logD("Could not open '%s': %s\n", output_path,
                            av_err2str(ret));
            return ret;
        }
    }
    /* Write the stream header, if any. */
    ret = avformat_write_header(*out_fmt_ctx, NULL);
    if (ret < 0) {
        BZLogUtil::logD("Error occurred when opening output file: %s\n",
                        av_err2str(ret));
        return ret;
    }
    return 0;
}

int VideoUtil::getVideoPts(const char *videoPath, list<int64_t> *ptsList) {
    if (nullptr == videoPath || nullptr == ptsList)return -1;
    AVFormatContext *in_fmt_ctx = nullptr;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    AVPacket *avPacket = av_packet_alloc();
    while (true) {
        av_init_packet(avPacket);
        ret = av_read_frame(in_fmt_ctx, avPacket);
        if (ret < 0) {
            BZLogUtil::logD("AVPacket 读取完成");
            break;
        }
        AVStream *avStream = in_fmt_ctx->streams[avPacket->stream_index];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ptsList->push_back(avPacket->pts);
        }
    }
    av_packet_free(&avPacket);
    avformat_close_input(&in_fmt_ctx);
    return 0;
}

float VideoUtil::getVideoAverageDuration(const char *videoPath) {
    if (nullptr == videoPath)return -1;
    AVFormatContext *in_fmt_ctx = nullptr;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    float averageDuration = 3333;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = in_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            averageDuration = (float) ((avStream->duration * 1.0f / avStream->nb_frames) *
                                       av_q2d(avStream->time_base) * 1000);
            break;
        }
    }
    return averageDuration;
}

bool VideoUtil::videoIsSupport(const char *videoPath, bool isSoftDecode) {
    if (nullptr == videoPath) {
        return false;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    unsigned int i;
    if (avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (nullptr == in_fmt_ctx) {
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return false;
    }
    bool result = false;
    for (i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *inStream = in_fmt_ctx->streams[i];
        if (inStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (isSoftDecode) {
                if (inStream->codecpar->codec_id == AV_CODEC_ID_H264
                    || inStream->codecpar->codec_id == AV_CODEC_ID_MPEG4) {
                    result = true;
                    break;
                }
            } else {
                if (inStream->codecpar->codec_id == AV_CODEC_ID_H264
                    || inStream->codecpar->codec_id == AV_CODEC_ID_MPEG4
                    || inStream->codecpar->codec_id == AV_CODEC_ID_VP9
                    || inStream->codecpar->codec_id == AV_CODEC_ID_HEVC
                    || inStream->codecpar->codec_id == AV_CODEC_ID_VP8) {
                    result = true;
                    break;
                }
            }
        }
    }
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return result;
}


bool VideoUtil::audioIsSupport(const char *audioPath) {
    if (nullptr == audioPath) {
        return false;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    unsigned int i;
    if (avformat_open_input(&in_fmt_ctx, audioPath, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (nullptr == in_fmt_ctx) {
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return false;
    }
    bool result = false;
    for (i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *inStream = in_fmt_ctx->streams[i];
        if (inStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (inStream->codecpar->codec_id == AV_CODEC_ID_AAC
                || inStream->codecpar->codec_id == AV_CODEC_ID_MP3) {
                result = true;
                break;
            }
        }
    }
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return result;
}


int
VideoUtil::clipAudio(const char *audioPath, const char *outPath, int64_t startTime,
                     int64_t endTime) {
    if (nullptr == audioPath || nullptr == outPath || startTime < 0 || startTime > endTime) {
        BZLogUtil::logE("nullptr == audioPath || startTime < 0 || startTime > endTime");
        return -1;
    }
    return clipVideo(audioPath, outPath, startTime, endTime);
}

bool VideoUtil::hasAudio(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("hasAudio nullptr == videoPath");
        return false;
    }
    AVFormatContext *in_fmt_ctx = NULL;
    if (avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return false;
    }
    bool hasAudio = false;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && stream->duration > 0) {
            hasAudio = true;
            break;
        }
    }
    if (NULL != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return hasAudio;
}

bool VideoUtil::hasVideo(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("hasAudio nullptr == videoPath");
        return false;
    }
    AVFormatContext *in_fmt_ctx = NULL;
    if (avformat_open_input(&in_fmt_ctx, videoPath, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return false;
    }
    bool hasVideo = false;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && stream->duration > 0) {
            hasVideo = true;
            break;
        }
    }
    if (NULL != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return hasVideo;
}


int VideoUtil::clipVideo(const char *videoPath, const char *outPath, int64_t startTime,
                         int64_t endTime) {
    if (nullptr == videoPath || nullptr == outPath || startTime < 0 || startTime >= endTime) {
        BZLogUtil::logE("nullptr == videoPath || startTime < 0 || startTime >= endTime");
        return -1;
    }
    //用命令行裁切之后时间戳有问题,自己写
    int ret = 0;
    //对总时长进行限制
    int64_t durationTime = endTime - startTime;
    AVFormatContext *in_fmt_ctx = nullptr;
    ret = VideoUtil::openInputFileForSoft(videoPath, &in_fmt_ctx, false, false);
    if (ret < 0) {
        BZLogUtil::logE("openInputFileForSoft fail videoPath=%s", videoPath);
        return -1;
    }

    AVFormatContext *out_fmt_ctx = nullptr;
    ret = VideoUtil::openOutputFile(in_fmt_ctx, &out_fmt_ctx, outPath);
    if (ret < 0) {
        BZLogUtil::logE("openOutputFile fail outPath=%s", outPath);
        return -1;
    }
    if (nullptr == out_fmt_ctx) {
        return -1;
    }
    out_fmt_ctx->duration = endTime - startTime;

    AVStream *videoStream = nullptr;
    AVStream *audioStream = nullptr;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = in_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            int64_t startPts =
                    static_cast<int64_t>(1.0f * startTime * avStream->time_base.den /
                                         (1000 * avStream->time_base.num));
            av_seek_frame(in_fmt_ctx, avStream->index, startPts, AVSEEK_FLAG_BACKWARD);
            videoStream = avStream;
        } else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = avStream;
        }
    }
    //这样要以音频seek
    if (nullptr == videoStream && nullptr != audioStream) {
        int64_t startPts =
                static_cast<int64_t>(1.0f * startTime * audioStream->time_base.den /
                                     (1000 * audioStream->time_base.num));
        av_seek_frame(in_fmt_ctx, audioStream->index, startPts, AVSEEK_FLAG_BACKWARD);
    }

    bool videoStreamIsEnd = false;
    bool audioStreamIsEnd = false;
    int64_t videoStreamEndPts = 0;
    if (nullptr != videoStream) {
        videoStreamEndPts =
                static_cast<int64_t>(1.0f * endTime * videoStream->time_base.den /
                                     (1000 * videoStream->time_base.num));
    } else {
        videoStreamIsEnd = true;
    }
    int64_t audioStreamEndPts = 0;
    if (nullptr != audioStream) {
        audioStreamEndPts = static_cast<int64_t>(1.0f * endTime * audioStream->time_base.den /
                                                 (1000 * audioStream->time_base.num));
    } else {
        audioStreamIsEnd = true;
    }


    AVPacket *avPacket = av_packet_alloc();
    const int64_t START_FLAG = -11111;
    int64_t videoStartPts = START_FLAG;
    int64_t videoStartDts = START_FLAG;
    int64_t audioStartPts = START_FLAG;
    int64_t audioStartDts = START_FLAG;
    while (true) {
        av_init_packet(avPacket);
        ret = av_read_frame(in_fmt_ctx, avPacket);
        if (ret < 0) {
            ret = 0;
            break;
        }
        if (videoStreamIsEnd && audioStreamIsEnd) {
            break;
        }
        if (avPacket->pts == AV_NOPTS_VALUE) {
            continue;
        }
        if (in_fmt_ctx->streams[avPacket->stream_index]->codecpar->codec_type ==
            AVMEDIA_TYPE_VIDEO) {
            if (videoStartPts == START_FLAG) {
                videoStartPts = avPacket->pts;
                videoStartDts = avPacket->dts;
            }
            if (avPacket->pts > videoStreamEndPts) {
                videoStreamIsEnd = true;
                videoStream->duration = avPacket->pts - videoStartPts;
                av_packet_unref(avPacket);
                continue;
            }
            avPacket->pts -= videoStartPts;
            avPacket->dts -= videoStartDts;

            //计算持续时间是否到了,纠正关键帧太少导致的精度不够
            auto temp = 1.0f * avPacket->pts * 1000 * videoStream->time_base.num /
                        videoStream->time_base.den;
            if (temp > durationTime) {
                videoStreamIsEnd = true;
                av_packet_unref(avPacket);
                continue;
            }
        } else if (in_fmt_ctx->streams[avPacket->stream_index]->codecpar->codec_type ==
                   AVMEDIA_TYPE_AUDIO) {
            if (audioStartPts == START_FLAG) {
                audioStartPts = avPacket->pts;
                audioStartDts = avPacket->dts;
            }
            if (avPacket->pts > audioStreamEndPts) {
                audioStreamIsEnd = true;
                audioStream->duration = avPacket->pts - audioStartPts;
                av_packet_unref(avPacket);
                continue;
            }
            avPacket->pts -= audioStartPts;
            avPacket->dts -= audioStartDts;
            //计算持续时间是否到了,纠正关键帧太少导致的精度不够
            auto temp = 1.0f * avPacket->pts * 1000 * audioStream->time_base.num /
                        audioStream->time_base.den;
            if (temp > durationTime) {
                audioStreamIsEnd = true;
                av_packet_unref(avPacket);
                continue;
            }
        }
        if (nullptr != avPacket->data && avPacket->size > 0) {
            av_interleaved_write_frame(out_fmt_ctx, avPacket);
        }
    }
    avformat_close_input(&in_fmt_ctx);
    av_write_trailer(out_fmt_ctx);
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&out_fmt_ctx->pb);
    avformat_free_context(out_fmt_ctx);
    av_packet_free(&avPacket);
    return ret;
}

int VideoUtil::printVideoFrameInfo(const char *videoPath) {
    if (nullptr == videoPath) {
        return -1;
    }
    AVFormatContext *avFormatContext = nullptr;
    int ret = openInputFileForSoft(videoPath, &avFormatContext, false, false);
    if (ret < 0) {
        BZLogUtil::logE("openInputFileForSoft fail path=%s", videoPath);
        return -1;
    }
    AVPacket *avPacket = av_packet_alloc();
    int64_t videoKeyFrameCount = 0;
    int64_t videoFrameCount = 0;
    while (true) {
        av_init_packet(avPacket);
        ret = av_read_frame(avFormatContext, avPacket);
        if (ret < 0) {
            break;
        }
        AVStream *avStream = avFormatContext->streams[avPacket->stream_index];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoFrameCount++;
            if (avPacket->flags == AV_PKT_FLAG_KEY) {
                videoKeyFrameCount++;
            }
        }
        av_packet_unref(avPacket);
    }
    BZLogUtil::logD("videoFrameInfo videoKeyFrameCount=%lld videoFrameCount=%lld ratio=%f",
                    videoKeyFrameCount, videoFrameCount,
                    videoFrameCount * 1.0f / videoKeyFrameCount);
    return 0;
}

int
VideoUtil::mixAudios2Video(const char *outPath, const char *videoStreamInputPath, char **audioPaths,
                           size_t audioCount, OnActionListener *onActionListener) {
    if (nullptr == outPath || nullptr == videoStreamInputPath || nullptr == audioPaths ||
        audioCount <= 0) {
        BZLogUtil::logE(
                "nullptr == outPath || nullptr == videoStreamInputPath || nullptr == audioPaths ||audioCount <= 0");
        return -1;
    }
    BZLogUtil::logD("start mixAudios2Video");
    int ret = 0;
    list<const char *> availableAudioList;
    for (int i = 0; i < audioCount; ++i) {
        char *audioPath = audioPaths[i];
        bool hasAudio = VideoUtil::hasAudio(audioPath);
        if (hasAudio) {
            availableAudioList.push_back(audioPath);
        }
    }
    if (availableAudioList.empty()) {
        BZLogUtil::logE("availableAudioList.empty() 不需要处理音频");
        return -1;
    }

    BackgroundMusicUtil backgroundMusicUtil;
    if (availableAudioList.size() == 1) {
        ret = backgroundMusicUtil.replaceBackgroundMusicOnly(videoStreamInputPath,
                                                             availableAudioList.front(), outPath,
                                                             onActionListener);
    } else {
        //先把几个音频合并成一个音频
        //处理文件名字
        string outputParentDir;
        outputParentDir.append(outPath);
        outputParentDir = outputParentDir.substr(0, outputParentDir.find_last_of("/"));

        char mixOutPath[1024] = {0};
        sprintf(mixOutPath, "%s/mix_%lld.m4a", outputParentDir.c_str(), getMicrosecondTime());
        ret = backgroundMusicUtil.mixMusic(&availableAudioList, mixOutPath, onActionListener);
        if (ret >= 0) {
            ret = backgroundMusicUtil.replaceBackgroundMusicOnly(videoStreamInputPath,
                                                                 mixOutPath, outPath,
                                                                 nullptr);
        }
        //删除临时文件
        remove(mixOutPath);
    }
    BZLogUtil::logD("mixAudios2Video end");
    return ret;
}

