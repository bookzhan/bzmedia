//
// Created by luoye on 2017/6/18.
//

#ifndef BZFFMPEG_VIDEORECORDPARAMS_H
#define BZFFMPEG_VIDEORECORDPARAMS_H

#include <cwchar>
#include <list>

using namespace std;
typedef struct VideoRecordParams {
    const char *output_path = NULL;
    int inputWidth = 0;
    int inputHeight = 0;
    int targetWidth = 0;
    int targetHeight = 0;
    int videoFrameRate = 0;
    int nbSamples = 0;
    int sampleRate = 0;
    int videoRotate = 0;
    int pixelFormat = 0;
    bool hasAudio = true;
    bool needFlipVertical = false;
    bool allFrameIsKey = false;
    int64_t bit_rate = 1024 * 1024 * 8;//每秒1M;
    list<int64_t> *videoPts = nullptr;
    list<int64_t> *audioPts = nullptr;

    //视频是否同步编码
    bool synEncode = false;

    bool avPacketFromMediaCodec = false;
} VideoRecordParams;

#endif //BZFFMPEG_VIDEORECORDPARAMS_H
