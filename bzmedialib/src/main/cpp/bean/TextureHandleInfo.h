//
/**
 * Created by zhandalin on 2018-11-16 15:53.
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
    float gaussBlurRadius = 3;
};

#endif //SLIDESHOWSDK_TEXTUREHANDLEINFO_H
