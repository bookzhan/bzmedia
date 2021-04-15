//
/**
 * Created by bookzhan on 2019-02-20 09:56.
 * 说明:
 */
//

#include <jni.h>
#include <glutils/BZRenderEngine.h>

extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_BZRenderEngine_init(JNIEnv *env, jclass type) {
    return reinterpret_cast<jlong>(new BZRenderEngine());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_BZRenderEngine_setFlip(JNIEnv *env, jclass type,
                                                     jlong nativeHandel,
                                                     jboolean flipHorizontal,
                                                     jboolean flipVertical) {
    if (nativeHandel == 0) {
        return;
    }
    auto *bzRenderEngine = reinterpret_cast<BZRenderEngine *>(nativeHandel);
    bzRenderEngine->setFlip(flipHorizontal, flipVertical);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_BZRenderEngine_setFinalSize(JNIEnv *env, jclass type,
                                                          jlong nativeHandel,
                                                          jint finalWidth, jint finalHeight) {

    if (nativeHandel == 0) {
        return;
    }
    auto *bzRenderEngine = reinterpret_cast<BZRenderEngine *>(nativeHandel);
    bzRenderEngine->setFinalSize(finalWidth, finalHeight);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_BZRenderEngine_setTextureHandleInfo(JNIEnv *env, jclass type,
                                                                  jlong nativeHandel,
                                                                  jobject textureHandleInfoObj) {

    if (nativeHandel == 0) {
        return;
    }
    auto *bzRenderEngine = reinterpret_cast<BZRenderEngine *>(nativeHandel);

    if (nullptr != textureHandleInfoObj) {
        jclass textureHandleClass = env->GetObjectClass(textureHandleInfoObj);
        auto *textureHandleInfo = new TextureHandleInfo();
        textureHandleInfo->bgFillType = static_cast<BgFillType>(env->GetIntField(
                textureHandleInfoObj,
                env->GetFieldID(textureHandleClass,
                                "bgFillType",
                                "I")));
        textureHandleInfo->scaleType = static_cast<BZScaleType>(env->GetIntField(
                textureHandleInfoObj,
                env->GetFieldID(textureHandleClass,
                                "scaleType",
                                "I")));

        textureHandleInfo->gaussBlurRadius = env->GetFloatField(textureHandleInfoObj,
                                                                env->GetFieldID(textureHandleClass,
                                                                                "gaussBlurRadius",
                                                                                "F"));

        textureHandleInfo->bgTextureId = env->GetIntField(textureHandleInfoObj,
                                                          env->GetFieldID(textureHandleClass,
                                                                          "bgTextureId",
                                                                          "I"));
        textureHandleInfo->bgTextureWidth = env->GetIntField(textureHandleInfoObj,
                                                             env->GetFieldID(textureHandleClass,
                                                                             "bgTextureWidth",
                                                                             "I"));
        textureHandleInfo->bgTextureHeight = env->GetIntField(textureHandleInfoObj,
                                                              env->GetFieldID(textureHandleClass,
                                                                              "bgTextureHeight",
                                                                              "I"));
        textureHandleInfo->bgTextureIntensity = env->GetFloatField(textureHandleInfoObj,
                                                                   env->GetFieldID(
                                                                           textureHandleClass,
                                                                           "bgTextureIntensity",
                                                                           "F"));

        jobject _backgroundColorObj = env->GetObjectField(textureHandleInfoObj,
                                                          env->GetFieldID(textureHandleClass,
                                                                          "bgColor",
                                                                          "Lcom/luoye/bzmedia/bean/BZColor;"));

        if (nullptr != _backgroundColorObj) {
            jclass jBackgroundColorClass = env->GetObjectClass(_backgroundColorObj);
            textureHandleInfo->bgColor.r = env->GetFloatField(
                    _backgroundColorObj,
                    env->GetFieldID(
                            jBackgroundColorClass,
                            "r",
                            "F"));

            textureHandleInfo->bgColor.g = env->GetFloatField(
                    _backgroundColorObj,
                    env->GetFieldID(
                            jBackgroundColorClass,
                            "g",
                            "F"));

            textureHandleInfo->bgColor.b = env->GetFloatField(
                    _backgroundColorObj,
                    env->GetFieldID(
                            jBackgroundColorClass,
                            "b",
                            "F"));

            textureHandleInfo->bgColor.a = env->GetFloatField(
                    _backgroundColorObj,
                    env->GetFieldID(
                            jBackgroundColorClass,
                            "a",
                            "F"));
            env->DeleteLocalRef(_backgroundColorObj);
        }
        bzRenderEngine->setTextureHandleInfo(textureHandleInfo);
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_BZRenderEngine_setRotation(JNIEnv *env, jclass type,
                                                         jlong nativeHandel,
                                                         jint rotation) {

    if (nativeHandel == 0) {
        return;
    }
    auto *bzRenderEngine = reinterpret_cast<BZRenderEngine *>(nativeHandel);
    bzRenderEngine->setRotation(rotation);

}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_BZRenderEngine_draw(JNIEnv *env, jclass type, jlong nativeHandel,
                                                  jint textureId,
                                                  jint textureWidth, jint textureHeight) {

    if (nativeHandel == 0) {
        return -1;
    }
    auto *bzRenderEngine = reinterpret_cast<BZRenderEngine *>(nativeHandel);
    bzRenderEngine->setTextureSize(textureWidth, textureHeight);
    bzRenderEngine->setTextureId(textureId);

    return bzRenderEngine->draw();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_BZRenderEngine_release(JNIEnv *env, jclass type,
                                                     jlong nativeHandel) {

    if (nativeHandel == 0) {
        return -1;
    }
    auto *bzRenderEngine = reinterpret_cast<BZRenderEngine *>(nativeHandel);
    bzRenderEngine->releaseResource();
    delete (bzRenderEngine);
    return 0;
}