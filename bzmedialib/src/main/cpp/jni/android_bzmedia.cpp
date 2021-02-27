#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <common/BZLogUtil.h>
#include <recorder/VideoRecordParams.h>
#include <recorder/VideoRecorder.h>
#include <mediaedit/AdjustVideoSpeedUtil.h>
#include <mediaedit/BackgroundMusicUtil.h>
#include <player/PCMPlayerNative.h>
#include <common/JMethodInfo.h>
#include <mediaedit/ClipVideoFrameToImage.h>
#include <common/JvmManager.h>
#include <mediaedit/VideoUtil.h>
#include "ffmpeg_base_info.h"
#include "OnActionListener.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
const char *TAG = "bz_";
jclass bzMediaClass = NULL;

void log_call_back(void *ptr, int level, const char *fmt, va_list vl) {
    //Custom log
    if (level == 3) {
        __android_log_vprint(ANDROID_LOG_ERROR, TAG, fmt, vl);
    } else if (level == 2) {
        __android_log_vprint(ANDROID_LOG_DEBUG, TAG, fmt, vl);
    } else if (level == 1) {
        __android_log_vprint(ANDROID_LOG_VERBOSE, TAG, fmt, vl);
    } else {
        if (level <= 16) {//logs from ffmpeg
            __android_log_vprint(ANDROID_LOG_ERROR, TAG, fmt, vl);
        } else {
//            __android_log_vprint(ANDROID_LOG_VERBOSE, TAG, fmt, vl);
        }
    }
}

void getImageFromVideoCallBack(int64_t callBackHandle, int index, const char *imagePath) {
    if (callBackHandle == 0) {
        BZLogUtil::logW("callBackHandle==0");
        return;
    }
    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (NULL == bzMediaClass || NULL == jniEnv) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    JMethodInfo *jMethodInfo = reinterpret_cast<JMethodInfo *>(callBackHandle);
    jstring path = jniEnv->NewStringUTF(imagePath);
    jniEnv->CallVoidMethod(jMethodInfo->obj, jMethodInfo->methodID, index,
                           path);
    jniEnv->DeleteLocalRef(path);
    jniEnv = NULL;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void
getBitmapFromVideoCallBack(int64_t callBackHandle, int index, void *data, int width, int height) {
    if (callBackHandle == 0) {
        BZLogUtil::logW("getBitmapFromVideoCallBack callBackHandle==0");
        return;
    }
    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    int ret;
    void *targetPixels;
    jclass bitmapCls = jniEnv->FindClass("android/graphics/Bitmap");

    jobject newBitmap;
    jmethodID createBitmapFunctionMethodID = jniEnv->GetStaticMethodID(bitmapCls, "createBitmap",
                                                                       "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jstring configName = jniEnv->NewStringUTF("ARGB_8888");
    jclass bitmapConfigClass = jniEnv->FindClass("android/graphics/Bitmap$Config");

    jmethodID valueOfBitmapConfigFunctionMethodID = jniEnv->GetStaticMethodID(bitmapConfigClass,
                                                                              "valueOf",
                                                                              "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jobject bitmapConfigObj = jniEnv->CallStaticObjectMethod(bitmapConfigClass,
                                                             valueOfBitmapConfigFunctionMethodID,
                                                             configName);

    newBitmap = jniEnv->CallStaticObjectMethod(bitmapCls, createBitmapFunctionMethodID, width,
                                               height, bitmapConfigObj);

    if ((ret = AndroidBitmap_lockPixels(jniEnv, newBitmap, &targetPixels)) < 0) {
        BZLogUtil::logE(
                "getBitmapFromVideoCallBack AndroidBitmap_lockPixels() targetPixels failed ! error=%d",
                ret);
    }
    if (ret >= 0) {
        memcpy(targetPixels, data, (size_t) (width * height * 4));
        JMethodInfo *jMethodInfo = reinterpret_cast<JMethodInfo *>(callBackHandle);
        jniEnv->CallVoidMethod(jMethodInfo->obj, jMethodInfo->methodID, index, newBitmap);
    }
    //必须要释放
    AndroidBitmap_unlockPixels(jniEnv, newBitmap);


    jniEnv->DeleteLocalRef(configName);
    jniEnv->DeleteLocalRef(bitmapConfigObj);
    jniEnv->DeleteLocalRef(bitmapConfigClass);
    jniEnv->DeleteLocalRef(newBitmap);

    jniEnv = NULL;
    if (needDetach)JvmManager::getJavaVM()->DetachCurrentThread();

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_initNative(JNIEnv *env, jclass clazz, jobject context,
                                          jboolean is_debug, jint sdk_int) {
    BZLogUtil::enableLog = is_debug;
    __android_log_write(ANDROID_LOG_DEBUG, BZLOG_TAG, "welcome bzmedia ^_^");
    if (is_debug) {
        av_log_set_callback(log_call_back);
    }
    if (NULL == bzMediaClass) {
        bzMediaClass = (jclass) env->NewGlobalRef(clazz);
    }
    TextureConvertYUVUtil::useHDShader = sdk_int >= 19;
    jclass pcmPlayerClass = env->FindClass("com/luoye/bzmedia/player/PCMPlayer");
    PCMPlayerNative::pcmPlayerClass = (jclass) env->NewGlobalRef(pcmPlayerClass);
    env->DeleteLocalRef(pcmPlayerClass);
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_test(JNIEnv *env, jclass clazz) {
    printFFmpegBaseInfo();
    return 0;
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_startRecord(JNIEnv *env, jclass clazz,
                                           jobject videoRecordParamsObj) {
    VideoRecordParams videoRecordParams;

    jclass videoRecordParamsClass = env->GetObjectClass(videoRecordParamsObj);
    jstring output_path_ = (jstring) env->GetObjectField(videoRecordParamsObj,
                                                         env->GetFieldID(videoRecordParamsClass,
                                                                         "outputPath",
                                                                         "Ljava/lang/String;"));
    const char *output_path = NULL;
    if (NULL != output_path_) {
        output_path = env->GetStringUTFChars(output_path_, 0);
        videoRecordParams.output_path = output_path;
        BZLogUtil::logD("output_path=%s", output_path);
    } else {
        BZLogUtil::logE("output_path is NULL");
        return -1;
    }
    jint srcWidth = env->GetIntField(videoRecordParamsObj,
                                     env->GetFieldID(videoRecordParamsClass, "inputWidth", "I"));
    videoRecordParams.inputWidth = srcWidth;
    BZLogUtil::logD("inputWidth=%d", srcWidth);

    jint srcHeight = env->GetIntField(videoRecordParamsObj,
                                      env->GetFieldID(videoRecordParamsClass, "inputHeight", "I"));
    videoRecordParams.inputHeight = srcHeight;
    BZLogUtil::logD("inputHeight=%d", srcHeight);

    jint targetWidth = env->GetIntField(videoRecordParamsObj,
                                        env->GetFieldID(videoRecordParamsClass, "targetWidth",
                                                        "I"));
    videoRecordParams.targetWidth = targetWidth;
    BZLogUtil::logD("targetWidth=%d", targetWidth);

    jint targetHeight = env->GetIntField(videoRecordParamsObj,
                                         env->GetFieldID(videoRecordParamsClass, "targetHeight",
                                                         "I"));
    videoRecordParams.targetHeight = targetHeight;
    BZLogUtil::logD("targetHeight=%d", targetHeight);

    jint videoRate = env->GetIntField(videoRecordParamsObj,
                                      env->GetFieldID(videoRecordParamsClass, "videoFrameRate",
                                                      "I"));
    videoRecordParams.videoFrameRate = videoRate;
    BZLogUtil::logD("videoFrameRate=%d", videoRate);
    jint nbSamples = env->GetIntField(videoRecordParamsObj,
                                      env->GetFieldID(videoRecordParamsClass, "nbSamples", "I"));
    videoRecordParams.nbSamples = nbSamples;
    BZLogUtil::logD("nbSamples=%d", nbSamples);

    jint sampleRate = env->GetIntField(videoRecordParamsObj,
                                       env->GetFieldID(videoRecordParamsClass, "sampleRate", "I"));
    videoRecordParams.sampleRate = sampleRate;
    BZLogUtil::logD("sampleRate=%d", sampleRate);

    jint videoRotate = env->GetIntField(videoRecordParamsObj,
                                        env->GetFieldID(videoRecordParamsClass, "videoRotate",
                                                        "I"));
    videoRecordParams.videoRotate = videoRotate;
    BZLogUtil::logD("videoRotate=%d", videoRotate);

    jint pixelFormat = env->GetIntField(videoRecordParamsObj,
                                        env->GetFieldID(videoRecordParamsClass, "pixelFormat",
                                                        "I"));
    videoRecordParams.pixelFormat = pixelFormat;
    BZLogUtil::logD("pixelFormat=%d", pixelFormat);

    jboolean hasAudio = env->GetBooleanField(videoRecordParamsObj,
                                             env->GetFieldID(videoRecordParamsClass, "hasAudio",
                                                             "Z"));
    videoRecordParams.hasAudio = hasAudio;
    BZLogUtil::logD("hasAudio=%d", hasAudio);

    jboolean synEncode = env->GetBooleanField(videoRecordParamsObj,
                                              env->GetFieldID(videoRecordParamsClass, "synEncode",
                                                              "Z"));
    videoRecordParams.synEncode = synEncode;
    BZLogUtil::logD("synEncode=%d", synEncode);


    jboolean avPacketFromMediaCodec = env->GetBooleanField(videoRecordParamsObj,
                                                           env->GetFieldID(videoRecordParamsClass,
                                                                           "avPacketFromMediaCodec",
                                                                           "Z"));
    videoRecordParams.avPacketFromMediaCodec = avPacketFromMediaCodec;
    BZLogUtil::logD("avPacketFromMediaCodec=%d", avPacketFromMediaCodec);


    jboolean needFlipVertical = env->GetBooleanField(videoRecordParamsObj,
                                                     env->GetFieldID(videoRecordParamsClass,
                                                                     "needFlipVertical",
                                                                     "Z"));
    videoRecordParams.needFlipVertical = needFlipVertical;
    BZLogUtil::logD("needFlipVertical=%d", needFlipVertical);


    jboolean allFrameIsKey = env->GetBooleanField(videoRecordParamsObj,
                                                  env->GetFieldID(videoRecordParamsClass,
                                                                  "allFrameIsKey",
                                                                  "Z"));
    videoRecordParams.allFrameIsKey = allFrameIsKey;
    BZLogUtil::logD("allFrameIsKey=%d", needFlipVertical);


    jlong bitRate = env->GetLongField(videoRecordParamsObj,
                                      env->GetFieldID(videoRecordParamsClass, "bitRate",
                                                      "J"));
    videoRecordParams.bit_rate = bitRate;
    BZLogUtil::logD("bitRate=%lld", bitRate);

    VideoRecorder *videoRecorder = new VideoRecorder();
    int64_t ret = videoRecorder->startRecord(videoRecordParams);
    if (ret < 0) {
        BZLogUtil::logE("startRecord fail");
        videoRecorder->setStopRecordFlag();
        videoRecorder->stopRecord();
        delete (videoRecorder);
        return 0;
    }
    if (NULL != output_path_ && NULL != output_path)
        env->ReleaseStringUTFChars(output_path_, output_path);

    env->DeleteLocalRef(videoRecordParamsObj);
    env->DeleteLocalRef(videoRecordParamsClass);

    return reinterpret_cast<int64_t>(videoRecorder);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_addAudioData(JNIEnv *env, jclass clazz, jlong native_handle,
                                            jbyteArray data_, jint length) {
    if (native_handle == 0) {
        return -1;
    }
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(native_handle);
    long ret = videoRecorder->addAudioData(reinterpret_cast<unsigned char *>(data), length);
    env->ReleaseByteArrayElements(data_, data, 0);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_setStopRecordFlag(JNIEnv *env, jclass clazz, jlong native_handle) {
    if (native_handle == 0) {
        return -1;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(native_handle);
    videoRecorder->setStopRecordFlag();
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_stopRecord(JNIEnv *env, jclass clazz, jlong nativeHandle) {
    if (nativeHandle == 0) {
        return -1;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    int ret = videoRecorder->stopRecord();
    delete (videoRecorder);
    return ret;
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_updateVideoRecorderTexture(JNIEnv *env, jclass clazz,
                                                          jlong nativeHandle, jint textureId) {
    if (nativeHandle == 0) {
        return 0;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    return videoRecorder->updateTexture(textureId);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_addYUV420Data(JNIEnv *env, jclass clazz, jlong native_handle,
                                             jbyteArray data_, jlong pts) {
    if (native_handle == 0) {
        return -1;
    }
    unsigned char *buffer = (unsigned char *) env->GetByteArrayElements(data_, NULL);
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(native_handle);
    long ret = videoRecorder->addVideoData(buffer, pts);
    env->ReleaseByteArrayElements(data_, reinterpret_cast<jbyte *>(buffer), 0);
    return ret;
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_addVideoPacketData(JNIEnv *env, jclass clazz, jlong native_handle,
                                                  jbyteArray data_, jlong size, jlong pts) {
    if (native_handle == 0) {
        return -1;
    }
    unsigned char *buffer = (unsigned char *) env->GetByteArrayElements(data_, NULL);
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(native_handle);
    long ret = videoRecorder->addVideoPacketData(buffer, size, pts);
    env->ReleaseByteArrayElements(data_, reinterpret_cast<jbyte *>(buffer), 0);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_adjustVideoSpeed(JNIEnv *env, jclass type, jstring srcVideoPath_,
                                                jstring outputPath_, jfloat speed) {
    const char *srcVideoPath = env->GetStringUTFChars(srcVideoPath_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    AdjustVideoSpeedUtil adjustVideoSpeedUtil;
    int ret = adjustVideoSpeedUtil.adjustVideoSpeed(srcVideoPath, outputPath, speed);

    env->ReleaseStringUTFChars(srcVideoPath_, srcVideoPath);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    return ret;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_addBackgroundMusic(
        JNIEnv *env, jclass type,
        jstring inputPath_,
        jstring outputPath_, jstring musicPath_,
        jfloat srcMusicVolume,
        jfloat bgMusicVolume, jobject onActionListenerObj) {

    if (NULL == inputPath_ || NULL == outputPath_ || NULL == musicPath_) {
        BZLogUtil::logE("addBackgroundMusic param is error");
        return -1;
    }
    const char *inputPath = env->GetStringUTFChars(inputPath_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);
    const char *musicPath = env->GetStringUTFChars(musicPath_, 0);

    OnActionListener *onActionListener = new OnActionListener(onActionListenerObj);

    BackgroundMusicUtil addBackgroundMusic;
    int ret = addBackgroundMusic.startAddBackgroundMusic(inputPath, outputPath, musicPath,
                                                         srcMusicVolume, bgMusicVolume,
                                                         onActionListener);
    if (ret < 0) {
        onActionListener->fail();
    } else {
        onActionListener->success();
    }
    delete (onActionListener);
    env->ReleaseStringUTFChars(inputPath_, inputPath);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    env->ReleaseStringUTFChars(musicPath_, musicPath);
    return ret;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_replaceBackgroundMusic(
        JNIEnv *env, jclass type, jstring videoPath_, jstring musicPath_, jstring outputPath_,
        jobject onActionListenerObj) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);
    const char *musicPath = env->GetStringUTFChars(musicPath_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    OnActionListener *onActionListener = new OnActionListener(onActionListenerObj);
    BackgroundMusicUtil addBackgroundMusic;
    int ret = addBackgroundMusic.replaceBackgroundMusic(videoPath,
                                                        musicPath,
                                                        outputPath,
                                                        onActionListener);

    if (ret < 0) {
        onActionListener->fail();
    } else {
        onActionListener->success();
    }
    delete (onActionListener);

    env->ReleaseStringUTFChars(videoPath_, videoPath);
    env->ReleaseStringUTFChars(musicPath_, musicPath);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_getImageFromVideo(JNIEnv *env, jclass clazz, jstring videoPath_,
                                                 jstring outImageParentPath_, jint imageCount,
                                                 jint scale2Width,
                                                 jobject onGetImageFromVideoListener) {
    if (NULL == videoPath_ || NULL == outImageParentPath_ || imageCount <= 0) {
        BZLogUtil::logE("getImageFromVideo param is error");
        return -1;
    }
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);
    const char *outImageParentPath = env->GetStringUTFChars(outImageParentPath_, 0);

    JMethodInfo *jMethodInfo = new JMethodInfo();
    jMethodInfo->obj = env->NewGlobalRef(onGetImageFromVideoListener);
    jclass classListener = env->GetObjectClass(onGetImageFromVideoListener);
    jMethodInfo->methodID = env->GetMethodID(classListener, "onGetImageFromVideo",
                                             "(ILjava/lang/String;)V");

    ClipVideoFrameToImage clipVideoFrameToImage;
    int ret = clipVideoFrameToImage.clipVideoFrameToImage(videoPath, outImageParentPath, imageCount,
                                                          scale2Width,
                                                          reinterpret_cast<int64_t>(jMethodInfo),
                                                          getImageFromVideoCallBack);
    env->ReleaseStringUTFChars(videoPath_, videoPath);
    env->ReleaseStringUTFChars(outImageParentPath_, outImageParentPath);

    env->DeleteGlobalRef(jMethodInfo->obj);
    env->DeleteLocalRef(classListener);
    delete (jMethodInfo);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_getBitmapFromVideo(JNIEnv *env, jclass clazz, jstring videoPath_,
                                                  jint image_count, jint scale2_width,
                                                  jobject onGetBitmapFromVideoListener) {
    if (NULL == videoPath_) {
        BZLogUtil::logE("getBitmapFromVideo NULL==videoPath_");
        return -1;
    }
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);

    JMethodInfo *jMethodInfo = new JMethodInfo();
    jMethodInfo->obj = env->NewGlobalRef(onGetBitmapFromVideoListener);
    jclass classListener = env->GetObjectClass(onGetBitmapFromVideoListener);
    jMethodInfo->methodID = env->GetMethodID(classListener,
                                             "onGetBitmapFromVideo",
                                             "(ILandroid/graphics/Bitmap;)V");

    ClipVideoFrameToImage clipVideoFrameToImage;
    int ret = clipVideoFrameToImage.clipVideoFrameToImage(videoPath,
                                                          image_count,
                                                          scale2_width,
                                                          reinterpret_cast<int64_t>(jMethodInfo),
                                                          getBitmapFromVideoCallBack);
    env->ReleaseStringUTFChars(videoPath_, videoPath);
    env->DeleteLocalRef(classListener);
    env->DeleteGlobalRef(jMethodInfo->obj);
    delete jMethodInfo;
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_clipAudio(JNIEnv *env, jclass clazz, jstring audioPath_,
                                         jstring out_path, jlong start_time, jlong end_time) {
    const char *audioPath = env->GetStringUTFChars(audioPath_, 0);
    const char *outPath = env->GetStringUTFChars(out_path, 0);

    int ret = VideoUtil::clipAudio(audioPath, outPath, start_time, end_time);

    env->ReleaseStringUTFChars(audioPath_, audioPath);
    env->ReleaseStringUTFChars(out_path, outPath);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_clipVideo(JNIEnv *env, jclass clazz, jstring videoPath_,
                                         jstring outPath_, jlong start_time, jlong end_time) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);
    const char *outPath = env->GetStringUTFChars(outPath_, 0);

    int ret = VideoUtil::clipVideo(videoPath, outPath, start_time, end_time);

    env->ReleaseStringUTFChars(videoPath_, videoPath);
    env->ReleaseStringUTFChars(outPath_, outPath);
    return ret;
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_getMediaDuration(JNIEnv *env, jclass clazz, jstring media_path) {
    const char *mediaPath = env->GetStringUTFChars(media_path, 0);

    jlong temp = VideoUtil::getMediaDuration(mediaPath);;

    env->ReleaseStringUTFChars(media_path, mediaPath);

    return temp;
}