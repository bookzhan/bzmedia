//
/**
 * Created by bookzhan on 2018-08-27 13:28.
 * 说明: 视频编辑SDK入口
 */
//

#include <jni.h>
#include <common/BZLogUtil.h>
#include <mediaedit/AudioFeatureInfoUtil.h>
#include <mediaedit/BackgroundMusicUtil.h>
#include <mediaedit/AudioTransCode.h>
#include "../mediaedit/VideoUtil.h"
#include "../mediaedit/VideoTranscodeParams.h"
#include "../common/JvmManager.h"
#include "../common/CallbackMethodIDInfo.h"
#include "ffmpeg_cmd.h"

typedef struct TransCodeMethodInfo {
    jobject classObj = nullptr;
    jmethodID pcmCallBackCallBackMethodId = nullptr;
    jmethodID videoTextureCallBackMethodId = nullptr;
    jmethodID progressCallBackMethodId = nullptr;
    jmethodID finishCallBackMethodId = nullptr;
    jbyteArray videoTransPcmjbyteArray = nullptr;
} TransCodeMethodInfo;

typedef struct AudioFeatureMethodInfo {
    jobject classObj = nullptr;
    jmethodID callBackMethodId = nullptr;
} AudioFeatureMethodInfo;

typedef struct AudioTransCodeMethodInfo {
    jobject classObj = nullptr;
    jmethodID pcmCallBackCallBackMethodId = nullptr;
    jmethodID progressCallBackMethodId = nullptr;
    jmethodID finishCallBackMethodId = nullptr;
    jbyteArray videoTransPcmjbyteArray = nullptr;
} AudioTransCodeMethodInfo;


void audioFeatureCallBack(int64_t methodInfo, int64_t videoTime, float feature) {
    if (methodInfo == 0) {
        return;
    }
    AudioFeatureMethodInfo *featureMethodInfo = reinterpret_cast<AudioFeatureMethodInfo *>(methodInfo);
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == featureMethodInfo->classObj ||
        nullptr == featureMethodInfo->callBackMethodId || nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }

    jniEnv->CallVoidMethod(featureMethodInfo->classObj,
                           featureMethodInfo->callBackMethodId, videoTime, feature);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();

}

void transCodeProgressCallBack(int64_t methodInfo, float progress) {
    if (methodInfo == 0) {
        return;
    }
    TransCodeMethodInfo *transCodeMethodInfo = reinterpret_cast<TransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    if (nullptr == transCodeMethodInfo->classObj ||
        nullptr == transCodeMethodInfo->progressCallBackMethodId || nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    if (progress > 1)progress = 1;
    if (progress < 0)progress = 0;

    jniEnv->CallVoidMethod(transCodeMethodInfo->classObj,
                           transCodeMethodInfo->progressCallBackMethodId, progress);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();

}

void transCodeFinishCallBack(int64_t methodInfo) {
    if (methodInfo == 0) {
        return;
    }
    TransCodeMethodInfo *transCodeMethodInfo = reinterpret_cast<TransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    if (nullptr == transCodeMethodInfo->classObj ||
        nullptr == transCodeMethodInfo->finishCallBackMethodId || nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }

    jniEnv->CallVoidMethod(transCodeMethodInfo->classObj,
                           transCodeMethodInfo->finishCallBackMethodId);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();

}

const char *videoTransCodePCMCallBack(int64_t methodInfo, const char *pcmData, int length) {
    if (methodInfo == 0) {
        return nullptr;
    }
    TransCodeMethodInfo *transCodeMethodInfo = reinterpret_cast<TransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == transCodeMethodInfo->classObj ||
        nullptr == transCodeMethodInfo->pcmCallBackCallBackMethodId || nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return nullptr;
    }
    if (nullptr == transCodeMethodInfo->videoTransPcmjbyteArray) {
        jbyteArray pcmjbyteArrayTemp = jniEnv->NewByteArray(length);
        transCodeMethodInfo->videoTransPcmjbyteArray = (jbyteArray) jniEnv->NewGlobalRef(
                pcmjbyteArrayTemp);
    }
    jniEnv->SetByteArrayRegion(transCodeMethodInfo->videoTransPcmjbyteArray, 0, length,
                               (const jbyte *) pcmData);

    jbyteArray byteArray = (jbyteArray) jniEnv->CallObjectMethod(transCodeMethodInfo->classObj,
                                                                 transCodeMethodInfo->pcmCallBackCallBackMethodId,
                                                                 transCodeMethodInfo->videoTransPcmjbyteArray);

    if (nullptr == byteArray) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return nullptr;
    }
    char *pcmBuffer = (char *) malloc(sizeof(char) * jniEnv->GetArrayLength(byteArray));
    jniEnv->GetByteArrayRegion(byteArray, 0, jniEnv->GetArrayLength(byteArray),
                               (jbyte *) pcmBuffer);

    jniEnv->DeleteLocalRef(byteArray);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
    return (const char *) pcmBuffer;
}

int videoTextureCallBack(int64_t methodInfo, int textureId, int width, int height,
                         int64_t pts,
                         int64_t videoTime) {
    if (methodInfo == 0) {
        return -1;
    }
    TransCodeMethodInfo *transCodeMethodInfo = reinterpret_cast<TransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == transCodeMethodInfo->classObj ||
        nullptr == transCodeMethodInfo->videoTextureCallBackMethodId ||
        nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return 0;
    }
    textureId = jniEnv->CallIntMethod(transCodeMethodInfo->classObj,
                                      transCodeMethodInfo->videoTextureCallBackMethodId, textureId,
                                      width, height, pts, videoTime);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
    return textureId;
}


VideoTransCodeParams *parseVideoTransCodeParamsObj(JNIEnv *env, jobject videoTransCodeParamsObj) {
    VideoTransCodeParams *transCodeParams = new VideoTransCodeParams();
    jclass VideoTransCodeParamsClass = env->GetObjectClass(videoTransCodeParamsObj);
    jstring inputPath_ = (jstring) env->GetObjectField(videoTransCodeParamsObj,
                                                       env->GetFieldID(VideoTransCodeParamsClass,
                                                                       "inputPath",
                                                                       "Ljava/lang/String;"));

    jstring outPutPath_ = (jstring) env->GetObjectField(videoTransCodeParamsObj,
                                                        env->GetFieldID(VideoTransCodeParamsClass,
                                                                        "outputPath",
                                                                        "Ljava/lang/String;"));
    const char *inputPath = nullptr;
    if (nullptr != inputPath_) {
        inputPath = env->GetStringUTFChars(inputPath_, 0);
        transCodeParams->inputPath = inputPath;
        BZLogUtil::logD("inputPath=%s", inputPath);
        env->DeleteLocalRef(inputPath_);
    } else {
        env->DeleteLocalRef(inputPath_);
        BZLogUtil::logE("inputPath is nullptr");
        delete (transCodeParams);
        return nullptr;
    }
    const char *outPutPath = nullptr;
    if (nullptr != outPutPath_) {
        outPutPath = env->GetStringUTFChars(outPutPath_, 0);
        transCodeParams->outputPath = outPutPath;
        BZLogUtil::logD("outPutPath=%s", outPutPath);
        env->DeleteLocalRef(outPutPath_);
    } else {
        env->DeleteLocalRef(outPutPath_);
        BZLogUtil::logE("outPutPath is nullptr");
        delete (transCodeParams);
        return nullptr;
    }
    jint gopSize = env->GetIntField(videoTransCodeParamsObj,
                                    env->GetFieldID(VideoTransCodeParamsClass, "gopSize", "I"));
    transCodeParams->gopSize = gopSize;
    BZLogUtil::logD("gopSize=%d", gopSize);

    jlong startTime = env->GetLongField(videoTransCodeParamsObj,
                                        env->GetFieldID(VideoTransCodeParamsClass, "startTime",
                                                        "J"));
    transCodeParams->startTime = startTime;
    BZLogUtil::logD("startTime=%lld", startTime);

    jlong endTime = env->GetLongField(videoTransCodeParamsObj,
                                      env->GetFieldID(VideoTransCodeParamsClass, "endTime",
                                                      "J"));
    transCodeParams->endTime = endTime;
    BZLogUtil::logD("endTime=%lld", endTime);

    jint frameRate = env->GetIntField(videoTransCodeParamsObj,
                                      env->GetFieldID(VideoTransCodeParamsClass, "frameRate", "I"));
    transCodeParams->frameRate = frameRate;
    BZLogUtil::logD("frameRate=%d", frameRate);

    jint targetWidth = env->GetIntField(videoTransCodeParamsObj,
                                        env->GetFieldID(VideoTransCodeParamsClass, "targetWidth",
                                                        "I"));
    transCodeParams->targetWidth = targetWidth;
    BZLogUtil::logD("targetWidth=%d", targetWidth);

    jint targetHeight = env->GetIntField(videoTransCodeParamsObj,
                                         env->GetFieldID(VideoTransCodeParamsClass, "targetHeight",
                                                         "I"));
    transCodeParams->targetHeight = targetHeight;
    BZLogUtil::logD("targetHeight=%d", targetHeight);


    env->DeleteLocalRef(videoTransCodeParamsObj);
    env->DeleteLocalRef(VideoTransCodeParamsClass);
    return transCodeParams;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_luoye_bzmedia_BZMedia_videoIsSupportForBZMedia(JNIEnv *env, jclass type, jstring videoPath_) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);

    bool result = VideoUtil::videoIsSupport(videoPath);

    env->ReleaseStringUTFChars(videoPath_, videoPath);

    return static_cast<jboolean>(result);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_luoye_bzmedia_BZMedia_audioIsSupport(JNIEnv *env, jclass type, jstring audioPath_) {
    const char *audioPath = env->GetStringUTFChars(audioPath_, 0);

    bool result = VideoUtil::audioIsSupport(audioPath);

    env->ReleaseStringUTFChars(audioPath_, audioPath);
    return static_cast<jboolean>(result);
}


extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_startVideoTransCode(JNIEnv *env, jclass type,
                                                   jobject videoTransCodeParamsObj,
                                                   jobject callBackObj) {
    if (nullptr == videoTransCodeParamsObj) {
        BZLogUtil::logE("nullptr==videoTransCodeParamsObj");
        return -1;
    }
    int ret = 0;
    VideoTransCodeParams *transCodeParams = parseVideoTransCodeParamsObj(env,
                                                                         videoTransCodeParamsObj);
    string cmd_buffer;
    cmd_buffer.append("ffpeg -y");
    char buffer[128];
    if (transCodeParams->startTime > 0) {
        memset(buffer, 0, 128);
        sprintf(buffer, "%.3f", transCodeParams->startTime / 1000.0f);
        cmd_buffer.append(" -ss ");
        cmd_buffer.append(buffer);
    }
    if (transCodeParams->endTime > 0) {
        memset(buffer, 0, 128);
        sprintf(buffer, "%.3f", (transCodeParams->endTime - transCodeParams->startTime) / 1000.0f);
        cmd_buffer.append(" -t ");
        cmd_buffer.append(buffer);
    }
    cmd_buffer.append(" -i \"");
    cmd_buffer.append(transCodeParams->inputPath);
    cmd_buffer.append("\"");

    if (transCodeParams->targetHeight > 0 || transCodeParams->targetWidth > 0) {
        memset(buffer, 0, 128);
        sprintf(buffer, "%d:%d", transCodeParams->targetWidth, transCodeParams->targetHeight);
        cmd_buffer.append(" -vf scale=");
        cmd_buffer.append(buffer);
    }
    if (transCodeParams->gopSize > 0) {
        memset(buffer, 0, 128);
        sprintf(buffer, " -g %d", transCodeParams->gopSize);
        cmd_buffer.append(buffer);
    }
    if (transCodeParams->frameRate > 0) {
        memset(buffer, 0, 128);
        sprintf(buffer, " -r %d", transCodeParams->frameRate);
        cmd_buffer.append(buffer);
    }
    cmd_buffer.append(" \"");
    cmd_buffer.append(transCodeParams->outputPath);
    cmd_buffer.append("\"");
    const char *cmd = cmd_buffer.c_str();
    BZLogUtil::logD("cmd=%s", cmd);
    OnActionListener *onActionListener = new OnActionListener(callBackObj);
    int64_t total_time = -1;
    if (transCodeParams->endTime - transCodeParams->startTime > 0) {
        total_time = transCodeParams->endTime - transCodeParams->startTime;
        int64_t duration = VideoUtil::getMediaDuration(transCodeParams->inputPath);
        if (duration < total_time) {
            total_time = duration - transCodeParams->startTime;
        }
    }
    ret = executeFFmpegCommand4TotalTime(reinterpret_cast<int64_t>(onActionListener), cmd,
                                         OnActionListener::progressCallBack, total_time);
    if (ret >= 0) {
        onActionListener->success();
    } else {
        onActionListener->fail();
    }
    delete (onActionListener);
    delete (transCodeParams);
    return ret;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_stopVideoTransCode(JNIEnv *env, jclass type) {
    return cancelExecuteFFmpegCommand();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_getAudioFeatureInfo(JNIEnv *env, jclass clazz, jstring audioPath_,
                                                   jint samples,
                                                   jobject featureObj) {
    const char *audioPath = env->GetStringUTFChars(audioPath_, 0);

    AudioFeatureInfoUtil audioFeatureInfoUtil;
    int result = 0;
    if (nullptr != featureObj) {
        AudioFeatureMethodInfo *audioFeatureMethodInfo = new AudioFeatureMethodInfo();
        jclass audioFeatureClass = env->GetObjectClass(featureObj);
        jmethodID audioFeatureMethodID = env->GetMethodID(audioFeatureClass, "onAudioFeatureInfo",
                                                          "(JF)V");

        audioFeatureMethodInfo->classObj = featureObj;
        audioFeatureMethodInfo->callBackMethodId = audioFeatureMethodID;

        audioFeatureInfoUtil.setCallBackHandle(reinterpret_cast<int64_t>(audioFeatureMethodInfo));

        result = audioFeatureInfoUtil.getAudioFeatureInfo(audioPath, samples, audioFeatureCallBack);

        delete (audioFeatureMethodInfo);
    } else {
        result = audioFeatureInfoUtil.getAudioFeatureInfo(audioPath, samples, audioFeatureCallBack);
    }
    env->ReleaseStringUTFChars(audioPath_, audioPath);
    return result;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_audioFade(JNIEnv *env, jclass clazz, jstring mediaPath_,
                                         jstring outPath_, jboolean need_fade_in,
                                         jlong fade_in_duration, jboolean need_fade_out,
                                         jlong fade_out_duration) {
    const char *mediaPath = env->GetStringUTFChars(mediaPath_, 0);
    const char *outPath = env->GetStringUTFChars(outPath_, 0);

    int ret = 0;
    BackgroundMusicUtil backgroundMusicUtil;
    backgroundMusicUtil.fadeMusic(mediaPath, outPath, need_fade_in, fade_in_duration, need_fade_out,
                                  fade_out_duration);
    env->ReleaseStringUTFChars(mediaPath_, mediaPath);
    env->ReleaseStringUTFChars(outPath_, outPath);
    return ret;
}


void audioTransCodeProgressCallBack(int64_t methodInfo, float progress) {
    if (methodInfo == 0) {
        return;
    }
    auto *transCodeMethodInfo = reinterpret_cast<AudioTransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    if (nullptr == transCodeMethodInfo->classObj ||
        nullptr == transCodeMethodInfo->progressCallBackMethodId || nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    if (progress > 1)progress = 1;
    if (progress < 0)progress = 0;

    jniEnv->CallVoidMethod(transCodeMethodInfo->classObj,
                           transCodeMethodInfo->progressCallBackMethodId, progress);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();

}

const char *audioTransCodePCMCallBack(int64_t methodInfo, const char *pcmData, int length) {
    if (methodInfo == 0) {
        return nullptr;
    }
    auto *transCodeMethodInfo = reinterpret_cast<AudioTransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == transCodeMethodInfo->classObj ||
        nullptr == transCodeMethodInfo->pcmCallBackCallBackMethodId || nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return nullptr;
    }
    if (nullptr == transCodeMethodInfo->videoTransPcmjbyteArray) {
        jbyteArray pcmjbyteArrayTemp = jniEnv->NewByteArray(length);
        transCodeMethodInfo->videoTransPcmjbyteArray = (jbyteArray) jniEnv->NewGlobalRef(
                pcmjbyteArrayTemp);
    }
    jniEnv->SetByteArrayRegion(transCodeMethodInfo->videoTransPcmjbyteArray, 0, length,
                               (const jbyte *) pcmData);

    jbyteArray byteArray = (jbyteArray) jniEnv->CallObjectMethod(transCodeMethodInfo->classObj,
                                                                 transCodeMethodInfo->pcmCallBackCallBackMethodId,
                                                                 transCodeMethodInfo->videoTransPcmjbyteArray);
    if (nullptr == byteArray) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return nullptr;
    }
    char *pcmBuffer = (char *) malloc(sizeof(char) * jniEnv->GetArrayLength(byteArray));
    jniEnv->GetByteArrayRegion(byteArray, 0, jniEnv->GetArrayLength(byteArray),
                               (jbyte *) pcmBuffer);

    jniEnv->DeleteLocalRef(byteArray);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
    return (const char *) pcmBuffer;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_startAudioTransCode(JNIEnv *env, jclass clazz, jstring input_path_,
                                                   jstring output_path_,
                                                   jobject callBackObj) {
    AudioTransCode audioTransCode;
    long methodInfoHandle = 0;
    if (nullptr != callBackObj) {
        auto *transCodeMethodInfo = new AudioTransCodeMethodInfo();
        jclass callBackClass = env->GetObjectClass(callBackObj);
        transCodeMethodInfo->classObj = env->NewGlobalRef(callBackObj);
        transCodeMethodInfo->progressCallBackMethodId = env->GetMethodID(callBackClass,
                                                                         "transCodeProgress",
                                                                         "(F)V");

        transCodeMethodInfo->pcmCallBackCallBackMethodId = env->GetMethodID(callBackClass,
                                                                            "onPcmCallBack",
                                                                            "([B)[B");

        methodInfoHandle = reinterpret_cast<long>(transCodeMethodInfo);
    }
    const char *input_path = env->GetStringUTFChars(input_path_, 0);
    const char *output_path = env->GetStringUTFChars(output_path_, 0);

    int ret = audioTransCode.startTransCode(input_path, output_path, methodInfoHandle,
                                            audioTransCodePCMCallBack,
                                            audioTransCodeProgressCallBack);
    env->ReleaseStringUTFChars(input_path_, input_path);
    env->ReleaseStringUTFChars(output_path_, output_path);
    return ret;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_luoye_bzmedia_BZMedia_videoCodecIsSupportForExoPlayer(JNIEnv *env, jclass clazz,
                                                               jstring videoPath_) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);

    bool result = VideoUtil::videoCodecIsSupportForExoPlayer(videoPath);

    env->ReleaseStringUTFChars(videoPath_, videoPath);

    return static_cast<jboolean>(result);
}