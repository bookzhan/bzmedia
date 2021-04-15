//
/**
 * Created by bookzhan on 2021-10-16 12:49.
 *description:
 */
//

#ifndef BZMEDIA_GETVIDEOFRAMEUTIL_H
#define BZMEDIA_GETVIDEOFRAMEUTIL_H


#include <cstdint>
#include <list>
#include <glprogram/AVFrameDrawProgram.h>
#include <glutils/FrameBufferUtils.h>
#include <glprogram/BaseProgram.h>

extern "C" {
#include <libavutil/frame.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libswscale/swscale.h>
}


class GetVideoFrameUtil {
public:
    int
    getVideoFrame(const char *videoPath, int imageCount, int maxWidth, int64_t startTime,
                  int64_t endTime,
                  int64_t callBackHandle,
                  void (*)(int64_t callBackHandle, int index, void *imageData,
                           int width, int height));

    AVFrame *getVideoFrameAtTime(const char *videoPath, int64_t time, int maxWidth);

private:
    AVFormatContext *in_fmt_ctx = nullptr;
    std::list<int64_t> targetVideoTimeList;
    std::list<int64_t> keyVideoTimeList;

    int width = 480, height = 480, videoIndex = -1;
    int outWidth = 0, outHeight = 0;
    int video_rotate = 0;
    int64_t duration = 0;
    AVFrameDrawProgram *avFrameDrawProgram = nullptr;
    FrameBufferUtils *outFrameBufferUtils = nullptr;
    FrameBufferUtils *srcFrameBufferUtils = nullptr;
    BaseProgram *baseProgram = nullptr;
    AVStream *input_video_stream = nullptr;
    SwsContext *sws_video_to_YUV = nullptr;
    AVFrame *yuv420Frame = nullptr;

    int
    initContext(const char *path, int imageCount, int maxWidth, int64_t startTime, int64_t endTime);

    void dealWidthAVFrame(AVFrame *srcAVFrame, AVFrame *targetAVFrame);

    void releaseContext();

    int64_t getListValue(std::list<int64_t> ls, int64_t index);

    int64_t getSeekPts(int64_t currentFramePts);
};


#endif //BZMEDIA_GETVIDEOFRAMEUTIL_H
