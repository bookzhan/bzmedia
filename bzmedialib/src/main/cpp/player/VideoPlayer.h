//
/**
 * Created by bookzhan on 2019-02-13 10:38.
 * 说明:
 */
//

#ifndef BZMEDIA_VIDEOPLAYERNEW_H
#define BZMEDIA_VIDEOPLAYERNEW_H


#include <atomic>
#include "VideoPlayerInitParams.h"
#include "PCMPlayerNative.h"
#include "AVFrameDeque.h"
#include "AVPacketDeque.h"
#include <mediaedit/VideoUtil.h>
#include <glprogram/AVFrameDrawProgram.h>
#include <deque>
#include <mutex>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/opt.h>
};

using namespace std;

class VideoPlayer {
public:
    int init(VideoPlayerInitParams *videoPlayerInitParams);

    int reStart();

    int start();

    //gl 线程调用
    int onPause();

    int pause();

    //time 单位毫秒
    void seek(int64_t time);

    int release();

    void setAudioVolume(float volume);

    int pcmPlayerStart();

    int pcmPlayerPause();

    //gl 线程调用,返回当前播放时间,time 单位毫秒
    virtual int64_t onDraw(int64_t time);

    void setPlayLoop(bool isLoop);

    ~VideoPlayer();

    int64_t getCurrentAudioPts();

    int getVideoWidth();

    int getVideoHeight();

    VideoPlayerInitParams * getVideoPlayerInitParams();

    bool isPlaying();

protected:
    int64_t getPlayTime();

    int64_t videoDuration = 0;

    bool requestSeekToStart = false;

    virtual void onVideoPlayCompletion(bool isPlayCompletion);

    atomic_bool videoDecodeSuccess = {false};

private:
    VideoPlayerInitParams *videoPlayerInitParams = nullptr;
    const int NB_SAMPLES = 2048;
    const int SAMPLE_RATE = 44100;
    //读包缓存的数量,越大越有利于提速
    const int PACKET_BUFFER_SIZE = 10;

    atomic_bool videoPlayerIsPause = {true};
    atomic_bool videoPlayerIsRelease = {false};
    atomic_bool videoDecodeThreadIsEnd = {true};
    atomic_bool audioPlayThreadIsEnd = {true};
    atomic_bool audioPlayIsPause = {false};

    PCMPlayerNative *pcmPlayer = nullptr;
    AVFormatContext *in_fmt_ctx = nullptr;

    int64_t videoPlayTime = 0, audioPlayTime = 0;
    int averageDuration = 33;
    AVStream *videoStream = nullptr, *audioStream = nullptr;

    bool innerInitSuccessFlag = true;
    AVCodecContext *videoCodecContext = nullptr, *audioCodecContext = nullptr;
    int videoWidth = 720, videoHeight = 720;
    int64_t videoStreamTotalTime = 1, audioStreamTotalTime = 1;
    int videoRotate = 0;
    SwrContext *swr_audio_ctx = nullptr;
    AVFrame *audioFrame = nullptr;
    AVAudioFifo *audioFifo = nullptr;

    AVPacketDeque videoPacketDeque;
    AVPacketDeque audioPacketDeque;
    AVFrameDeque videoAVFrameDeque;
    //帧缓存最大数量
    const int VIDEO_AVFRAME_DEQUE_MAX_SIZE = 10;

    //线程同步锁
    mutex videoCodecLock;
    mutex audioCodecLock;
    mutex fmt_ctxLock;
    mutex resourceReleaseLock;
    mutex playTimeLock;
    mutex pcmPlayerLock;

    AVFrame *lastDrawAVFrame = nullptr;
    int64_t lastVideoDrawPts = 0;
    int64_t lastAudioPlayPts = 0;
    AVFrameDrawProgram *avFrameDrawProgram = nullptr;
    float volume = 1;
    int64_t logCount = 0;
    const int LOG_SPACE = 30;
    bool requestSetVolume = false;
    bool videoPlayCompletionHasCallBacked = false;
    //是否循环播放
    bool isLoop = false;

    int pcmPlayerInit();

    void onPCMDataAvailable(const char *pcmData, int length);

    int pcmPlayerRelease();

    void decodeVideoThread();

    int decodeVideo();

    int flushDecodeVideo();

    void audioPlayThread();

    int allocResource();

    //time 单位毫秒
    void seekInner(int64_t videoPts, int64_t audioPts);

    int readPacket();

    int64_t drawFrame(AVFrame *avFrame);

    void callBackProgress(float progress);

    AVFrame *getFinalAVFrame(int64_t currentTime);

    int releaseResource();

    void setPlayTime();

    bool lastAVFrameEnable();

};


#endif //BZMEDIA_VIDEOPLAYERNEW_H
