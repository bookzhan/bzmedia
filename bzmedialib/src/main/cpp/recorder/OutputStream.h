/**
 * Created by bookzhan on 2017-03-29 18:03.
 * 说明:
 */

#ifndef BZFFMPEG_OUTPUTSTREAM_H
#define BZFFMPEG_OUTPUTSTREAM_H

extern "C" {
#include <libavutil/frame.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
};

class BZOutputStream {
public:
    AVStream *avStream = NULL;
    AVCodec *avCodec = NULL;

    AVCodecContext *avCodecContext = NULL;
    AVPacket *avPacket = NULL;

    /* pts of the next frame that will be generated */
    int64_t last_pts = -1;
    int samples_count = 0;

    AVFrame *frame = NULL;//用来存储最终要写到文件里面的数据

    //用于视频图像格式转换,音频重采样的临时frame,用来存储最原始的数据
    AVFrame *tmp_frame = NULL;

    //用于滤镜的临时frame
    AVFrame *filter_frame = NULL;

    SwrContext *swr_audio_ctx = NULL;//音频重采样
};

#endif //BZFFMPEG_OUTPUTSTREAM_H
