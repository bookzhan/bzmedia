//
/**
 * Created by zhandalin on 2018-11-14 10:49.
 * 说明:管理MpGetTextureNative 共三个,建立一个队列,在gl线程调用
 */
//

#ifndef SLIDESHOWSDK_VIDEOTEXTUREMANGER_H
#define SLIDESHOWSDK_VIDEOTEXTUREMANGER_H


#include <bean/TextureInfo.h>
#include <list>
#include <map>
#include <bean/BgFillType.h>
#include <bean/BZColor.h>
#include <bean/TextureHandleInfo.h>
#include "FrameBufferUtils.h"
#include <glprogram/GaussBlurProgram.h>


using namespace std;

class VideoTextureManger {
public:
    void setUniformTextureSize(int uniformTextureWidth, int uniformTextureHeight);

    TextureInfo *getUniformTexture(TextureInfo *textureInfo);

    void setGaussBlurProgramRadius(float gaussBlurRadius);

    void release();

    void setTextureHandleInfo(TextureHandleInfo *textureHandleInfo);

private:
    //最少必须为2个否则会重复建立,销毁,很耗时
    const int BUFFER_COUNT = 2;
    TextureInfo *errorTextureInfo = nullptr;
    TextureInfo tempTextureInfo;
    bool isPreviewMode = true;
    bool userSoftDecode = false;

    int uniformTextureWidth = 0;
    int uniformTextureHeight = 0;
    GaussBlurProgram *gaussBlurProgram = nullptr;
    BaseProgram *baseProgram = nullptr;
    FrameBufferUtils *uniformSizeFrameBufferUtils = nullptr;
    TextureHandleInfo *textureHandleInfo = nullptr;
    float volume = 1;
    bool videoPlayerIsPause = true;

    float gaussBlurRadius = 2;
};


#endif //SLIDESHOWSDK_VIDEOTEXTUREMANGER_H
