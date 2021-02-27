//
/**
 * Created by zhandalin on 2018-08-13 11:11.
 * 说明:
 */
//

#ifndef SLIDESHOWSDK_GAUSSBLURPROGRAM_H
#define SLIDESHOWSDK_GAUSSBLURPROGRAM_H


#include "BaseProgram.h"
#include <glutils/FrameBufferUtils.h>

class GaussBlurProgram : public BaseProgram {
public:
    GaussBlurProgram();

    void setRadius(float radius);

    //用来规范纹理的,一般要使得纹理填充满ViewPort
    int setSize(int textureWidth, int textureHeight, int viewPortWidth, int viewPortHeight,
                bool fitFullViewPort);

    int draw();

    int releaseResource();

protected:
    int initProgram(const char *vertexShader, const char *fragmentShader);

    //必须要横竖画两次
    void setDrawHorizontal(bool drawHorizontal);

    float radius = 2.0f;
    GLint texelWidthOffsetLocation = 0;
    GLint texelHeightLocation = 0;

    FrameBufferUtils *frameBufferUtils_1 = nullptr;
    FrameBufferUtils *frameBufferUtils_2 = nullptr;

    int textureWidth = 0, textureHeight = 0, viewPortWidth = 0, viewPortHeight = 0;

    int finalViewPortX = 0, finalViewPortY = 0, finalViewPortWidth = 0, finalViewPortHeight = 0;

    //默认会填充满整个ViewPort
    bool fitFullViewPort = true;
};


#endif //SLIDESHOWSDK_GAUSSBLURPROGRAM_H
