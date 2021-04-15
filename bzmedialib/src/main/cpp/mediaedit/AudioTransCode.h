//
/**
*Created by bookzhan on 2022−04-09 15:16.
*description:
*/
//

#ifndef BZMEDIA_AUDIOTRANSCODE_H
#define BZMEDIA_AUDIOTRANSCODE_H


#include <cstdint>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
};
#ifndef NB_SAMPLES
#define NB_SAMPLES 1024
#endif

class AudioTransCode {
public:
    int startTransCode(const char *inputPath, const char *outputPath, int64_t methodInfoHandle,
                       const char *(*pcmCallBack)(int64_t, const char *pcmData, int length),
                       void (*progressCallBack)(int64_t, float));

private:
    AVFormatContext *in_fmt_ctx = nullptr;
    AVFormatContext *out_fmt_ctx = nullptr;

    int openOutputFile(const char *outPath);

    SwrContext *swr_audio_ctx_2_pcm = nullptr;
    SwrContext *swr_audio_ctx_2_aac = nullptr;
    AVCodecContext *avAudioEncodeCodecContext = nullptr;
    AVFrame *audioFrame = nullptr;
    AVFrame *audioFramePcm = nullptr;
    AVFrame *audioFramePcmTemp = nullptr;
    AVFrame *audioFrameAac = nullptr;
    AVAudioFifo *audioFifo = nullptr;

    int releaseResource();
};


#endif //BZMEDIA_AUDIOTRANSCODE_H
