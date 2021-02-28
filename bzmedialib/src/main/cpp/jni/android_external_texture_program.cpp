//
/**
 * Created by zhandalin on 2018-11-08 18:35.
 * 说明:
 */
//

#include <jni.h>
#include "../glprogram/ImageExternalProgram.h"

extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_glutils_ExternalTextureProgram_initNative(JNIEnv *env, jclass instance,
                                                                     jboolean flipVertical,jboolean needFadeShow) {

    ImageExternalProgram *imageExternalProgram = new ImageExternalProgram(needFadeShow);
    imageExternalProgram->setFlip(false, flipVertical);
    return reinterpret_cast<jlong>(imageExternalProgram);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_glutils_ExternalTextureProgram_setFlip(JNIEnv *env, jclass type,
                                                                  jlong nativeHandle,
                                                                  jboolean needFlipHorizontal,
                                                                  jboolean needFlipVertical) {

    if (nativeHandle == 0) {
        return -1;
    }
    ImageExternalProgram *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    imageExternalProgram->setFlip(needFlipHorizontal, needFlipVertical);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_glutils_ExternalTextureProgram_initGlResource(JNIEnv *env,
                                                                         jclass instance,
                                                                         jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    ImageExternalProgram *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    //有的手机上不能先初始化
//    imageExternalProgram->init();
    return imageExternalProgram->initImageExternal();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_glutils_ExternalTextureProgram_setVideoRotation(JNIEnv *env,
                                                                           jclass instance,
                                                                           jlong nativeHandle,
                                                                           jint rotation) {

    if (nativeHandle == 0) {
        return -1;
    }
    ImageExternalProgram *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    imageExternalProgram->setRotation(rotation);

    return 0;

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_glutils_ExternalTextureProgram_onDrawFrame(JNIEnv *env, jclass instance,
                                                                      jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    ImageExternalProgram *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    return imageExternalProgram->draw();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_glutils_ExternalTextureProgram_releaseGlResource(JNIEnv *env,
                                                                            jclass instance,
                                                                            jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    ImageExternalProgram *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    imageExternalProgram->releaseResource();
    delete imageExternalProgram;
    return 0;
}
