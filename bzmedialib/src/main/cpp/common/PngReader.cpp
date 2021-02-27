//
/**
 * Created by zhandalin on 2018-04-12 15:02.
 * 说明:
 */
//
extern "C"{
#include <libswscale/swscale.h>
}

#include "PngReader.h"
#include "../mediaedit/VideoUtil.h"
#include "BZLogUtil.h"

ImageTextureInfo *PngReader::getPngInfo(const char *filePath) {

    ImageTextureInfo *imageTextureInfo = new ImageTextureInfo();
    int ret = 0;
    AVFormatContext *in_fmt_ctx = NULL;
    ret = VideoUtil::openInputFileForSoft(filePath, &in_fmt_ctx);
    if (ret < 0 || nullptr == in_fmt_ctx || in_fmt_ctx->nb_streams <= 0) {
        BZLogUtil::logE("getPngInfo openInputFile fail");
        return imageTextureInfo;
    }
    SwsContext *sws_video_ctx = nullptr;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *avStream = in_fmt_ctx->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            imageTextureInfo->stride = avStream->codecpar->width * 4;
            imageTextureInfo->textureWidth = avStream->codecpar->width;
            imageTextureInfo->textureHeight = avStream->codecpar->height;
            if (avStream->codec->pix_fmt != AV_PIX_FMT_RGBA) {
                sws_video_ctx = sws_getContext(avStream->codecpar->width,
                                               avStream->codecpar->height,
                                               avStream->codec->pix_fmt,
                                               avStream->codecpar->width,
                                               avStream->codecpar->height,
                                               AV_PIX_FMT_RGBA,
                                               1, nullptr, nullptr, nullptr);
            }
            break;
        }
    }
    AVPacket *avPacket = av_packet_alloc();
    AVFrame *avFrame = av_frame_alloc();
    int got_picture_ptr = 0;
    AVFrame *avFrameTemp = VideoUtil::allocVideoFrame(AV_PIX_FMT_RGBA,
                                                      imageTextureInfo->textureWidth,
                                                      imageTextureInfo->textureHeight);

    while (true) {
        av_init_packet(avPacket);
        ret = av_read_frame(in_fmt_ctx, avPacket);
        if (ret < 0) {
            break;
        }
        AVStream *avStream = in_fmt_ctx->streams[avPacket->stream_index];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ret = avcodec_decode_video2(avStream->codec, avFrame, &got_picture_ptr, avPacket);
            if (ret < 0) {
                BZLogUtil::logE("getPngInfo avcodec_decode_video2 fail");
                break;
            }
            if (got_picture_ptr != 0) {
                if (nullptr != sws_video_ctx) {
                    sws_scale(sws_video_ctx,
                              (const uint8_t *const *) avFrame->data,
                              avFrame->linesize, 0, avFrameTemp->height,
                              avFrameTemp->data,
                              avFrameTemp->linesize);
                    size_t length = sizeof(char) * avFrameTemp->width * avFrameTemp->height * 4;
                    char *buffer = (char *) malloc(length);
                    memcpy(buffer, avFrameTemp->data[0], length);
                    imageTextureInfo->data = buffer;
                } else {
                    size_t length = sizeof(char) * avFrame->width * avFrame->height * 4;
                    char *buffer = (char *) malloc(length);
                    memcpy(buffer, avFrame->data[0], length);
                    imageTextureInfo->data = buffer;
                }
                break;
            }
        }
    }
    av_packet_free(&avPacket);
    av_frame_free(&avFrame);
    av_frame_free(&avFrameTemp);
    if (nullptr != in_fmt_ctx) {
        for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
            if (nullptr != in_fmt_ctx->streams[i]->codec) {
                avcodec_close(in_fmt_ctx->streams[i]->codec);
                in_fmt_ctx->streams[i]->codec = nullptr;
            }
        }
        avformat_close_input(&in_fmt_ctx);
        in_fmt_ctx = nullptr;
    }
    if (nullptr != sws_video_ctx) {
        sws_freeContext(sws_video_ctx);
    }
//    BZLogUtil::logD("getPngInfo 耗时=%lld path=%s", getCurrentTime() - startTime, filePath);
    free((void *) filePath);
    return imageTextureInfo;
}
