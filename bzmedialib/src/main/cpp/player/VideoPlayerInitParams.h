//
/**
 * Created by bookzhan on 2018-01-03 10:01.
 * 说明:视频播放器初始化需要的参数
 */
//

#ifndef BZFFMPEG_VIDEOPLAYERINITPARAMS_H
#define BZFFMPEG_VIDEOPLAYERINITPARAMS_H

#include <cstdint>

typedef struct VideoPlayerInitParams {
    const char *videoPath = nullptr;
    bool userSoftDecode = false;
    int64_t methodHandle = 0;

    void (*onVideoInfoAvailableCallBack)(int64_t javaHandle, int, int, int, int64_t, float)=nullptr;

    void (*progressCallBack)(int64_t javaHandle, float progress)=nullptr;

    void (*onVideoPlayCompletionCallBack)(int64_t javaHandle, int)=nullptr;

    //同步准备
    bool prepareSyn = false;
} VideoPlayerInitParams;

#endif //BZFFMPEG_VIDEOPLAYERINITPARAMS_H
