//
/**
 * Created by bookzhan on 2019-02-21 10:25.
 * 说明:根据时间点获取视频纹理帧,需要在GL线程调用
 */
//

#ifndef BZMEDIA_VIDEOFRAMEGETTER_H
#define BZMEDIA_VIDEOFRAMEGETTER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
};

#include <bean/FilteringContext.h>
#include <cstdint>
#include <glutils/FrameBufferUtils.h>
#include <glprogram/AVFrameDrawProgram.h>


class VideoFrameGetter {
public:
    int init(const char *videoPath, bool userSoftDecode);

    int setStartTime(int64_t startTime);

    int setEnableLoop(bool enableLoop);

    int getVideoWidth();

    int getVideoHeight();

    int64_t getVideoDuration();

    int getVideoFrame(int64_t currentTime);

    AVFrame *getVideoFrameData(int64_t currentTime);

    //外面一层需要存储的对象
    void setObjectHandle(int64_t objectHandle);

    int64_t getObjectHandle();

    int release();

private:
    AVFormatContext *in_fmt_ctx = nullptr;
    AVStream *videoStream = nullptr;
    AVCodecContext *videoCodecContext = nullptr;
    AVFrame *videoFrame = nullptr;
    AVFrame *videoFrameRGBA = nullptr;
    AVFrame *lastDrawAVFrame = nullptr;
    AVPacket *decodePacket = nullptr;
    AVFrameDrawProgram *avFrameDrawProgram = nullptr;
    FrameBufferUtils *frameBufferUtils = nullptr;
    bool hasFlush = false;
    bool enableLoop = false;

    int videoRotate = 0;
    int videoWidth = 0;
    int videoHeight = 0;
    int64_t videoDuration = 0;
    int64_t keyFrameSpaceTime = 1000;
    int64_t timeTag = -1;

    //返回一个纹理
    int drawFrame(AVFrame *avFrame);

    AVFrame *handleFrame(AVFrame *avFrame);

    AVFrame *decodeAnVideoFrame();

    long logCount = 0;
    const int LOG_SPACE = 100;
    const int TARGET_FRAME_RATE = 30;
    //平均每一帧,导致要降低整理的基数
    float frameIncreaseExtent = -1;
    int64_t videoFrameIndex = 0;
    int64_t skipFrameCount = 0;
    float skipFrameBufferCount = 0;
    bool userSoftDecode = true;
    SwsContext *rgba_sws_video_ctx = nullptr;
    //外面一层需要存储的对象
    int64_t objectHandle = 0;

    AVFrame *getAVideoFrame(int64_t currentTime);
};


#endif //BZMEDIA_VIDEOFRAMEGETTER_H
