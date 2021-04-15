//
/**
 * Created by bookzhan on 2018-11-16 15:53.
 * 说明:
 */
//

#ifndef SLIDESHOWSDK_TEXTUREHANDLEINFO_H
#define SLIDESHOWSDK_TEXTUREHANDLEINFO_H

#include "BZColor.h"
#include "BgFillType.h"
#include "BZScaleType.h"

class TextureHandleInfo {
public:
    BgFillType bgFillType = GAUSS_BLUR;
    BZColor bgColor;
    BZScaleType scaleType = NORMAL;
    float gaussBlurRadius = 2;

    int bgTextureId = 0;
    int bgTextureWidth = 0;
    int bgTextureHeight = 0;
    float bgTextureIntensity = 0;
};

#endif //SLIDESHOWSDK_TEXTUREHANDLEINFO_H
