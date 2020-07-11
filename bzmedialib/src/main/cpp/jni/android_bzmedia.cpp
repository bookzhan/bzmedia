#include <jni.h>
#include <string>
#include <common/BZLogUtil.h>
#include <recorder/VideoRecordParams.h>
#include <recorder/VideoRecorder.h>
#include <mediaedit/AdjustVideoSpeedUtil.h>
#include "ffmpeg_base_info.h"

typedef long;
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
const char *TAG = "bz_";

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


extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_initNative(JNIEnv *env, jclass clazz, jobject context,
                                          jboolean is_debug, jint sdk_int) {
    BZLogUtil::enableLog = is_debug;
    __android_log_write(ANDROID_LOG_DEBUG, BZLOG_TAG, "welcome bzmedia ^_^");
    if (is_debug) {
        av_log_set_callback(log_call_back);
    }
    TextureConvertYUVUtil::useHDShader = sdk_int >= 19;
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
    long ret= videoRecorder->addAudioData(reinterpret_cast<unsigned char *>(data), length);
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