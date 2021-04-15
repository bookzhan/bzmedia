//
/**
 * Created by bookzhan on 2021~03~11 14:09.
 *description:
 */
//

#ifndef BZMEDIA_ADJUSTEFFECTCONFIG_H
#define BZMEDIA_ADJUSTEFFECTCONFIG_H

#include "BZColor.h"

class AdjustEffectConfig {
public:
    float shadows = 1;//0.45~1
    float highlights = 1;//0.25~1.75
    float contrast = 1;//0.7~1.3
    float fadeAmount = 0;//0~1
    float saturation = 1;//0~2.05
    float shadowsTintIntensity = 0;//0~1
    float highlightsTintIntensity = 0.5;//0.5~1
    BZColor *shadowsTintColor = nullptr;//0~1
    BZColor *highlightsTintColor = nullptr;//0~1
    float exposure = 0;//-1~1
    float warmth = 0;//-1~1
    float green = 1;//0.75~1.25
    float hueAdjust = 0;//-0.174~0.174
    float vignette = 0;//0~1
};

#endif //BZMEDIA_ADJUSTEFFECTCONFIG_H
