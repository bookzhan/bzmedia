
/**
 * Created by bookzhan on 2017-08-09 20:46.
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
    AVFrame *picture = nullptr;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return nullptr;
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
        return nullptr;
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
    AVFormatContext *in_fmt_ctx = nullptr;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
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
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return temp;
}

int VideoUtil::getVideoHeight(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("getVideoHeight nullptr == videoPath");
        return -1;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
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
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return temp;
}

int VideoUtil::getVideoRotate(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("getVideoRotate nullptr == videoPath");
        return 0;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    int videoRotate = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVDictionaryEntry *entry = av_dict_get(stream->metadata, "rotate", nullptr,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (nullptr != entry) {
                videoRotate = atoi(entry->value);
            }
            break;
        }
    }
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return videoRotate;
}

int64_t VideoUtil::getMediaDuration(const char *mediaPath) {
    if (nullptr == mediaPath) {
        BZLogUtil::logE("getMediaDuration nullptr == mediaPath");
        return -1;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, mediaPath, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file mediaPath=%s", mediaPath);
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    int64_t videoDuration = in_fmt_ctx->duration / 1000;
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);

    return videoDuration;
}


int VideoUtil::getVideoInfo(const char *videoPath, long handle, void (*sendMediaInfoCallBack)(
        long, int, int)) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("nullptr == videoPath");
        return -1;
    }
    if (nullptr == sendMediaInfoCallBack)
        return -1;
    AVFormatContext *in_fmt_ctx = nullptr;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
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
            AVDictionaryEntry *entry = av_dict_get(stream->metadata, "rotate", nullptr,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (nullptr != entry) {
                rotate = atoi(entry->value);
            }
        }

    }
    if (nullptr != in_fmt_ctx)
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
    BZLogUtil::logD("videoPath=%s", videoPath);
    AVFormatContext *in_fmt_ctx = nullptr;
    int ret = 0;
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in_fmt_ctx, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    int videoIndex = 0;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            BZLogUtil::logD("video index=%d", i);
            videoIndex = i;
        }
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            BZLogUtil::logD("--audio-- index=%d", i);
        }
    }
    AVStream *videoInStream = in_fmt_ctx->streams[videoIndex];
    AVPacket *decode_pkt = av_packet_alloc();
    int videoKeyFrameCount = 0;
    int videoFrameCount = 0;
    while (av_read_frame(in_fmt_ctx, decode_pkt) >= 0) {
        if (decode_pkt->stream_index == videoIndex) {
            videoFrameCount++;
            if (decode_pkt->flags == AV_PKT_FLAG_KEY) {
                videoKeyFrameCount++;
            }
            AVStream *stream = in_fmt_ctx->streams[decode_pkt->stream_index];
            int64_t temp = decode_pkt->pts * 1000 * stream->time_base.num /
                           stream->time_base.den;
            if (temp < 16000) {
                BZLogUtil::logD(
                        "videoFrameCount=%d\tflags=%d\tvideo stream_index=%d\tpts=%lld\tdts=%lld\tduration=%lld\tdurationTime=%lld isKeyFrame=%d time=%lld",
                        videoFrameCount,
                        decode_pkt->flags,
                        decode_pkt->stream_index,
                        decode_pkt->pts, decode_pkt->dts, decode_pkt->duration,
                        (decode_pkt->duration * 1000 *
                         videoInStream->time_base.num /
                         videoInStream->time_base.den), decode_pkt->flags == AV_PKT_FLAG_KEY, temp);
            }
        } else {
//            BZLogUtil::logD("audio stream_index=%d\tpts=%lld\tdts=%lld\tduration=%lld",
//                            decode_pkt->stream_index,
//                            decode_pkt->pts, decode_pkt->dts, decode_pkt->duration);
        }
        av_init_packet(decode_pkt);
    }
    BZLogUtil::logD("videoKeyFrameCount=%d,videoFrameCount=%d", videoKeyFrameCount,
                    videoFrameCount);
    if (nullptr != in_fmt_ctx)
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
    if (nullptr == filename)
        return -1;
    int ret;
    unsigned int i;

    if ((ret = avformat_open_input(in_fmt_ctx, filename, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info((*in_fmt_ctx), nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    for (i = 0; i < (*in_fmt_ctx)->nb_streams; i++) {
        AVStream *inStream = (*in_fmt_ctx)->streams[i];
        if (nullptr == inStream) {
            continue;
        }
        AVCodecID codec_id = inStream->codecpar->codec_id;
        if (codec_id == AV_CODEC_ID_NONE) {
            BZLogUtil::logW("openInputFileForHard codec_id == AV_CODEC_ID_NONE");
            continue;
        }
        AVCodec *avCodec = nullptr;
        if (codec_id == AV_CODEC_ID_H264) {//换成硬解码
            avCodec = avcodec_find_decoder_by_name("h264_mediacodec");
        } else if (codec_id == AV_CODEC_ID_MPEG4) {
            avCodec = avcodec_find_decoder_by_name("mpeg4_mediacodec");
        } else if (codec_id == AV_CODEC_ID_HEVC) {
            avCodec = avcodec_find_decoder_by_name("hevc_mediacodec");
        } else {
            avCodec = avcodec_find_decoder(codec_id);
        }
        if (nullptr == avCodec) {
            BZLogUtil::logE("can't find_decoder");
            return -1;
        }
        AVCodecContext *codec_ctx = avcodec_alloc_context3(avCodec);
        if (nullptr == codec_ctx) {
            BZLogUtil::logE("can't avcodec_alloc_context3");
            return -1;
        }
        avcodec_parameters_to_context(codec_ctx, inStream->codecpar);
        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && openVideoDecode) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                                nullptr, nullptr);
            if (ret < 0) {
                BZLogUtil::logE("Failed to open decoder for stream");
                return ret;
            }
        } else if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && openAudioDecode) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                                nullptr, nullptr);
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

int VideoUtil::openInputFileForSoft(const char *filename, AVFormatContext **in_fmt_ctx,
                                    bool openVideoDecode,
                                    bool openAudioDecode) {
    if (nullptr == filename)
        return -1;
    int ret;
    unsigned int i;

    if ((ret = avformat_open_input(in_fmt_ctx, filename, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info((*in_fmt_ctx), nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    for (i = 0; i < (*in_fmt_ctx)->nb_streams; i++) {
        AVStream *inStream = (*in_fmt_ctx)->streams[i];
        if (nullptr == inStream) {
            continue;
        }
        if (inStream->codecpar->codec_id == AV_CODEC_ID_NONE) {
            BZLogUtil::logW("openInputFileForSoft codec_id == AV_CODEC_ID_NONE");
            continue;
        }
        if (inStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && !openVideoDecode) {
            continue;
        } else if (inStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && !openAudioDecode) {
            continue;
        }
        AVCodec *avCodec = avcodec_find_decoder(inStream->codecpar->codec_id);
        if (nullptr == avCodec) {
            BZLogUtil::logE("can't find_decoder");
            return -1;
        }
        AVCodecContext *codec_ctx = avcodec_alloc_context3(avCodec);
        if (nullptr == codec_ctx) {
            BZLogUtil::logE("can't avcodec_alloc_context3");
            return -1;
        }
        avcodec_parameters_to_context(codec_ctx, inStream->codecpar);
        /* Reencode video & audio and remux subtitles etc. */
        if (inStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && openVideoDecode) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                                nullptr, nullptr);
            if (ret < 0) {
                BZLogUtil::logE("Failed to open decoder for stream");
                return ret;
            }
        } else if (inStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && openAudioDecode) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                                nullptr, nullptr);
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
    AVCodec *avCodec = nullptr;
    if (codec_id == AV_CODEC_ID_H264) {//换成硬解码
        avCodec = avcodec_find_decoder_by_name("h264_mediacodec");
    } else if (codec_id == AV_CODEC_ID_MPEG4) {
        avCodec = avcodec_find_decoder_by_name("mpeg4_mediacodec");
    } else {
        avCodec = avcodec_find_decoder(codec_id);
    }

    if (nullptr == avCodec) {
        BZLogUtil::logE("can't find_decoder");
        return -1;
    }
    AVCodecContext *codec_ctx = avcodec_alloc_context3(avCodec);
    if (nullptr == codec_ctx) {
        BZLogUtil::logE("can't avcodec_alloc_context3");
        return -1;
    }
    avcodec_parameters_to_context(codec_ctx, inStream->codecpar);

    /* Reencode video & audio and remux subtitles etc. */
    if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
        || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        /* Open decoder */
        ret = avcodec_open2(codec_ctx,
                            avCodec, nullptr);
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
                              const char *output_path, bool needAudio, bool needOpenCodec) {
    if (nullptr == output_path) {
        BZLogUtil::logE("nullptr == output_path");
        return -1;
    }
    int ret = 0;
    ret = avformat_alloc_output_context2(out_fmt_ctx, nullptr, nullptr, output_path);
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
        if (nullptr == in_stream) {
            continue;
        }
        if (in_stream->codecpar->codec_id == AV_CODEC_ID_NONE) {
            BZLogUtil::logW("openOutputFile codec_id == AV_CODEC_ID_NONE");
            continue;
        }
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
        if (needOpenCodec) {
            AVDictionary *param = nullptr;
            if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                av_dict_set(&param, "preset", "ultrafast", 0);
            }
            ret = avcodec_open2(out_stream->codec, in_stream->codec->codec, &param);
            if (nullptr != param) {
                av_dict_free(&param);
            }
            if (ret < 0) {
                BZLogUtil::logD("avCodecContext Could not open video codec: %s\n", av_err2str(ret));
                return -1;
            }
        }
        ret = avcodec_parameters_from_context(out_stream->codecpar, in_stream->codec);
        if (ret < 0) {
            BZLogUtil::logD("Could not copy the stream parameters\n");
            return -1;
        }
        out_stream->index = out_stream_index;
        out_stream_index++;
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
    ret = avformat_write_header(*out_fmt_ctx, nullptr);
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
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
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
    if ((ret = avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr)) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
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

bool VideoUtil::videoIsSupport(const char *videoPath) {
    if (nullptr == videoPath) {
        return false;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    unsigned int i;
    if (avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (nullptr == in_fmt_ctx) {
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return false;
    }
    bool result = false;
    for (i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *inStream = in_fmt_ctx->streams[i];
        if (inStream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO
            && inStream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        AVCodec *avCodec = avcodec_find_decoder(inStream->codecpar->codec_id);
        if (nullptr == avCodec) {
            BZLogUtil::logE("can't find_decoder");
            result = false;
            break;
        }
        result = true;
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
    if (avformat_open_input(&in_fmt_ctx, audioPath, nullptr, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (nullptr == in_fmt_ctx) {
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
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
    if (endTime <= 0) {
        endTime = getMediaDuration(audioPath);
        BZLogUtil::logD("endTime <= 0  getMediaDuration=%lld", endTime);
    }
    BZLogUtil::logD("clipAudio audioPath=%s,outPath=%s", audioPath, outPath);
    BZLogUtil::logD("clipAudio startTime=%lld,endTime=%lld", startTime, endTime);
    const char *fmt = "ffmpeg -y -ss %.3f -t %.3f -i \"%s\" -vn -c copy \"%s\"";
    char cmd[1024] = {0};
    sprintf(cmd, fmt, startTime / 1000.0f, (endTime - startTime) / 1000.0f, audioPath, outPath);
    BZLogUtil::logD("cmd=%s", cmd);
    int ret = executeFFmpegCommand(0, cmd, nullptr);
    if (ret < 0) {
        BZLogUtil::logE("executeFFmpegCommand fail %s", cmd);
        return ret;
    }
    return ret;
}

bool VideoUtil::hasAudio(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("hasAudio nullptr == videoPath");
        return false;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    if (avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
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
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return hasAudio;
}

bool VideoUtil::isAACAudio(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("hasAudio nullptr == videoPath");
        return false;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    if (avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return false;
    }
    bool isAACAudio = false;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && stream->duration > 0 &&
            stream->codecpar->codec_id == AV_CODEC_ID_AAC) {
            isAACAudio = true;
            break;
        }
    }
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return isAACAudio;
}

bool VideoUtil::hasVideo(const char *videoPath) {
    if (nullptr == videoPath) {
        BZLogUtil::logE("hasAudio nullptr == videoPath");
        return false;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    if (avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
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
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return hasVideo;
}


int VideoUtil::clipVideo(const char *videoPath, const char *outPath, int64_t startTime,
                         int64_t endTime) {
    if (nullptr == videoPath || nullptr == outPath || startTime < 0 || startTime >= endTime) {
        BZLogUtil::logE("nullptr == videoPath || startTime < 0 || startTime >= endTime");
        return -1;
    }
    BZLogUtil::logD("clipVideo videoPath=%s,outPath=%s", videoPath, outPath);
    BZLogUtil::logD("clipVideo startTime=%lld,endTime=%lld", startTime, endTime);
    const char *fmt = "ffmpeg -y -ss %.3f -t %.3f -accurate_seek -i \"%s\" -avoid_negative_ts 1 -c copy \"%s\"";
    char cmd[1024] = {0};
    sprintf(cmd, fmt, startTime / 1000.0f, (endTime - startTime) / 1000.0f, videoPath, outPath);
    BZLogUtil::logD("cmd=%s", cmd);
    int ret = executeFFmpegCommand(0, cmd, nullptr);
    if (ret < 0) {
        BZLogUtil::logE("executeFFmpegCommand fail %s", cmd);
        return ret;
    }
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

bool VideoUtil::isStandardAudio(const char *path) {
    if (nullptr == path) {
        BZLogUtil::logE("hasAudio nullptr == path");
        return false;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    if (avformat_open_input(&in_fmt_ctx, path, nullptr, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return false;
    }
    bool isStandardAudio = false;
    for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *stream;
        stream = in_fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && stream->duration > 0
            && stream->codecpar->sample_rate == 44100 && stream->codecpar->channels == 2 &&
            stream->codecpar->codec_id == AV_CODEC_ID_AAC) {
            isStandardAudio = true;
            break;
        }
    }
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return isStandardAudio;
}

int VideoUtil::repeatVideoToDuration(const char *inputPath, const char *outPath, int64_t duration) {
    if (nullptr == inputPath || nullptr == outPath || duration <= 0) {
        BZLogUtil::logE(
                "repeatVideoToDuration nullptr == input_path_ || nullptr == outputPath_ || duration <= 0");
        return -1;
    }
    BZLogUtil::logD("repeatVideoToDuration inputPath=%s outPath=%s duration=%lld", inputPath,
                    outPath, duration);
    int ret = 0;
    AVFormatContext *inAVFormatContext = nullptr;
    ret = VideoUtil::openInputFileForSoft(inputPath, &inAVFormatContext, false, false);
    if (ret < 0) {
        BZLogUtil::logE("repeatVideoToDuration openInputFileForSoft fail path=%s", inputPath);
        return -1;
    }
    if (nullptr == inAVFormatContext) {
        BZLogUtil::logE("repeatVideoToDuration nullptr==inAVFormatContext");
        return -1;
    }
    AVStream *inVideoStream = nullptr;
    for (int i = 0; i < inAVFormatContext->nb_streams; ++i) {
        AVStream *avStream = inAVFormatContext->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            inVideoStream = avStream;
            break;
        }
    }
    if (nullptr == inVideoStream) {
        avformat_close_input(&inAVFormatContext);
        inAVFormatContext = nullptr;
        BZLogUtil::logE("repeatVideoToDuration nullptr==inVideoStream");
        return -1;
    }
    AVFormatContext *outAVFormatContext = nullptr;
    ret = VideoUtil::openOutputFile(inAVFormatContext, &outAVFormatContext, outPath, false, false);
    if (ret < 0) {
        BZLogUtil::logE("repeatVideoToDuration openOutputFile fail path=%s", outPath);
        return -1;
    }
    if (nullptr == outAVFormatContext) {
        BZLogUtil::logE("repeatVideoToDuration nullptr == outAVFormatContext");
        return -1;
    }
    AVPacket pkt;
    int videoCount = 0;
    long currentVideoTime = 0;
    float VIDEO_RATE = 30;
    while (true) {
        ret = av_read_frame(inAVFormatContext, &pkt);
        if (ret < 0) {
            ret = av_seek_frame(inAVFormatContext, 0, 0, AVSEEK_FLAG_BACKWARD);
            if (ret < 0) {
                break;
            }
            continue;
        }
        AVStream *in_stream = inAVFormatContext->streams[pkt.stream_index];
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        pkt.pts =
                currentVideoTime * in_stream->time_base.den / (1000.0f * in_stream->time_base.num);
        pkt.dts = pkt.pts;
        pkt.duration = (int64_t) ((1000.0f / VIDEO_RATE) * in_stream->time_base.den /
                                  (1000.0f * in_stream->time_base.num));
        pkt.pos = -1;
        pkt.stream_index = 0;
        currentVideoTime = (int64_t) (videoCount * (1000.0f / VIDEO_RATE));
        videoCount++;
        ret = av_interleaved_write_frame(outAVFormatContext, &pkt);
        if (ret < 0) {
            BZLogUtil::logE("Error muxing packet\n");
            av_packet_unref(&pkt);
            continue;
        }
        av_packet_unref(&pkt);
        if (currentVideoTime > duration) {
            break;
        }
    }
    avformat_close_input(&inAVFormatContext);
    ret = av_write_trailer(outAVFormatContext);
    if (ret < 0) {
        BZLogUtil::logE("av_write_trailer fail");
    }
    return 0;
}

int VideoUtil::setVideoDuration(const char *inputPath, const char *outPath, int64_t duration) {
    if (nullptr == inputPath || nullptr == outPath || duration <= 0) {
        BZLogUtil::logE(
                "setVideoDuration nullptr == input_path_ || nullptr == outputPath_ || duration <= 0");
        return -1;
    }
    BZLogUtil::logD("setVideoDuration inputPath=%s outPath=%s duration=%lld", inputPath,
                    outPath, duration);
    int ret = 0;
    AVFormatContext *inAVFormatContext = nullptr;
    ret = VideoUtil::openInputFileForSoft(inputPath, &inAVFormatContext, false, false);
    if (ret < 0) {
        BZLogUtil::logE("setVideoDuration openInputFileForSoft fail path=%s", inputPath);
        return -1;
    }
    if (nullptr == inAVFormatContext) {
        BZLogUtil::logE("setVideoDuration nullptr==inAVFormatContext");
        return -1;
    }
    AVStream *inVideoStream = nullptr;
    for (int i = 0; i < inAVFormatContext->nb_streams; ++i) {
        AVStream *avStream = inAVFormatContext->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            inVideoStream = avStream;
            break;
        }
    }
    if (nullptr == inVideoStream || inVideoStream->nb_frames <= 0) {
        avformat_close_input(&inAVFormatContext);
        inAVFormatContext = nullptr;
        BZLogUtil::logE(
                "setVideoDuration nullptr == inVideoStream || inVideoStream->nb_frames <= 0");
        return -1;
    }
    //每一帧应该有多长
    int64_t frameDuration = duration / inVideoStream->nb_frames;
    //每一帧的PTS
    int64_t frameDurationPts = (int64_t) (frameDuration * inVideoStream->time_base.den /
                                          (1000.0f * inVideoStream->time_base.num));

    AVFormatContext *outAVFormatContext = nullptr;
    ret = VideoUtil::openOutputFile(inAVFormatContext, &outAVFormatContext, outPath, false, false);
    if (ret < 0) {
        BZLogUtil::logE("setVideoDuration openOutputFile fail path=%s", outPath);
        return -1;
    }
    if (nullptr == outAVFormatContext) {
        BZLogUtil::logE("setVideoDuration nullptr == outAVFormatContext");
        return -1;
    }
    outAVFormatContext->duration = duration * 1000;
    AVPacket pkt;
    int videoCount = 0;
    float VIDEO_RATE = 30;
    int64_t pts = 0;
    while (true) {
        ret = av_read_frame(inAVFormatContext, &pkt);
        if (ret < 0) {
            break;
        }
        AVStream *in_stream = inAVFormatContext->streams[pkt.stream_index];
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        pkt.pts = pts;
        pkt.dts = pkt.pts;
        pkt.duration = frameDurationPts;
        pkt.pos = -1;
        pkt.stream_index = 0;
        pts += frameDurationPts;
        videoCount++;
        ret = av_interleaved_write_frame(outAVFormatContext, &pkt);
        if (ret < 0) {
            BZLogUtil::logE("Error muxing packet\n");
            av_packet_unref(&pkt);
            continue;
        }
        av_packet_unref(&pkt);
    }
    avformat_close_input(&inAVFormatContext);
    ret = av_write_trailer(outAVFormatContext);
    if (ret < 0) {
        BZLogUtil::logE("av_write_trailer fail");
    }
    return ret;
}

bool VideoUtil::videoCodecIsSupportForExoPlayer(const char *videoPath) {
    if (nullptr == videoPath) {
        return false;
    }
    AVFormatContext *in_fmt_ctx = nullptr;
    unsigned int i;
    if (avformat_open_input(&in_fmt_ctx, videoPath, nullptr, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot open input file\n");
        return false;
    }
    if (nullptr == in_fmt_ctx) {
        return false;
    }
    if (avformat_find_stream_info(in_fmt_ctx, nullptr) < 0) {
        av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
        return false;
    }
    bool result = false;
    for (i = 0; i < in_fmt_ctx->nb_streams; i++) {
        AVStream *inStream = in_fmt_ctx->streams[i];
        if (inStream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO
            && inStream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }
        if (inStream->codecpar->codec_id == AV_CODEC_ID_AAC
            || inStream->codecpar->codec_id == AV_CODEC_ID_HEVC
            || inStream->codecpar->codec_id == AV_CODEC_ID_FLV1
            || inStream->codecpar->codec_id == AV_CODEC_ID_MP3
            || inStream->codecpar->codec_id == AV_CODEC_ID_AMR_NB
            || inStream->codecpar->codec_id == AV_CODEC_ID_AMR_WB
            || inStream->codecpar->codec_id == AV_CODEC_ID_FLAC
            || inStream->codecpar->codec_id == AV_CODEC_ID_H263
            || inStream->codecpar->codec_id == AV_CODEC_ID_MPEG4
            || inStream->codecpar->codec_id == AV_CODEC_ID_VP8
            || inStream->codecpar->codec_id == AV_CODEC_ID_VP9
            || inStream->codecpar->codec_id == AV_CODEC_ID_PCM_S16LE
            || inStream->codecpar->codec_id == AV_CODEC_ID_PCM_S8
            || inStream->codecpar->codec_id == AV_CODEC_ID_H264) {
            result = true;
        } else {
            result = false;
            break;
        }
    }
    if (nullptr != in_fmt_ctx)
        avformat_close_input(&in_fmt_ctx);
    return result;
}

