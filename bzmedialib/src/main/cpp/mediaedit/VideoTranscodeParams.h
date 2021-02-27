//
/**
 * Created by zhandalin on 2018-01-11 17:12.
 * 说明:
 */
//

#ifndef BZFFMPEG_VIDEOTRANSCODEPARAMS_H
#define BZFFMPEG_VIDEOTRANSCODEPARAMS_H

#include <clocale>

typedef struct VideoTranscodeParams {
    const char *inputPath = NULL;
    const char *outputPath = NULL;

    void (*progressCallBack)(int64_t, float) = NULL;

    int (*videoTextureCallBack)(int64_t methodInfo, int textureId, int width, int height,
                                int64_t pts,
                                int64_t videoTime) = NULL;

    const char *(*pcmCallBack)(int64_t, const char *pcmData, int length) = NULL;

    void (*videoTransCodeFinish)(int64_t) = NULL;

    int gopSize = 30;

    bool doWithVideo = true;
    bool doWithAudio = false;

    bool needCallBackVideo = false;


    int maxWidth = -1;

    //优先级最高
    int targetWidth = -1;
    int targetHeight = -1;
    //控制开始的时间
    int64_t startTime = -1;

    //控制结束的时间
    int64_t endTime = -1;

    //帧率控制
    int frameRate = -1;

    //视频旋转角度
    int videoRotate = 0;
    bool userSoftDecode = false;
} VideoTransCodeParams;

#endif //BZFFMPEG_VIDEOTRANSCODEPARAMS_H
