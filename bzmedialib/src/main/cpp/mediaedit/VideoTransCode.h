//
/**
 * Created by zhandalin on 2018-01-11 17:11.
 * 说明:
 */
//

#ifndef BZFFMPEG_VIDEOTRANSCODE_H
#define BZFFMPEG_VIDEOTRANSCODE_H
extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
};

#include "VideoTranscodeParams.h"
#include "../glutils/FrameBufferUtils.h"
#include <list>
#include <atomic>
#include <glprogram/AVFrameDrawProgram.h>
#include <glutils/EGLContextUtil.h>
#include <glutils/TextureConvertYUVUtil.h>

using namespace std;

#ifndef NB_SAMPLES
#define NB_SAMPLES 1024
#endif

//视频转码类
class VideoTransCode {
public:
    int startTransCode(VideoTransCodeParams *videoTransCodeParams);

    void stopVideoTransCode();

    void setMethodInfoHandle(int64_t methodInfoHandle);

    int64_t getMethodInfoHandle();

protected:
    virtual AVFrame *videoFrameDoWith(AVCodecContext *decodeContext, AVFrame *srcFrame);

    virtual int releaseResource();

    //平均帧率
    float avg_frame_rate = 25;
    int64_t avg_frame_duration = 33333;
    //如果设置了指定的帧率的话,这个参数会自动计算,间隔多少帧舍去一帧
    float spaceFrame = -1;
    long videoDecodeFrameCount = 0;
    long videoEncodeFrameCount = 0;

    long skipFreameCount = 1;
private:
    AVFormatContext *in_fmt_ctx = NULL;
    AVFormatContext *out_fmt_ctx = NULL;
    AVCodecContext *avVideoEncodeCodecContext = NULL;
    AVCodecContext *avAudioEncodeCodecContext = NULL;

    AVStream *videoStream = nullptr;

    AVFrame *audioFrame = NULL;
    AVFrame *audioFramePcm = NULL;
    AVFrame *audioFramePcmTemp = NULL;
    AVFrame *audioFrameAac = NULL;
    AVAudioFifo *audioFifo = NULL;

    //各种回调
    const char *(*pcmCallBack)(int64_t, const char *pcmData, int length)=NULL;

    int openOutputFile(const char *outputPath);

    int flushBuffer();

    bool isStopVideoTransCode;

    atomic_bool isTraning = {true};

    //用解码出来的AVCodecContext 不好使,自己写了
    AVFrame *videoFrame = NULL;
    int videoWidth = 0, videoHeight = 0;

    int finalVideoRotate = 0;

    int srcVideoRotate = 0;

    void (*progressCallBack)(int64_t, float)=NULL;

    VideoTransCodeParams *videoTransCodeParams = NULL;
    SwrContext *swr_audio_ctx_2_pcm = NULL;
    SwrContext *swr_audio_ctx_2_aac = NULL;

    EGLContextUtil *eglContextUtil = NULL;
    AVFrameDrawProgram *avFrameDrawProgram = NULL;
    TextureConvertYUVUtil *textureConvertYUVUtil = NULL;
    FrameBufferUtils *videoSrcframeBufferUtils = NULL;
    int64_t methodInfoHandle = 0;
};

#endif //BZFFMPEG_VIDEOTRANSCODE_H
