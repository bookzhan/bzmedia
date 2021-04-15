//
/**
 * Created by bookzhan on 2019-01-14 10:10.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include "MergeVideoAndAudio.h"

int MergeVideoAndAudio::startMergeVideoAndAudio(const char *videoPath, const char *audioPath,
                                                const char *outputPath) {
    if (nullptr == videoPath || nullptr == audioPath || nullptr == outputPath) {
        BZLogUtil::logE("nullptr == videoPath || nullptr == audioPath || nullptr == outputPath");
        return -1;
    }
    int ret = 0;
    //判断是否有视频,音频流
    AVFormatContext *video_fmt_ctx = nullptr;
    ret = VideoUtil::openInputFileForSoft(videoPath, &video_fmt_ctx, false, false);
    if (ret < 0) {
        BZLogUtil::logE("openInputFileForSoft %s", videoPath);
        return -1;
    }
    AVFormatContext *audio_fmt_ctx = nullptr;
    ret = VideoUtil::openInputFileForSoft(audioPath, &audio_fmt_ctx, false, false);
    if (ret < 0) {
        BZLogUtil::logE("openInputFileForSoft %s", audioPath);
        return -1;
    }
    AVStream *videoInStream = nullptr;
    AVStream *audioInStream = nullptr;

    for (int i = 0; i < video_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = video_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoInStream = avStream;
            break;
        }
    }
    for (int i = 0; i < audio_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = audio_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioInStream = avStream;
            break;
        }
    }
    if (nullptr == audioInStream || nullptr == videoInStream) {
        BZLogUtil::logE("nullptr==audioInStream|| nullptr==videoInStream");
        avformat_close_input(&audio_fmt_ctx);
        avformat_close_input(&video_fmt_ctx);
        return -1;
    }
    AVFormatContext *out_fmt_ctx = nullptr;
    ret = openOutputFile(&out_fmt_ctx, videoInStream, audioInStream, outputPath);
    if (ret < 0) {
        avformat_close_input(&audio_fmt_ctx);
        avformat_close_input(&video_fmt_ctx);
        return -1;
    }
    AVPacket *videoAVPacket = av_packet_alloc();
    AVPacket *audioAVPacket = av_packet_alloc();

    int64_t videoMaxPts = (audioInStream->duration * 1000 * audioInStream->time_base.num /
                           audioInStream->time_base.den) * videoInStream->time_base.den /
                          (videoInStream->time_base.num * 1000);

    int64_t videoTime = 0, audioTime = 0;
    bool videoEnd = false;
    bool audioEnd = false;
    while (true) {
        av_init_packet(audioAVPacket);
        ret = av_read_frame(audio_fmt_ctx, audioAVPacket);
        if (ret < 0) {
            audioEnd = true;
        }
        if (videoEnd && audioEnd) {
            break;
        }
        if (audioAVPacket->stream_index != audioInStream->index && !audioEnd) {
            av_packet_unref(audioAVPacket);
            continue;
        }
        if (audioAVPacket->stream_index == audioInStream->index && !audioEnd) {
            audioAVPacket->stream_index = 1;
            audioTime = audioAVPacket->pts * 1000 * audioInStream->time_base.num /
                        audioInStream->time_base.den;
            av_interleaved_write_frame(out_fmt_ctx, audioAVPacket);
        }
        av_packet_unref(audioAVPacket);

        while (videoTime < audioTime || audioEnd) {
            av_init_packet(videoAVPacket);
            ret = av_read_frame(video_fmt_ctx, videoAVPacket);
            if (ret < 0) {
                videoEnd = true;
                break;
            }
            if (videoAVPacket->stream_index != videoInStream->index) {
                av_packet_unref(videoAVPacket);
                continue;
            }
            videoAVPacket->stream_index = 0;
            videoAVPacket->pts = static_cast<int64_t>(videoAVPacket->pts * 1.0f /
                                                      videoInStream->duration * videoMaxPts);
            videoAVPacket->dts = videoAVPacket->pts;

            videoTime = videoAVPacket->pts * 1000 * videoInStream->time_base.num /
                        videoInStream->time_base.den;
            av_interleaved_write_frame(out_fmt_ctx, videoAVPacket);

            av_packet_unref(videoAVPacket);
        }
    }
    ret = av_write_trailer(out_fmt_ctx);
    if (ret != 0) {
        BZLogUtil::logE("av_write_trailer fail");
    }
    if (out_fmt_ctx && !(out_fmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&out_fmt_ctx->pb);

    for (int i = 0; i < out_fmt_ctx->nb_streams; ++i) {
        if (nullptr != out_fmt_ctx->streams[i]->codec)
            avcodec_free_context(&out_fmt_ctx->streams[i]->codec);
    }
    avformat_free_context(out_fmt_ctx);

    for (int i = 0; i < audio_fmt_ctx->nb_streams; ++i) {
        if (nullptr != audio_fmt_ctx->streams[i]->codec)
            avcodec_free_context(&audio_fmt_ctx->streams[i]->codec);
    }
    for (int i = 0; i < video_fmt_ctx->nb_streams; ++i) {
        if (nullptr != video_fmt_ctx->streams[i]->codec)
            avcodec_free_context(&video_fmt_ctx->streams[i]->codec);
    }

    av_packet_free(&videoAVPacket);
    av_packet_free(&audioAVPacket);
    avformat_close_input(&audio_fmt_ctx);
    avformat_close_input(&video_fmt_ctx);

    return 0;
}

int MergeVideoAndAudio::openOutputFile(AVFormatContext **out_fmt_ctx, AVStream *videoInStream,
                                       AVStream *audioInStream, const char *outputPath) {
    if (nullptr == videoInStream || nullptr == audioInStream) {
        BZLogUtil::logE("nullptr == videoInStream || nullptr == audioInStream");
        return -1;
    }
    if (videoInStream->duration <= 0 || audioInStream->duration <= 0) {
        BZLogUtil::logE("videoInStream->duration<=0||audioInStream->duration<=0");
        return -1;
    }
    int ret = 0;
    ret = avformat_alloc_output_context2(out_fmt_ctx, NULL, NULL, outputPath);
    if (ret < 0) {
        BZLogUtil::logE("alloc_output_context2 fail");
        return ret;
    }
    if (!out_fmt_ctx) {
        return -1;
    }

    //VideoStream
    AVStream *out_video_stream = avformat_new_stream(*out_fmt_ctx, videoInStream->codec->codec);
    if (!out_video_stream) {
        BZLogUtil::logD("Failed allocating output stream\n");
        ret = AVERROR_UNKNOWN;
        return ret;
    }
    ret = av_dict_copy(&out_video_stream->metadata, videoInStream->metadata, AV_DICT_IGNORE_SUFFIX);
    if (ret < 0) {
        BZLogUtil::logE("Failed allocating output stream\n");
        return ret;
    }
    out_video_stream->time_base = videoInStream->time_base;

    avcodec_copy_context(out_video_stream->codec, videoInStream->codec);
    if (ret < 0) {
        BZLogUtil::logD("Failed to copy context from input to output stream codec context\n");
        return ret;
    }
    ret = avcodec_parameters_from_context(out_video_stream->codecpar, videoInStream->codec);
    if (ret < 0) {
        BZLogUtil::logD("Could not copy the stream parameters\n");
        return -1;
    }
    out_video_stream->index = 0;
    (*out_fmt_ctx)->oformat->codec_tag = 0;
    out_video_stream->codec->codec_tag = 0;
    if ((*out_fmt_ctx)->oformat->flags & AVFMT_GLOBALHEADER)
        out_video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    //audioStream
    AVStream *out_audio_stream = avformat_new_stream(*out_fmt_ctx, audioInStream->codec->codec);
    if (!out_audio_stream) {
        BZLogUtil::logD("Failed allocating output stream\n");
        ret = AVERROR_UNKNOWN;
        return ret;
    }
    ret = av_dict_copy(&out_audio_stream->metadata, audioInStream->metadata, AV_DICT_IGNORE_SUFFIX);
    if (ret < 0) {
        BZLogUtil::logE("Failed allocating output stream\n");
        return ret;
    }
    out_audio_stream->time_base = audioInStream->time_base;

    avcodec_copy_context(out_audio_stream->codec, audioInStream->codec);
    if (ret < 0) {
        BZLogUtil::logD("Failed to copy context from input to output stream codec context\n");
        return ret;
    }
    ret = avcodec_parameters_from_context(out_audio_stream->codecpar, audioInStream->codec);
    if (ret < 0) {
        BZLogUtil::logD("Could not copy the stream parameters\n");
        return -1;
    }
    out_audio_stream->index = 1;
    (*out_fmt_ctx)->oformat->codec_tag = 0;
    out_audio_stream->codec->codec_tag = 0;
    if ((*out_fmt_ctx)->oformat->flags & AVFMT_GLOBALHEADER)
        out_audio_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


//    av_dump_format(*out_fmt_ctx, 0, output_path, 1);

/* open the output file, if needed */
    if (!((*out_fmt_ctx)->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&(*out_fmt_ctx)->pb, outputPath, AVIO_FLAG_WRITE);
        if (ret < 0) {
            BZLogUtil::logD("Could not open '%s': %s\n", outputPath,
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
