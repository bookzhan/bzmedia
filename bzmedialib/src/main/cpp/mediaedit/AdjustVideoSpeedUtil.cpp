//
/**
 * Created by bookzhan on 2019-01-19 18:00.
 * 说明:
 */
//

#include "AdjustVideoSpeedUtil.h"
#include "VideoUtil.h"

int AdjustVideoSpeedUtil::adjustVideoSpeed(const char *srcVideoPath, const char *outputPath,
                                           float speed) {
    if (nullptr == srcVideoPath || nullptr == outputPath || speed <= 0) {
        BZLogUtil::logE("adjustVideoSpeed nullptr==srcVideoPath|| nullptr==outputPath||speed<=0");
        return -1;
    }
    int ret = 0;
    AVFormatContext *inAVFormatContext = nullptr;
    ret = VideoUtil::openInputFileForSoft(srcVideoPath, &inAVFormatContext, false, false);
    if (ret < 0) {
        BZLogUtil::logE("adjustVideoSpeed openInputFileForSoft fail path=%s", srcVideoPath);
        return -1;
    }
    if (nullptr == inAVFormatContext) {
        BZLogUtil::logE("nullptr==inAVFormatContext");
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
        BZLogUtil::logE("adjustVideoSpeed nullptr == inVideoStream");
        return -1;
    }
    AVFormatContext *outAVFormatContext = nullptr;
    ret = VideoUtil::openOutputFile(inAVFormatContext, &outAVFormatContext, outputPath, false);
    if (ret < 0) {
        BZLogUtil::logD("adjustVideoSpeed openOutputFile fail path=%s", outputPath);
        return -1;
    }
    if (nullptr == outAVFormatContext) {
        BZLogUtil::logE("nullptr==outAVFormatContext");
        return -1;
    }
    int64_t videoMaxPts = static_cast<int64_t>(inVideoStream->duration / speed);
    for (int i = 0; i < outAVFormatContext->nb_streams; ++i) {
        AVStream *avStream = outAVFormatContext->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            avStream->duration = videoMaxPts;
            outAVFormatContext->duration = videoMaxPts * 1000000 * avStream->time_base.num /
                                           avStream->time_base.den;
            break;
        }
    }

    AVPacket *avPacket = av_packet_alloc();
    while (true) {
        av_init_packet(avPacket);
        ret = av_read_frame(inAVFormatContext, avPacket);
        if (ret < 0) {
            av_packet_unref(avPacket);
            break;
        }
        if (avPacket->stream_index == inVideoStream->index) {
            avPacket->stream_index = 0;
            avPacket->pts = static_cast<int64_t>(avPacket->pts * 1.0f /
                                                 inVideoStream->duration * videoMaxPts);
            avPacket->dts = static_cast<int64_t>(avPacket->dts * 1.0f /
                                                inVideoStream->duration * videoMaxPts);
            av_interleaved_write_frame(outAVFormatContext, avPacket);
        }
        av_packet_unref(avPacket);

    }
    ret = av_write_trailer(outAVFormatContext);
    if (ret != 0) {
        BZLogUtil::logE("av_write_trailer fail");
    }
    if (outAVFormatContext && !(outAVFormatContext->oformat->flags & AVFMT_NOFILE))
        avio_closep(&outAVFormatContext->pb);
    avformat_free_context(outAVFormatContext);
    avformat_close_input(&inAVFormatContext);
    av_packet_free(&avPacket);

    BZLogUtil::logD("adjustVideoSpeed end");
    return 0;
}
