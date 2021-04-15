//
/**
 * Created by bookzhan on 2019-02-21 15:49.
 * 说明:
 */
//

#include <jni.h>
#include <mediaedit/VideoFrameGetter.h>
#include <android/bitmap.h>
#include <common/BZLogUtil.h>

extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_init(JNIEnv *env, jclass type,
                                                        jstring videoPath_,
                                                        jboolean userSoftDecode) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);

    auto *videoFrameGetter = new VideoFrameGetter();
    int ret = videoFrameGetter->init(videoPath, userSoftDecode);

    env->ReleaseStringUTFChars(videoPath_, videoPath);
    if (ret < 0) {
        delete videoFrameGetter;
        return 0;
    } else {
        return reinterpret_cast<jlong>(videoFrameGetter);
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_getVideoWidth(JNIEnv *env, jclass type,
                                                                 jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *videoFrameGetter = reinterpret_cast<VideoFrameGetter *>(nativeHandle);

    return videoFrameGetter->getVideoWidth();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_getVideoHeight(JNIEnv *env, jclass type,
                                                                  jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *videoFrameGetter = reinterpret_cast<VideoFrameGetter *>(nativeHandle);

    return videoFrameGetter->getVideoHeight();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_getVideoFrame(JNIEnv *env, jclass type,
                                                                 jlong nativeHandle,
                                                                 jlong currentTime) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *videoFrameGetter = reinterpret_cast<VideoFrameGetter *>(nativeHandle);
    return videoFrameGetter->getVideoFrame(currentTime);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_getVideoFrame4Bitmap(JNIEnv *jniEnv,
                                                                        jclass type,
                                                                        jlong nativeHandle,
                                                                        jlong currentTime) {

    if (nativeHandle == 0) {
        return nullptr;
    }
    auto *videoFrameGetter = reinterpret_cast<VideoFrameGetter *>(nativeHandle);
    AVFrame *avFrame = videoFrameGetter->getVideoFrameData(currentTime);
    if (nullptr == avFrame || nullptr == avFrame->data[0]) {
        return nullptr;
    }

    int64_t objectHandle = videoFrameGetter->getObjectHandle();
    int ret;
    jobject newBitmapGlobalRef;
    void *targetPixels;
    int width = avFrame->width;
    int height = avFrame->height;
    if (objectHandle == 0) {
        jclass bitmapCls = jniEnv->FindClass("android/graphics/Bitmap");
        jmethodID createBitmapFunctionMethodID = jniEnv->GetStaticMethodID(bitmapCls,
                                                                           "createBitmap",
                                                                           "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jstring configName = jniEnv->NewStringUTF("ARGB_8888");
        jclass bitmapConfigClass = jniEnv->FindClass("android/graphics/Bitmap$Config");

        jmethodID valueOfBitmapConfigFunctionMethodID = jniEnv->GetStaticMethodID(bitmapConfigClass,
                                                                                  "valueOf",
                                                                                  "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
        jobject bitmapConfigObj = jniEnv->CallStaticObjectMethod(bitmapConfigClass,
                                                                 valueOfBitmapConfigFunctionMethodID,
                                                                 configName);

        jobject newBitmap = jniEnv->CallStaticObjectMethod(bitmapCls, createBitmapFunctionMethodID,
                                                           width,
                                                           height, bitmapConfigObj);
        newBitmapGlobalRef = jniEnv->NewGlobalRef(newBitmap);
        videoFrameGetter->setObjectHandle(reinterpret_cast<int64_t>(newBitmapGlobalRef));
        jniEnv->DeleteLocalRef(bitmapCls);
        jniEnv->DeleteLocalRef(configName);
        jniEnv->DeleteLocalRef(bitmapConfigObj);
        jniEnv->DeleteLocalRef(bitmapConfigClass);
    } else {
        newBitmapGlobalRef = reinterpret_cast<jobject>(objectHandle);
    }
    if ((ret = AndroidBitmap_lockPixels(jniEnv, newBitmapGlobalRef, &targetPixels)) < 0) {
        BZLogUtil::logE("gifDataCallBack AndroidBitmap_lockPixels() targetPixels failed ! error=%d",
                        ret);
    }
    if (ret >= 0) {
        memcpy(targetPixels, avFrame->data[0], (size_t) (width * height * 4));
        //必须要释放
        AndroidBitmap_unlockPixels(jniEnv, newBitmapGlobalRef);
    }
    return newBitmapGlobalRef;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_release(JNIEnv *env, jclass type,
                                                           jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    auto *videoFrameGetter = reinterpret_cast<VideoFrameGetter *>(nativeHandle);
    int64_t objectHandle = videoFrameGetter->getObjectHandle();
    if (objectHandle != 0) {
        jobject newBitmapGlobalRef = reinterpret_cast<jobject>(objectHandle);
        env->DeleteGlobalRef(newBitmapGlobalRef);
    }
    videoFrameGetter->release();
    delete (videoFrameGetter);
    return 0;
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_getVideoDuration(JNIEnv *env, jclass clazz,
                                                                    jlong nativeHandle) {
    if (nativeHandle == 0) {
        return -1;
    }
    auto *videoFrameGetter = reinterpret_cast<VideoFrameGetter *>(nativeHandle);
    return videoFrameGetter->getVideoDuration();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_setStartTime(JNIEnv *env, jclass clazz,
                                                                jlong nativeHandle,
                                                                jlong start_time) {
    if (nativeHandle == 0) {
        return -1;
    }
    auto *videoFrameGetter = reinterpret_cast<VideoFrameGetter *>(nativeHandle);
    return videoFrameGetter->setStartTime(start_time);
}extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_opengl_VideoFrameGetterUtil_setEnableLoop(JNIEnv *env, jclass clazz,
                                                                 jlong nativeHandle,
                                                                 jboolean enable_loop) {
    if (nativeHandle == 0) {
        return -1;
    }
    auto *videoFrameGetter = reinterpret_cast<VideoFrameGetter *>(nativeHandle);
    return videoFrameGetter->setEnableLoop(enable_loop);
}