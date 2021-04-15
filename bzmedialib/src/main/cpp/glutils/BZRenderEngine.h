//
/**
 * Created by bookzhan on 2019-01-22 14:24.
 * 说明:全部API都需要在GL环境中调用
 */
//

#ifndef SLIDESHOWSDK_BZRENDERENGINE_H
#define SLIDESHOWSDK_BZRENDERENGINE_H


#include <glprogram/BaseProgram.h>
#include <glprogram/TextureYUVProgram.h>
#include "VideoTextureManger.h"
#include <bean/TextureHandleInfo.h>

class BZRenderEngine {
public:
    BZRenderEngine();

    void setFlip(bool flipHorizontal, bool flipVertical);

    void setFinalSize(int finalWidth, int finalHeight);

    void setTextureHandleInfo(TextureHandleInfo *textureHandleInfo);

    void setRotation(int rotation);

    void setTextureSize(int textureWidth, int textureHeight);

    int setTextureId(int textureId);

    void setTextureId(int textureIdY, int textureIdUV);

    int draw();

    int releaseResource();

    ~BZRenderEngine();

private:
    BaseProgram *baseProgram = nullptr;
    TextureYUVProgram *textureYUVProgram = nullptr;

    bool flipHorizontal = false;
    bool flipVertical = false;
    int rotation = 0;
    int finalWidth = 0, finalHeight = 0;
    VideoTextureManger *videoTextureManger = nullptr;
    FrameBufferUtils *frameBufferUtils = nullptr;
    int textureWidth = 720, textureHeight = 720;
    TextureInfo textureInfo;
};


#endif //SLIDESHOWSDK_BZRENDERENGINE_H
