
/**
 * Created by zhandalin on 2017-06-12 17:27.
 * 说明:
 */


#ifndef BZFFMPEG_CLIPVIDEOFRAMETOIMAGE_H
#define BZFFMPEG_CLIPVIDEOFRAMETOIMAGE_H


#include <cstdint>
#include <glprogram/AVFrameDrawProgram.h>
#include <glutils/FrameBufferUtils.h>

extern "C" {
#include <libavutil/frame.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libswscale/swscale.h>
};

class ClipVideoFrameToImage {
public:
    int
    clipVideoFrameToImage(const char *videoPath, const char *outImageParentPath, int imageCount,
                          int scale2Width, int64_t callBackHandle,
                          void (*)(int64_t callBackHandle, int index, const char *imagePath));


    int
    clipVideoFrameToImage(const char *videoPath, int imageCount, int scale2Width,
                          int64_t callBackHandle,
                          void (*)(int64_t callBackHandle, int index, void *imageData,
                                   int width, int height));


    int
    clipVideoFrameToImageAtTime(const char *videoPath, const char *outImagePath,
                                int64_t time);

    AVFrame *clipVideoFrameAtTime(const char *videoPath, int64_t time);

private:
    AVFormatContext *in_fmt_ctx = NULL;
    AVFormatContext *out_fmt_ctx = NULL;
    AVCodecContext *videoCodecContext = NULL;

    AVFrameDrawProgram *avFrameDrawProgram = nullptr;
    FrameBufferUtils *frameBufferUtils = nullptr;

    int video_rotate = 0, outWidth = 480, outHeight = 480;

    SwsContext *sws_nv_2_yuv420p = nullptr;
    AVFrame *nv_2_yuv420pFrame = nullptr;

    AVStream *input_video_stream = NULL;

    AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);

    int saveFrame(const char *fileName, AVFrame *videoFrame);

    const char *outImageParentPath = NULL;

    void (*getImageFromVideoCallBack)(int64_t callBackHandle, int index,
                                      const char *imagePath) = NULL;

    void (*getImageDataFromVideoCallBack)(int64_t callBackHandle, int index, void *imageData,
                                          int width, int height) = NULL;

    int getVideoFrame(int imageCount, int scale2Width);

    void dealWidthAVFrame(AVFrame *srcAVFrame, AVFrame *targetAVFrame);

    int openInputFile(const char *filename);

    int64_t callBackHandle = 0;
};


#endif //BZFFMPEG_CLIPVIDEOFRAMETOIMAGE_H
