//
/**
 * Created by bookzhan on 2021-03-11 13:58.
 *description:
 */
//

#ifndef BZMEDIA_ADJUSTPROGRAM_H
#define BZMEDIA_ADJUSTPROGRAM_H


#include <bean/AdjustEffectConfig.h>
#include "BaseProgram.h"

class AdjustProgram : public BaseProgram {
public:
    AdjustProgram();

    void setAdjustConfig(AdjustEffectConfig *adjustEffectConfig);

     int releaseResource();
protected:
    int initProgram(const char *vertexShader, const char *fragmentShader);

    int drawArraysBefore();

    void releaseAdjustEffectConfig(AdjustEffectConfig *adjustEffectConfig);

    GLint shadowsLoc;
    GLint highlightsLoc;
    GLint contrastLoc;
    GLint fadeAmountLoc;
    GLint saturationLoc;
    GLint shadowsTintIntensityLoc;
    GLint highlightsTintIntensityLoc;
    GLint shadowsTintColorLoc;
    GLint highlightsTintColorLoc;
    GLint exposureLoc;
    GLint warmthLoc;
    GLint greenLoc;
    GLint hueAdjustLoc;
    GLint vignetteLoc;

    AdjustEffectConfig *adjustEffectConfig = nullptr;
};


#endif //BZMEDIA_ADJUSTPROGRAM_H
