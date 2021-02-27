//
/**
 * Created by zhandalin on 2017-10-12 14:48.
 * 说明:通过文件名获取对应的纹理id
 */
//

#ifndef BZFFMPEG_GLIMAGETEXTUREUTIL_H
#define BZFFMPEG_GLIMAGETEXTUREUTIL_H


#include "ImageTextureInfo.h"

class GLImageTextureUtil {
public:
    static char *
    (*imageTextureCallBack)(const char *, int rotate, int flipHorizontal,
                            int flipVertical);

    static int
    (*circleTextureCallBack)(int width, int height);

    static int
    (*getRhombusTextureCallBack)(int width, int height);


    static ImageTextureInfo *
    getTextureIdFromAssets(const char *fileName, int rotate, int flipHorizontal,
                           int flipVertical);

    static ImageTextureInfo *
    getTextureIdFromSDCard(const char *fileName, int rotate, int flipHorizontal,
                           int flipVertical);


    static ImageTextureInfo *getBitmap(const char *fileName);

    //gl线程调用
    static int genTextureFromImageData(void *imageData, int width, int height);

    //gl线程调用
    static int getCircleTexture(int width, int height);
};


#endif //BZFFMPEG_GLIMAGETEXTUREUTIL_H
