//
// Created by 000 on 2018/11/19.
//

#ifndef SLIDESHOWSDK_ADJUSTCONFIGUTIL_H
#define SLIDESHOWSDK_ADJUSTCONFIGUTIL_H


#include <bean/AdjustEffectConfig.h>
#include <jni.h>

class AdjustConfigUtil {
public:
    static AdjustEffectConfig *getAdjustConfig(JNIEnv *env, jobject pAdjustEffectConfigObj);
};


#endif //SLIDESHOWSDK_ADJUSTCONFIGUTIL_H
