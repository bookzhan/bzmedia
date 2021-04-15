//
/**
 * Created by bookzhan on 2021-03-30 10:17.
 *description:
 */
//

#include <jni.h>
#include <glprogram/AdjustProgram.h>
#include <common/BZLogUtil.h>
#include <utils/AdjustConfigUtil.h>
#include <glprogram/ImageExternalProgram.h>
#include <glutils/CropTextureUtil.h>
#include <glprogram/YUV420DrawProgram.h>
#include <glprogram/MosaicProgram.h>
#include <glprogram/ZoomProgram.h>

extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_ExternalTextureProgram_initNative(JNIEnv *env, jclass instance,
                                                                jboolean flipVertical,
                                                                jboolean needFadeShow) {

    auto *imageExternalProgram = new ImageExternalProgram(needFadeShow);
    imageExternalProgram->setFlip(false, flipVertical);
    return reinterpret_cast<jlong>(imageExternalProgram);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_ExternalTextureProgram_setFlip(JNIEnv *env, jclass type,
                                                             jlong nativeHandle,
                                                             jboolean needFlipHorizontal,
                                                             jboolean needFlipVertical) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    imageExternalProgram->setFlip(needFlipHorizontal, needFlipVertical);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_ExternalTextureProgram_initGlResource(JNIEnv *env,
                                                                    jclass instance,
                                                                    jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    //有的手机上不能先初始化
//    imageExternalProgram->init();
    return imageExternalProgram->initImageExternal();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_ExternalTextureProgram_setVideoRotation(JNIEnv *env,
                                                                      jclass instance,
                                                                      jlong nativeHandle,
                                                                      jint rotation) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    imageExternalProgram->setRotation(rotation);

    return 0;

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_ExternalTextureProgram_onDrawFrame(JNIEnv *env, jclass instance,
                                                                 jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    return imageExternalProgram->draw();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_ExternalTextureProgram_releaseGlResource(JNIEnv *env,
                                                                       jclass instance,
                                                                       jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *imageExternalProgram = reinterpret_cast<ImageExternalProgram *>(nativeHandle);
    imageExternalProgram->releaseResource();
    delete imageExternalProgram;
    return 0;
}


extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_AdjustProgram_initNative(JNIEnv *env, jclass clazz,
                                                       jboolean flip_vertical) {
    auto *adjustProgram = new AdjustProgram();
    adjustProgram->setFlip(false, flip_vertical);
    return reinterpret_cast<jlong>(adjustProgram);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_AdjustProgram_onDrawFrame(JNIEnv *env, jclass clazz,
                                                        jlong native_handle, jint texture_id) {
    if (native_handle == 0) {
        BZLogUtil::logE("AdjustProgram_onDrawFrame native_handle==0");
        return -1;
    }
    auto *adjustProgram = reinterpret_cast<AdjustProgram *>(native_handle);
    adjustProgram->setTextureId(texture_id);
    return adjustProgram->draw();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_AdjustProgram_setAdjustConfig(JNIEnv *env, jclass clazz,
                                                            jlong native_handle,
                                                            jobject adjust_effect_config) {
    if (native_handle == 0) {
        BZLogUtil::logE("AdjustProgram_setAdjustConfig native_handle==0");
        return -1;
    }
    auto *adjustProgram = reinterpret_cast<AdjustProgram *>(native_handle);
    AdjustEffectConfig *adjustEffectConfig = AdjustConfigUtil::getAdjustConfig(env,
                                                                               adjust_effect_config);
    adjustProgram->setAdjustConfig(adjustEffectConfig);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_AdjustProgram_setFlip(JNIEnv *env, jclass clazz, jlong native_handle,
                                                    jboolean need_flip_horizontal,
                                                    jboolean need_flip_vertical) {
    if (native_handle == 0) {
        BZLogUtil::logE("AdjustProgram_setFlip native_handle==0");
        return -1;
    }
    auto *adjustProgram = reinterpret_cast<AdjustProgram *>(native_handle);
    adjustProgram->setFlip(need_flip_horizontal, need_flip_vertical);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_AdjustProgram_release(JNIEnv *env, jclass clazz,
                                                    jlong native_handle) {
    if (native_handle == 0) {
        BZLogUtil::logE("AdjustProgram_release native_handle==0");
        return -1;
    }
    auto *adjustProgram = reinterpret_cast<AdjustProgram *>(native_handle);
    adjustProgram->releaseResource();
    delete adjustProgram;
    return 0;
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_CropTextureUtil_initCropTexture(JNIEnv *env, jclass type) {
    CropTextureUtil *cropTextureUtil = new CropTextureUtil();
    return reinterpret_cast<int64_t >(cropTextureUtil);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_CropTextureUtil_cropTexture(JNIEnv *env, jclass type, jlong handle,
                                                          jint srcTexture, jint srcWidth,
                                                          jint srcHeight,
                                                          jint startX, jint startY,
                                                          jint targetWidth,
                                                          jint targetHeight) {
    if (handle == 0) {
        return -1;
    }
    CropTextureUtil *cropTextureUtil = reinterpret_cast<CropTextureUtil *>(handle);
    return cropTextureUtil->cropTexture(srcTexture, srcWidth, srcHeight,
                                        startX, startY, targetWidth,
                                        targetHeight);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_CropTextureUtil_cropTextureRelease(JNIEnv *env, jclass type,
                                                                 jlong handle) {
    if (handle == 0) {
        return -1;
    }
    CropTextureUtil *cropTextureUtil = reinterpret_cast<CropTextureUtil *>(handle);
    int ret = cropTextureUtil->releaseCropTexture();
    delete (cropTextureUtil);
    return ret;
}


extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_YUV420Program_init(JNIEnv *env, jclass clazz) {
    auto *yuv420DrawProgram = new YUV420DrawProgram();
    yuv420DrawProgram->init();
    return reinterpret_cast<jlong>(yuv420DrawProgram);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_YUV420Program_setRotation(JNIEnv *env, jclass clazz,
                                                        jlong native_handle, jint rotation) {
    if (native_handle == 0) {
        return;
    }
    auto *yuv420DrawProgram = reinterpret_cast<YUV420DrawProgram *>(native_handle);
    yuv420DrawProgram->setRotation(rotation);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_YUV420Program_setFlip(JNIEnv *env, jclass clazz, jlong native_handle,
                                                    jboolean flip_horizontal,
                                                    jboolean flip_vertical) {
    if (native_handle == 0) {
        return;
    }
    auto *yuv420DrawProgram = reinterpret_cast<YUV420DrawProgram *>(native_handle);
    yuv420DrawProgram->setFlip(flip_horizontal, flip_vertical);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_YUV420Program_draw(JNIEnv *env, jclass clazz, jlong native_handle,
                                                 jbyteArray pixels, jint width, jint height) {
    if (native_handle == 0) {
        return -1;
    }
    auto *yuv420DrawProgram = reinterpret_cast<YUV420DrawProgram *>(native_handle);
    jbyte *data = env->GetByteArrayElements(pixels, nullptr);
    int ret = yuv420DrawProgram->draw(data, width, height);
    env->ReleaseByteArrayElements(pixels, data, 0);
    return ret;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_YUV420Program_release(JNIEnv *env, jclass clazz,
                                                    jlong native_handle) {
    if (native_handle == 0) {
        return;
    }
    auto *yuv420DrawProgram = reinterpret_cast<YUV420DrawProgram *>(native_handle);
    yuv420DrawProgram->releaseResource();
    delete (yuv420DrawProgram);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_initNative(JNIEnv *env, jclass clazz,
                                                       jboolean flip_vertical) {
    auto *mosaicProgram = new MosaicProgram();
    mosaicProgram->setFlip(false, flip_vertical);
    return reinterpret_cast<jlong>(mosaicProgram);
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_setTextureSize(JNIEnv *env, jclass clazz,
                                                           jlong native_handle, jint width,
                                                           jint height) {
    if (native_handle == 0) {
        return -1;
    }
    auto *mosaicProgram = reinterpret_cast<MosaicProgram *>(native_handle);
    mosaicProgram->setTextureSize(width, height);
    return 0;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_setMosaicSize(JNIEnv *env, jclass clazz,
                                                          jlong native_handle, jfloat width,
                                                          jfloat height) {
    if (native_handle == 0) {
        return -1;
    }
    auto *mosaicProgram = reinterpret_cast<MosaicProgram *>(native_handle);
    mosaicProgram->setMosaicSize(width, height);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_setCenterPoint(JNIEnv *env, jclass clazz,
                                                           jlong native_handle, jfloat x,
                                                           jfloat y) {
    if (native_handle == 0) {
        return -1;
    }
    auto *mosaicProgram = reinterpret_cast<MosaicProgram *>(native_handle);
    mosaicProgram->setCenterPoint(x, y);
    return 0;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_setRadius(JNIEnv *env, jclass clazz,
                                                      jlong native_handle, jfloat radius) {
    if (native_handle == 0) {
        return -1;
    }
    auto *mosaicProgram = reinterpret_cast<MosaicProgram *>(native_handle);
    mosaicProgram->setRadius(radius);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_onDrawFrame(JNIEnv *env, jclass clazz,
                                                        jlong native_handle, jint texture_id) {
    if (native_handle == 0) {
        return -1;
    }
    auto *mosaicProgram = reinterpret_cast<MosaicProgram *>(native_handle);
    mosaicProgram->setTextureId(texture_id);
    return mosaicProgram->draw();
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_release(JNIEnv *env, jclass clazz,
                                                    jlong native_handle) {
    if (native_handle == 0) {
        return -1;
    }
    auto *mosaicProgram = reinterpret_cast<MosaicProgram *>(native_handle);
    mosaicProgram->releaseResource();
    delete (mosaicProgram);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_setRectangleSize(JNIEnv *env, jclass clazz,
                                                             jlong native_handle, jint width,
                                                             jint height) {
    if (native_handle == 0) {
        return -1;
    }
    auto *mosaicProgram = reinterpret_cast<MosaicProgram *>(native_handle);
    mosaicProgram->setRectangleSize(width, height);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_MosaicProgram_setMosaicTypeImp(JNIEnv *env, jclass clazz,
                                                             jlong native_handle,
                                                             jint mosaic_type) {
    if (native_handle == 0) {
        return -1;
    }
    auto *mosaicProgram = reinterpret_cast<MosaicProgram *>(native_handle);
    mosaicProgram->setMosaicType(mosaic_type);
    return 0;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_ZoomProgram_initNative(JNIEnv *env, jclass clazz,
                                                     jboolean flip_vertical) {
    auto *zoomProgram = new ZoomProgram();
    zoomProgram->setFlip(false, flip_vertical);
    return reinterpret_cast<jlong>(zoomProgram);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_ZoomProgram_setCenterPoint(JNIEnv *env, jclass clazz,
                                                         jlong native_handle, jfloat x, jfloat y) {
    if (native_handle == 0) {
        return;
    }
    auto *zoomProgram = reinterpret_cast<ZoomProgram *>(native_handle);
    zoomProgram->setCenterPoint(x, y);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_ZoomProgram_setTextureSize(JNIEnv *env, jclass clazz,
                                                         jlong native_handle, jint width,
                                                         jint height) {
    if (native_handle == 0) {
        return;
    }
    auto *zoomProgram = reinterpret_cast<ZoomProgram *>(native_handle);
    zoomProgram->setTextureSize(width, height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_ZoomProgram_setRadius(JNIEnv *env, jclass clazz, jlong native_handle,
                                                    jfloat radius) {
    if (native_handle == 0) {
        return;
    }
    auto *zoomProgram = reinterpret_cast<ZoomProgram *>(native_handle);
    zoomProgram->setRadius(radius);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_opengl_ZoomProgram_setZoomRatio(JNIEnv *env, jclass clazz,
                                                       jlong native_handle, jfloat zoom_ratio) {
    if (native_handle == 0) {
        return;
    }
    auto *zoomProgram = reinterpret_cast<ZoomProgram *>(native_handle);
    zoomProgram->setZoomRatio(zoom_ratio);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_ZoomProgram_onDrawFrame(JNIEnv *env, jclass clazz,
                                                      jlong native_handle, jint texture_id) {
    if (native_handle == 0) {
        return -1;
    }
    auto *zoomProgram = reinterpret_cast<ZoomProgram *>(native_handle);
    zoomProgram->setTextureId(texture_id);
    return zoomProgram->draw();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_ZoomProgram_release(JNIEnv *env, jclass clazz, jlong native_handle) {
    if (native_handle == 0) {
        return -1;
    }
    auto *zoomProgram = reinterpret_cast<ZoomProgram *>(native_handle);
    zoomProgram->releaseResource();
    delete zoomProgram;
    return 0;
}

