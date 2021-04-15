//
// Created by 000 on 2018/11/19.
//

#include "AdjustConfigUtil.h"
#include <common/BZLogUtil.h>

AdjustEffectConfig *AdjustConfigUtil::getAdjustConfig(JNIEnv *env,
                                                      jobject pAdjustEffectConfigObj) {
    if (nullptr == env || nullptr == pAdjustEffectConfigObj) {
        BZLogUtil::logE("nullptr==env|| nullptr==pAdjustEffectConfigObj return");
        return nullptr;
    }
    AdjustEffectConfig *adjustEffectConfig = new AdjustEffectConfig();
    adjustEffectConfig->shadowsTintColor = new BZColor();
    adjustEffectConfig->highlightsTintColor = new BZColor();

    jclass jAdjustComfigClass = env->GetObjectClass(pAdjustEffectConfigObj);
    adjustEffectConfig->shadows = static_cast<float >(env->GetFloatField(
            pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass, "shadows", "F")));

    adjustEffectConfig->highlights = static_cast<float >(env->GetFloatField(
            pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass, "highlights", "F")));

    adjustEffectConfig->contrast = static_cast<float >(env->GetFloatField(
            pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass, "contrast", "F")));

    adjustEffectConfig->fadeAmount = static_cast<float >(env->GetFloatField(
            pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass, "fadeAmount", "F")));

    adjustEffectConfig->saturation = static_cast<float >(
            env->GetFloatField(pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass,
                                                                       "saturation", "F")));

    adjustEffectConfig->shadowsTintIntensity = static_cast<float >(env->GetFloatField(
            pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass,
                                                    "shadowsTintIntensity", "F")));

    adjustEffectConfig->highlightsTintIntensity = static_cast<float >(env->GetFloatField(
            pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass,
                                                    "highlightsTintIntensity", "F")));

    jobject shadowsTintColorObj = env->GetObjectField(
            pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass, "shadowsTintColor",
                                                    "Lcom/luoye/bzmedia/bean/BZColor;"));

    jobject highlightsTintColorObj = env->GetObjectField(
            pAdjustEffectConfigObj, env->GetFieldID(jAdjustComfigClass,
                                                    "highlightsTintColor",
                                                    "Lcom/luoye/bzmedia/bean/BZColor;"));
    jclass bzColorClass = env->FindClass("com/luoye/bzmedia/bean/BZColor");

    adjustEffectConfig->shadowsTintColor->r = static_cast<float >(
            env->GetFloatField(shadowsTintColorObj,
                               env->GetFieldID(bzColorClass, "r", "F")));

    adjustEffectConfig->shadowsTintColor->g = static_cast<float >(env->GetFloatField(
            shadowsTintColorObj, env->GetFieldID(bzColorClass, "g", "F")));

    adjustEffectConfig->shadowsTintColor->b = static_cast<float >(env->GetFloatField(
            shadowsTintColorObj, env->GetFieldID(bzColorClass, "b", "F")));
    adjustEffectConfig->shadowsTintColor->a = static_cast<float >(env->GetFloatField(
            shadowsTintColorObj, env->GetFieldID(bzColorClass, "a", "F")));

    adjustEffectConfig->highlightsTintColor->r = static_cast<float >(env->GetFloatField(
            highlightsTintColorObj, env->GetFieldID(bzColorClass, "r", "F")));
    adjustEffectConfig->highlightsTintColor->g = static_cast<float >(env->GetFloatField(
            highlightsTintColorObj, env->GetFieldID(
                    bzColorClass, "g", "F")));
    adjustEffectConfig->highlightsTintColor->b = static_cast<float >(env->GetFloatField(
            highlightsTintColorObj, env->GetFieldID(bzColorClass, "b", "F")));
    adjustEffectConfig->highlightsTintColor->a = static_cast<float >(env->GetFloatField(
            highlightsTintColorObj, env->GetFieldID(bzColorClass, "a", "F")));

    adjustEffectConfig->exposure = static_cast<float >(
            env->GetFloatField(pAdjustEffectConfigObj, env->GetFieldID(
                    jAdjustComfigClass, "exposure", "F")));
    adjustEffectConfig->warmth = static_cast<float >(
            env->GetFloatField(pAdjustEffectConfigObj, env->GetFieldID(
                    jAdjustComfigClass, "warmth", "F")));
    adjustEffectConfig->green = static_cast<float >(
            env->GetFloatField(pAdjustEffectConfigObj, env->GetFieldID(
                    jAdjustComfigClass, "green", "F")));
    adjustEffectConfig->hueAdjust = static_cast<float >(
            env->GetFloatField(pAdjustEffectConfigObj, env->GetFieldID(
                    jAdjustComfigClass, "hueAdjust", "F")));
    adjustEffectConfig->vignette = static_cast<float >(
            env->GetFloatField(pAdjustEffectConfigObj, env->GetFieldID(
                    jAdjustComfigClass, "vignette", "F")));
    env->DeleteLocalRef(jAdjustComfigClass);
    return adjustEffectConfig;
}
