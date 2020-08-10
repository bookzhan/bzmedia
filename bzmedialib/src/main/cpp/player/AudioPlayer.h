//
/**
 * Created by zhandalin on 2020-08-10 15:47.
 *description:
 */
//

#ifndef BZMEDIA_AUDIOPLAYER_H
#define BZMEDIA_AUDIOPLAYER_H
extern "C" {
#include <include/libavutil/opt.h>
#include <include/libavformat/avformat.h>
#include <include/libswresample/swresample.h>
#include <include/libavutil/frame.h>
#include <include/libavutil/audio_fifo.h>
}

#include <cstdint>
#include <mutex>
#include "PCMPlayerNative.h"


using namespace std;

class AudioPlayer {
public:
    AudioPlayer();

    int init(const char *audioPath, int64_t  = 0,
             void (*progressCallBack)(int64_t javaHandle, float progress) = nullptr);

    int seek(int64_t time);

    int setPlayLoop(bool isLoop);

    void setAudioVolume(float volume);

    int64_t getDuration();

    int pause();

    int start();

    int stop();

    int release();

    ~AudioPlayer();

private:
    const int NB_SAMPLES = 2048;
    const int SAMPLE_RATE = 44100;
    const int LOG_SPACE = 30;
    int64_t logCount = 0;
    int64_t methodHandle = 0;
    void (*progressCallBack)(int64_t javaHandle, float progress)=nullptr;

    SwrContext *swr_audio_ctx = nullptr;
    AVFrame *audioFrame = nullptr;
    AVAudioFifo *audioFifo = nullptr;
    mutex audioCodecLock;
    bool isLoop = false;
    float volume = 1;

    PCMPlayerNative *pcmPlayer = nullptr;
    AVFormatContext *in_fmt_ctx = nullptr;
    AVStream *audioStream = nullptr;
    int64_t audioStreamTotalTime = 0;
    AVCodecContext *audioCodecContext = nullptr;

    atomic_bool audioPlayThreadIsEnd = {true};

    void audioPlayThread();

    void onPCMDataAvailable(const char *pcmData, int length);

    bool videoPlayerIsRelease = false;

    void callBackProgress(float progress);

    bool playerIsPause = true;

    int64_t audioPlayTime = 0;
    bool requestSetVolume = false;
    bool requestSeek = false;
    int64_t seekAudioPts = 0;
};


#endif //BZMEDIA_AUDIOPLAYER_H
