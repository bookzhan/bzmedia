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
#include <common/JvmManager.h>
#include <mediaedit/VideoUtil.h>
#include <mediaedit/MergeVideoAndAudio.h>
#include <mediaedit/MergeVideoUtil.h>
#include <glutils/EGLContextUtil.h>
#include <glprogram/AdjustProgram.h>
#include <utils/AdjustConfigUtil.h>
#include <permission/PermissionUtil.h>
#include <glutils/CropTextureUtil.h>
#include <mediaedit/GetVideoFrameUtil.h>
#include "ffmpeg_base_info.h"
#include "OnActionListener.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
const char *TAG = "bz_";
jclass bzMediaClass = nullptr;

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
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == bzMediaClass || nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    JMethodInfo *jMethodInfo = reinterpret_cast<JMethodInfo *>(callBackHandle);
    jstring path = jniEnv->NewStringUTF(imagePath);
    jniEnv->CallVoidMethod(jMethodInfo->obj, jMethodInfo->methodID, index,
                           path);
    jniEnv->DeleteLocalRef(path);
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void
getBitmapFromVideoCallBack(int64_t callBackHandle, int index, void *data, int width, int height) {
    if (callBackHandle == 0) {
        BZLogUtil::logW("getBitmapFromVideoCallBack callBackHandle==0");
        return;
    }
    JNIEnv *jniEnv = nullptr;
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

    jniEnv = nullptr;
    if (needDetach)JvmManager::getJavaVM()->DetachCurrentThread();

}

void sendMediaInfo(long handle, int what, int extra) {
//    BZLogUtil::logD("sendMediaInfo--what=%d--extra=%d", what, extra);
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    JMethodInfo *jMethodInfo = reinterpret_cast<JMethodInfo *>(handle);
    jniEnv->CallVoidMethod(jMethodInfo->obj, jMethodInfo->methodID, what, extra);
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
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
    //权限检测
    if (!PermissionUtil::checkPermission(env, context)) {
        return -1;
    }
    if (nullptr == bzMediaClass) {
        bzMediaClass = (jclass) env->NewGlobalRef(clazz);
    }
    TextureConvertYUVUtil::useHDShader = sdk_int >= 19;
    jclass pcmPlayerClass = env->FindClass("com/luoye/bzmedia/player/PCMPlayer");
    PCMPlayerNative::pcmPlayerClass = (jclass) env->NewGlobalRef(pcmPlayerClass);
    env->DeleteLocalRef(pcmPlayerClass);

    av_register_all();
    avcodec_register_all();
    avfilter_register_all();
    avformat_network_init();
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_test(JNIEnv *env, jclass clazz) {
    printFFmpegBaseInfo();
    char *buffer = static_cast<char *>(malloc(2048));
//    PermissionUtil::encode_str("com.nbb.bestvideoeditor", &buffer);
    PermissionUtil::encode_str("com.nbb.effect.sample", &buffer);
    BZLogUtil::logD("encode_str=%s", buffer);
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
    const char *output_path = nullptr;
    if (nullptr != output_path_) {
        output_path = env->GetStringUTFChars(output_path_, 0);
        videoRecordParams.output_path = output_path;
        BZLogUtil::logD("output_path=%s", output_path);
    } else {
        BZLogUtil::logE("output_path is nullptr");
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
                                             env->GetFieldID(videoRecordParamsClass, "needAudio",
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
    if (nullptr != output_path_ && nullptr != output_path)
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
    jbyte *data = env->GetByteArrayElements(data_, nullptr);
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
                                                          jlong nativeHandle, jint textureId,
                                                          jlong pts) {
    if (nativeHandle == 0) {
        return 0;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    return videoRecorder->updateTexture(textureId, pts);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_addYUV420Data(JNIEnv *env, jclass clazz, jlong native_handle,
                                             jbyteArray data_, jlong pts) {
    if (native_handle == 0) {
        return -1;
    }
    unsigned char *buffer = (unsigned char *) env->GetByteArrayElements(data_, nullptr);
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
    unsigned char *buffer = (unsigned char *) env->GetByteArrayElements(data_, nullptr);
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(native_handle);
    long ret = videoRecorder->addVideoPacketData(buffer, size, pts);
    env->ReleaseByteArrayElements(data_, reinterpret_cast<jbyte *>(buffer), 0);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_adjustAudioSpeed(JNIEnv *env, jclass clazz, jstring audio_path,
                                                jstring outputPath_, jfloat speed,
                                                jobject onActionListenerObj) {
    if (nullptr == audio_path || nullptr == outputPath_) {
        BZLogUtil::logE("adjustAudioSpeed nullptr==audio_path|| nullptr==outputPath_");
        return -1;
    }
    const char *srcAudioPath = env->GetStringUTFChars(audio_path, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    if (speed > 100) {
        speed = 100;
    }
    if (speed < 0.1) {
        speed = 0.1;
    }
    char cmd[1024] = {0};
    sprintf(cmd,
            "ffmpeg -y -i \"%s\" -vn -af atempo=%.3f \"%s\"", srcAudioPath, speed, outputPath);
    BZLogUtil::logD("cmd=%s", cmd);
    OnActionListener *onActionListener = new OnActionListener(onActionListenerObj);

    int64_t duration = VideoUtil::getMediaDuration(srcAudioPath);
    duration = (long) (1.0f / speed * duration);
    int ret = executeFFmpegCommand4TotalTime(
            reinterpret_cast<int64_t>(onActionListener), cmd, OnActionListener::progressCallBack,
            duration);

    if (ret >= 0) {
        onActionListener->success();
    } else {
        onActionListener->fail();
    }
    delete onActionListener;
    env->ReleaseStringUTFChars(audio_path, srcAudioPath);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
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
Java_com_luoye_bzmedia_BZMedia_replaceBackgroundMusicOnly(JNIEnv *env, jclass clazz,
                                                          jstring videoPath_, jstring musicPath_,
                                                          jstring outputPath_,
                                                          jobject on_action_listener) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);
    const char *musicPath = env->GetStringUTFChars(musicPath_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    auto *onActionListener = new OnActionListener(on_action_listener);
    BackgroundMusicUtil addBackgroundMusic;
    int ret = addBackgroundMusic.replaceBackgroundMusicOnly(videoPath,
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
Java_com_luoye_bzmedia_BZMedia_adjustVideoSpeed(JNIEnv *env, jclass type, jstring srcVideoPath_,
                                                jstring outputPath_, jfloat speed) {
    if (nullptr == srcVideoPath_ || nullptr == outputPath_) {
        BZLogUtil::logE("adjustVideoSpeed nullptr==audio_path|| nullptr==outputPath_");
        return -1;
    }
    const char *srcVideoPath = env->GetStringUTFChars(srcVideoPath_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);
    BZLogUtil::logD("adjustVideoSpeed srcVideoPath=%s,outputPath=%s,speed=%f", srcVideoPath,
                    outputPath, speed);

    string outputParentDir;
    outputParentDir.append(outputPath);
    outputParentDir = outputParentDir.substr(0, outputParentDir.find_last_of("/"));


    string videoStreamOutPath;
    char videoStreamName[128] = {0};
    sprintf(videoStreamName, "/videoStream_%lld", getMicrosecondTime());
    videoStreamOutPath.append(outputParentDir);
    videoStreamOutPath.append(videoStreamName);
    videoStreamOutPath.append(".mp4");
    BZLogUtil::logD("start adjustVideoSpeed");

    const char *videoStreamPath = videoStreamOutPath.c_str();
    AdjustVideoSpeedUtil adjustVideoSpeedUtil;
    int ret = adjustVideoSpeedUtil.adjustVideoSpeed(srcVideoPath, videoStreamPath,
                                                    speed);
    bool hasAudio = VideoUtil::hasAudio(srcVideoPath);
    if (ret >= 0 && !hasAudio) {
        BZLogUtil::logD("!hasAudio rename file");
        ret = rename(videoStreamPath, outputPath);
    } else if (ret >= 0 && hasAudio) {
        string audioStreamOutPath;
        char audioStreamName[128] = {0};
        sprintf(audioStreamName, "/audioStream_%lld", getMicrosecondTime());
        audioStreamOutPath.append(outputParentDir);
        audioStreamOutPath.append(audioStreamName);
        audioStreamOutPath.append(".m4a");

        const char *audioStreamPath = audioStreamOutPath.c_str();
        jstring audioPathStr = env->NewStringUTF(audioStreamPath);
        BZLogUtil::logD("start adjustAudioSpeed");
        ret = Java_com_luoye_bzmedia_BZMedia_adjustAudioSpeed(env, type, srcVideoPath_,
                                                              audioPathStr, speed,
                                                              nullptr);

        if (ret >= 0) {
            BZLogUtil::logD("start replaceBackgroundMusicOnly");
            jstring videoStreamPathStr = env->NewStringUTF(videoStreamPath);
            ret = Java_com_luoye_bzmedia_BZMedia_replaceBackgroundMusicOnly(env, type,
                                                                            videoStreamPathStr,
                                                                            audioPathStr,
                                                                            outputPath_,
                                                                            nullptr);
            if (ret < 0) {
                BZLogUtil::logE("Java_com_luoye_bzmedia_BZMedia_replaceBackgroundMusic fail");
            }
            env->DeleteLocalRef(videoStreamPathStr);
        } else {
            BZLogUtil::logE("Java_com_luoye_bzmedia_BZMedia_adjustAudioSpeed fail");
        }
        env->DeleteLocalRef(audioPathStr);
        remove(audioStreamPath);
    } else {
        BZLogUtil::logE("adjustVideoSpeedUtil.adjustVideoSpeed fail");
    }
    remove(videoStreamPath);
    env->ReleaseStringUTFChars(srcVideoPath_, srcVideoPath);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_preHandleAudio(JNIEnv *env, jclass clazz,
                                              jstring audio_path, jstring outputPath_,
                                              jfloat speed, jfloat volume, jlong startDelayTime,
                                              jfloat fadeInTime, jfloat fadeOutTime,
                                              jlong durationTime,
                                              jobject onActionListenerObj) {
    if (nullptr == audio_path || nullptr == outputPath_) {
        BZLogUtil::logE("preHandleAudio nullptr==audio_path|| nullptr==outputPath_");
        return -1;
    }
    const char *srcAudioPath = env->GetStringUTFChars(audio_path, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    int64_t audioDuration = VideoUtil::getMediaDuration(srcAudioPath);
    if (audioDuration < 0) {
        BZLogUtil::logE("preHandleAudio audioDuration<0");
        return -1;
    }
    if (speed > 100) {
        speed = 100;
    }
    if (speed <= 0) {
        speed = 1;
    }
    if (speed < 0.1) {
        speed = 0.1;
    }
    if (volume < 0) {
        volume = 0;
    }
    if (durationTime <= 0) {
        durationTime = audioDuration / speed;
    }
    char cmd[1024] = {0};
    sprintf(cmd,
            "ffmpeg -y -i \"%s\" -t %.3f -af atempo=%.3f,volume=%.3f,afade=t=in:d=%.3f,afade=t=out:st=%.3f:d=%.3f,adelay=%lld|%lld|%lld|%lld -vn -b:a 128000 \"%s\"",
            srcAudioPath, durationTime / 1000.0f, speed, volume,
            fadeInTime / 1000, (durationTime - fadeOutTime) / 1000,
            fadeOutTime / 1000,
            startDelayTime, startDelayTime,
            startDelayTime, startDelayTime,
            outputPath);
    BZLogUtil::logD("cmd=%s", cmd);
    OnActionListener *onActionListener = new OnActionListener(onActionListenerObj);

    int ret = executeFFmpegCommand4TotalTime(
            reinterpret_cast<int64_t>(onActionListener), cmd, OnActionListener::progressCallBack,
            durationTime);

    if (ret >= 0) {
        onActionListener->success();
    } else {
        onActionListener->fail();
    }
    delete onActionListener;
    env->ReleaseStringUTFChars(audio_path, srcAudioPath);
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

    if (nullptr == inputPath_ || nullptr == outputPath_ || nullptr == musicPath_) {
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

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_mixAudios(JNIEnv *env, jclass clazz, jstring outputPath_,
                                         jobjectArray audios, jobject onActionListenerObj) {
    if (nullptr == outputPath_ || nullptr == audios) {
        BZLogUtil::logE("nullptr == outputPath_  || nullptr == audios");
        return -1;
    }
    size_t arrayLength = static_cast<size_t>(env->GetArrayLength(audios));
    if (arrayLength <= 1) {
        BZLogUtil::logE("arrayLength <= 1");
        return -1;
    }
    char **pstr = (char **) malloc(arrayLength * sizeof(char *));
    memset(pstr, 0, arrayLength * sizeof(char *));

    int i = 0;;
    list<const char *> availableAudioList;
    for (i = 0; i < arrayLength; i++) {
        jstring javaPath = (jstring) env->GetObjectArrayElement(audios, i);
        const char *path = env->GetStringUTFChars(javaPath, 0);
        size_t length = strlen(path) + 1;
        char *buffer = static_cast<char *>(malloc(length));
        memset(buffer, 0, length);
        sprintf(buffer, "%s", path);
        env->ReleaseStringUTFChars(javaPath, path);
        pstr[i] = buffer;

        bool hasAudio = VideoUtil::hasAudio(buffer);
        if (hasAudio) {
            availableAudioList.push_back(buffer);
        }
    }
    if (availableAudioList.empty()) {
        BZLogUtil::logE("availableAudioList.empty()");
        for (i = 0; i < arrayLength; i++) {
            free(pstr[i]);
        }
        free(pstr);
        return -1;
    }
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);
    auto *onActionListener = new OnActionListener(onActionListenerObj);

    BackgroundMusicUtil backgroundMusicUtil;
    int ret = backgroundMusicUtil.mixMusic(&availableAudioList, outputPath, onActionListener);
    if (ret >= 0) {
        onActionListener->success();
    } else {
        onActionListener->fail();
    }
    delete (onActionListener);
    for (i = 0; i < arrayLength; i++) {
        free(pstr[i]);
    }
    free(pstr);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_delayMusic(JNIEnv *env, jclass clazz, jstring musicPath_,
                                          jstring outputPath_, jlong time,
                                          jobject on_action_listener) {
    const char *musicPath = env->GetStringUTFChars(musicPath_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    auto *onActionListener = new OnActionListener(on_action_listener);
    BackgroundMusicUtil addBackgroundMusic;
    int ret = addBackgroundMusic.delayMusic(musicPath, outputPath, time, onActionListener);

    if (ret < 0) {
        onActionListener->fail();
    } else {
        onActionListener->success();
    }
    delete (onActionListener);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    env->ReleaseStringUTFChars(musicPath_, musicPath);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_getBitmapFromVideo(JNIEnv *env, jclass clazz, jstring videoPath_,
                                                  jint image_count, jint scale2_width,
                                                  jlong startTime, jlong endTime,
                                                  jobject onGetBitmapFromVideoListener) {
    if (nullptr == videoPath_) {
        BZLogUtil::logE("getBitmapFromVideo nullptr==videoPath_");
        return -1;
    }
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);

    JMethodInfo *jMethodInfo = new JMethodInfo();
    jMethodInfo->obj = env->NewGlobalRef(onGetBitmapFromVideoListener);
    jclass classListener = env->GetObjectClass(onGetBitmapFromVideoListener);
    jMethodInfo->methodID = env->GetMethodID(classListener,
                                             "onGetBitmapFromVideo",
                                             "(ILandroid/graphics/Bitmap;)V");

    GetVideoFrameUtil getVideoFrameUtil;
    int ret = getVideoFrameUtil.getVideoFrame(videoPath,
                                              image_count,
                                              scale2_width,
                                              startTime,
                                              endTime,
                                              reinterpret_cast<int64_t>(jMethodInfo),
                                              getBitmapFromVideoCallBack);
    env->ReleaseStringUTFChars(videoPath_, videoPath);
    env->DeleteLocalRef(classListener);
    env->DeleteGlobalRef(jMethodInfo->obj);
    delete jMethodInfo;
    return ret;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_luoye_bzmedia_BZMedia_getVideoFrameAtTime(JNIEnv *jniEnv, jclass type,
                                                   jstring videoPath_, jlong time,
                                                   jint scale2Width) {
    if (nullptr == videoPath_ || time < 0) {
        return nullptr;
    }
    const char *videoPath = jniEnv->GetStringUTFChars(videoPath_, 0);

    GetVideoFrameUtil getVideoFrameUtil;
    AVFrame *avFrame = getVideoFrameUtil.getVideoFrameAtTime(videoPath, time, scale2Width);
    jniEnv->ReleaseStringUTFChars(videoPath_, videoPath);
    if (nullptr != avFrame) {
        int width = avFrame->width;
        int height = avFrame->height;
        int ret = 0;
        void *targetPixels = nullptr;
        jclass bitmapCls = jniEnv->FindClass("android/graphics/Bitmap");

        jobject newBitmap;
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

        newBitmap = jniEnv->CallStaticObjectMethod(bitmapCls, createBitmapFunctionMethodID, width,
                                                   height, bitmapConfigObj);

        if ((ret = AndroidBitmap_lockPixels(jniEnv, newBitmap, &targetPixels)) < 0) {
            BZLogUtil::logE(
                    "gifDataCallBack AndroidBitmap_lockPixels() targetPixels failed ! error=%d",
                    ret);
        }
        if (ret >= 0) {
            memcpy(targetPixels, avFrame->data[0], (size_t) (width * height * 4));
            AndroidBitmap_unlockPixels(jniEnv, newBitmap);
        }
        av_frame_free(&avFrame);
        jniEnv->DeleteLocalRef(bitmapCls);
        jniEnv->DeleteLocalRef(configName);
        jniEnv->DeleteLocalRef(bitmapConfigObj);
        jniEnv->DeleteLocalRef(bitmapConfigClass);

        return newBitmap;
    }
    return nullptr;
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

extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_getVideoInfo(JNIEnv *env, jclass type,
                                            jstring videoPath_, jobject sendMediaInfoListener) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);

    JMethodInfo *jMethodInfo = new JMethodInfo();
    jMethodInfo->obj = sendMediaInfoListener;
    jMethodInfo->methodID = env->GetMethodID(env->GetObjectClass(sendMediaInfoListener),
                                             "sendMediaInfo",
                                             "(II)V");
    int temp = VideoUtil::getVideoInfo(videoPath, reinterpret_cast<long>(jMethodInfo),
                                       sendMediaInfo);

    delete jMethodInfo;
    env->ReleaseStringUTFChars(videoPath_, videoPath);
    return temp;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_mergeVideoOrAudio(
        JNIEnv *env, jclass type,
        jobjectArray inputPaths,
        jstring outPutPath, jboolean needVideo, jboolean needAudio, jobject onActionListenerObj) {

    if (!needVideo && !needAudio) {
        BZLogUtil::logE("!needVideo&&!needAudio");
        return -1;
    }
    size_t arrayLength = static_cast<size_t>(env->GetArrayLength(inputPaths));
    char **pstr = (char **) malloc(arrayLength * sizeof(char *));
    memset(pstr, 0, arrayLength * sizeof(char *));

    int i = 0;;
    for (i = 0; i < arrayLength; i++) {
        jstring javaPath = (jstring) env->GetObjectArrayElement(inputPaths, i);
        const char *path = env->GetStringUTFChars(javaPath, 0);
        size_t length = strlen(path) + 1;
        char *buffer = static_cast<char *>(malloc(length));
        memset(buffer, 0, length);
        sprintf(buffer, "%s", path);
        env->ReleaseStringUTFChars(javaPath, path);

        pstr[i] = buffer;
    }
    const char *output_path = env->GetStringUTFChars(outPutPath, 0);
    OnActionListener *onActionListener = new OnActionListener(onActionListenerObj);
    MergeVideoUtil mergeVideoUtil;
    int ret = mergeVideoUtil.startMergeVideo(pstr, static_cast<int>(arrayLength), output_path,
                                             needVideo, needAudio,
                                             onActionListener);

    for (i = 0; i < arrayLength; i++) {
        free(pstr[i]);
    }
    free(pstr);
    if (ret < 0) {
        onActionListener->fail();
    } else {
        onActionListener->success();
    }
    delete (onActionListener);
    env->ReleaseStringUTFChars(outPutPath, output_path);
    return ret;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_mergeAudio(JNIEnv *env, jclass clazz, jobjectArray inputPaths,
                                          jstring outPutPath, jobject onActionListenerObj) {

    size_t arrayLength = static_cast<size_t>(env->GetArrayLength(inputPaths));
    const char *output_path = env->GetStringUTFChars(outPutPath, 0);
    BZLogUtil::logD("output_path=%s", output_path);
    string outputParentDir;
    outputParentDir.append(output_path);
    outputParentDir = outputParentDir.substr(0, outputParentDir.find_last_of("/"));

    char **pstr = (char **) malloc(arrayLength * sizeof(char *));
    memset(pstr, 0, arrayLength * sizeof(char *));

    list<string *> willDeleteFile;
    //首先进行格式与声道统一
    string concatStr;
    for (int i = 0; i < arrayLength; i++) {
        jstring javaPath = (jstring) env->GetObjectArrayElement(inputPaths, i);
        const char *path = env->GetStringUTFChars(javaPath, 0);
        if (VideoUtil::isStandardAudio(path)) {
            BZLogUtil::logD("isStandardAudio path=%s", path);
            size_t length = strlen(path) + 1;
            char *buffer = static_cast<char *>(malloc(length));
            memset(buffer, 0, length);
            sprintf(buffer, "%s", path);
            pstr[i] = buffer;
        } else {
            string audioStreamOutPath;
            char audioStreamName[128] = {0};
            sprintf(audioStreamName, "/audio_temp_%lld", getMicrosecondTime());
            audioStreamOutPath.append(outputParentDir);
            audioStreamOutPath.append(audioStreamName);
            audioStreamOutPath.append(".m4a");

            const char *outTempPath = audioStreamOutPath.c_str();
            const char *cmdFormat = "ffmpeg -y -i %s -vn -ar 44100 -ac 2 %s";

            char finalCmd[1024] = {0};
            sprintf(finalCmd, cmdFormat, path, outTempPath);
            BZLogUtil::logD("cmd=%s", finalCmd);
            int ret = executeFFmpegCommand(0, finalCmd, nullptr);
            if (ret < 0) {
                BZLogUtil::logE("mergeAudio executeFFmpegCommand fail");
            }
            size_t length = strlen(outTempPath) + 1;
            char *buffer = static_cast<char *>(malloc(length));
            memset(buffer, 0, length);
            sprintf(buffer, "%s", outTempPath);
            pstr[i] = buffer;
            willDeleteFile.push_back(new string(outTempPath));
        }
        env->ReleaseStringUTFChars(javaPath, path);
    }
    auto *onActionListener = new OnActionListener(onActionListenerObj);
    MergeVideoUtil mergeVideoUtil;
    int ret = mergeVideoUtil.startMergeVideo(pstr, static_cast<int>(arrayLength), output_path,
                                             false, true,
                                             onActionListener);
    if (ret < 0) {
        onActionListener->fail();
    } else {
        onActionListener->success();
    }

    for (int i = 0; i < arrayLength; i++) {
        free(pstr[i]);
    }
    free(pstr);

    auto list_iterator = willDeleteFile.begin();
    while (list_iterator != willDeleteFile.end()) {
        string *path = *list_iterator;
        BZLogUtil::logD("remove path=%s", path->c_str());
        remove(path->c_str());
        delete (path);
        list_iterator++;
    }
    delete (onActionListener);
    env->ReleaseStringUTFChars(outPutPath, output_path);
    return ret;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_luoye_bzmedia_BZMedia_hasAudio(JNIEnv *env, jclass clazz, jstring path) {
    const char *srcPath = env->GetStringUTFChars(path, 0);
    bool hasAudio = VideoUtil::hasAudio(srcPath);
    env->ReleaseStringUTFChars(path, srcPath);
    return hasAudio;
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_initGLContext(JNIEnv *env, jclass type, jint width,
                                             jint height) {

    EGLContextUtil *eglContextUtil = new EGLContextUtil();
    eglContextUtil->initEGLContext(width, height);
    return reinterpret_cast<jlong>(eglContextUtil);

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_releaseEGLContext(JNIEnv *env, jclass type, jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    EGLContextUtil *eglContextUtil = reinterpret_cast<EGLContextUtil *>(nativeHandle);
    eglContextUtil->releaseEGLContext();
    delete (eglContextUtil);
    return 0;
}
extern "C" JNIEXPORT jobject JNICALL
Java_com_luoye_bzmedia_BZMedia_bzReadPixelsNative(JNIEnv *env, jclass type,
                                                  jint startX,
                                                  jint startY, jint width,
                                                  jint height) {
    if (width < 1 || height < 1) {
        BZLogUtil::logE("params is error width<1||height<1");
        return nullptr;
    }

    JNIEnv *jniEnv = nullptr;
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
        BZLogUtil::logE("gifDataCallBack AndroidBitmap_lockPixels() targetPixels failed ! error=%d",
                        ret);
    }
    if (ret >= 0) {
        char *data = new char[width * height * 4];
        glReadPixels(startX, startY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        memcpy(targetPixels, data, (size_t) (width * height * 4));
        AndroidBitmap_unlockPixels(jniEnv, newBitmap);
        delete[](data);
    }

    jniEnv->DeleteLocalRef(bitmapCls);
    jniEnv->DeleteLocalRef(configName);
    jniEnv->DeleteLocalRef(bitmapConfigObj);
    jniEnv->DeleteLocalRef(bitmapConfigClass);

    if (needDetach)JvmManager::getJavaVM()->DetachCurrentThread();

    return newBitmap;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_printVideoTimeStamp(JNIEnv *env, jclass clazz, jstring videoPath_) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);
    int ret = VideoUtil::printVideoTimeStamp(videoPath);

    env->ReleaseStringUTFChars(videoPath_, videoPath);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_separateAudioStream(JNIEnv *env, jclass clazz, jstring video_path,
                                                   jstring outputPath_) {
    if (nullptr == video_path || nullptr == outputPath_) {
        BZLogUtil::logE("separateAudioStream nullptr==input_path_|| nullptr==outputPath_");
        return -1;
    }
    const char *srcVideoPath = env->GetStringUTFChars(video_path, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    int ret = -1;
    if (VideoUtil::hasAudio(srcVideoPath)) {
        char cmd[1024] = {0};
        string tempExtraMusicPath;
        tempExtraMusicPath.append(outputPath);
        string suffixMusic = tempExtraMusicPath.substr(tempExtraMusicPath.find_last_of("."),
                                                       tempExtraMusicPath.length());
        //检测是不是m4a
        if (std::strcmp(".m4a", suffixMusic.c_str()) == 0 ||
            std::strcmp(".M4A", suffixMusic.c_str()) == 0) {
            sprintf(cmd, "ffmpeg -y -i \"%s\" -vn -acodec copy \"%s\"",
                    srcVideoPath, outputPath);
        } else {
            sprintf(cmd, "ffmpeg -y -i \"%s\" -vn \"%s\"",
                    srcVideoPath, outputPath);
        }
        BZLogUtil::logD("cmd=%s", cmd);
        ret = executeFFmpegCommand(0, cmd, nullptr);
    }
    env->ReleaseStringUTFChars(video_path, srcVideoPath);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_repeatVideoToDuration(JNIEnv *env, jclass clazz, jstring input_path_,
                                                     jstring outputPath_, jlong duration) {
    if (nullptr == input_path_ || nullptr == outputPath_ || duration <= 0) {
        BZLogUtil::logE(
                "repeatVideoToDuration nullptr == input_path_ || nullptr == outputPath_ || duration <= 0");
        return -1;
    }
    const char *inputPath = env->GetStringUTFChars(input_path_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    int ret = VideoUtil::repeatVideoToDuration(inputPath, outputPath, duration);

    env->ReleaseStringUTFChars(input_path_, inputPath);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_setVideoDuration(JNIEnv *env, jclass clazz, jstring input_path_,
                                                jstring outputPath_, jint duration) {
    if (nullptr == input_path_ || nullptr == outputPath_ || duration <= 0) {
        BZLogUtil::logE(
                "setVideoDuration nullptr == input_path_ || nullptr == outputPath_ || duration <= 0");
        return -1;
    }
    const char *inputPath = env->GetStringUTFChars(input_path_, 0);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    int ret = VideoUtil::setVideoDuration(inputPath, outputPath, duration);

    env->ReleaseStringUTFChars(input_path_, inputPath);
    env->ReleaseStringUTFChars(outputPath_, outputPath);
    return ret;
}