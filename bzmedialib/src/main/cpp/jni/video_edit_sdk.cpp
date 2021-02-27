//
/**
 * Created by zhandalin on 2018-08-27 13:28.
 * 说明: 视频编辑SDK入口
 */
//

#include <jni.h>
#include <common/BZLogUtil.h>
#include "../mediaedit/VideoUtil.h"
#include "../mediaedit/VideoTranscodeParams.h"
#include "../common/JvmManager.h"
#include "../mediaedit/VideoTransCode.h"
#include "../common/CallbackMethodIDInfo.h"

typedef struct TransCodeMethodInfo {
    jobject classObj = nullptr;
    jmethodID pcmCallBackCallBackMethodId = nullptr;
    jmethodID videoTextureCallBackMethodId = nullptr;
    jmethodID progressCallBackMethodId = nullptr;
    jmethodID finishCallBackMethodId = nullptr;
    jbyteArray videoTransPcmjbyteArray = nullptr;
} TransCodeMethodInfo;


void transCodeProgressCallBack(int64_t methodInfo, float progress) {
    if (methodInfo == 0) {
        return;
    }
    TransCodeMethodInfo *transCodeMethodInfo = reinterpret_cast<TransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    if (NULL == transCodeMethodInfo->classObj ||
        NULL == transCodeMethodInfo->progressCallBackMethodId || NULL == jniEnv) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    if (progress > 1)progress = 1;
    if (progress < 0)progress = 0;

    jniEnv->CallVoidMethod(transCodeMethodInfo->classObj,
                           transCodeMethodInfo->progressCallBackMethodId, progress);

    jniEnv = NULL;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();

}

void transCodeFinishCallBack(int64_t methodInfo) {
    if (methodInfo == 0) {
        return;
    }
    TransCodeMethodInfo *transCodeMethodInfo = reinterpret_cast<TransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    if (NULL == transCodeMethodInfo->classObj ||
        NULL == transCodeMethodInfo->finishCallBackMethodId || NULL == jniEnv) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }

    jniEnv->CallVoidMethod(transCodeMethodInfo->classObj,
                           transCodeMethodInfo->finishCallBackMethodId);

    jniEnv = NULL;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();

}

const char *videoTransCodePCMCallBack(int64_t methodInfo, const char *pcmData, int length) {
    if (methodInfo == 0) {
        return nullptr;
    }
    TransCodeMethodInfo *transCodeMethodInfo = reinterpret_cast<TransCodeMethodInfo *>(methodInfo);

    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (NULL == transCodeMethodInfo->classObj ||
        NULL == transCodeMethodInfo->pcmCallBackCallBackMethodId || NULL == jniEnv) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return NULL;
    }
    if (NULL == transCodeMethodInfo->videoTransPcmjbyteArray) {
        jbyteArray pcmjbyteArrayTemp = jniEnv->NewByteArray(length);
        transCodeMethodInfo->videoTransPcmjbyteArray = (jbyteArray) jniEnv->NewGlobalRef(
                pcmjbyteArrayTemp);
    }
    jniEnv->SetByteArrayRegion(transCodeMethodInfo->videoTransPcmjbyteArray, 0, length,
                               (const jbyte *) pcmData);

    jbyteArray byteArray = (jbyteArray) jniEnv->CallObjectMethod(transCodeMethodInfo->classObj,
                                                                 transCodeMethodInfo->pcmCallBackCallBackMethodId,
                                                                 transCodeMethodInfo->videoTransPcmjbyteArray);

    if (NULL == byteArray) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return NULL;
    }
    char *pcmBuffer = (char *) malloc(sizeof(char) * jniEnv->GetArrayLength(byteArray));
    jniEnv->GetByteArrayRegion(byteArray, 0, jniEnv->GetArrayLength(byteArray),
                               (jbyte *) pcmBuffer);

    jniEnv->DeleteLocalRef(byteArray);

    jniEnv = NULL;
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

    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (NULL == transCodeMethodInfo->classObj ||
        NULL == transCodeMethodInfo->videoTextureCallBackMethodId ||
        NULL == jniEnv) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return 0;
    }
    textureId = jniEnv->CallIntMethod(transCodeMethodInfo->classObj,
                                      transCodeMethodInfo->videoTextureCallBackMethodId, textureId,
                                      width, height, pts, videoTime);

    jniEnv = NULL;
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
    const char *inputPath = NULL;
    if (NULL != inputPath_) {
        inputPath = env->GetStringUTFChars(inputPath_, 0);
        transCodeParams->inputPath = inputPath;
        BZLogUtil::logD("inputPath=%s", inputPath);
        env->DeleteLocalRef(inputPath_);
    } else {
        env->DeleteLocalRef(inputPath_);
        BZLogUtil::logE("inputPath is NULL");
        delete (transCodeParams);
        return NULL;
    }
    const char *outPutPath = NULL;
    if (NULL != outPutPath_) {
        outPutPath = env->GetStringUTFChars(outPutPath_, 0);
        transCodeParams->outputPath = outPutPath;
        BZLogUtil::logD("outPutPath=%s", outPutPath);
        env->DeleteLocalRef(outPutPath_);
    } else {
        env->DeleteLocalRef(outPutPath_);
        BZLogUtil::logE("outPutPath is NULL");
        delete (transCodeParams);
        return NULL;
    }
    transCodeParams->videoTransCodeFinish = transCodeFinishCallBack;
    transCodeParams->progressCallBack = transCodeProgressCallBack;

    jint gopSize = env->GetIntField(videoTransCodeParamsObj,
                                    env->GetFieldID(VideoTransCodeParamsClass, "gopSize", "I"));
    transCodeParams->gopSize = gopSize;
    BZLogUtil::logD("gopSize=%d", gopSize);


    jboolean doWithVideo = env->GetBooleanField(videoTransCodeParamsObj,
                                                env->GetFieldID(VideoTransCodeParamsClass,
                                                                "doWithVideo",
                                                                "Z"));
    BZLogUtil::logD("doWithVideo=%d", doWithVideo);
    transCodeParams->doWithVideo = doWithVideo;


    jboolean userSoftDecode = env->GetBooleanField(videoTransCodeParamsObj,
                                                   env->GetFieldID(VideoTransCodeParamsClass,
                                                                   "userSoftDecode",
                                                                   "Z"));
    BZLogUtil::logD("userSoftDecode=%d", userSoftDecode);
    transCodeParams->userSoftDecode = userSoftDecode;


    jboolean needCallBackVideo = env->GetBooleanField(videoTransCodeParamsObj,
                                                      env->GetFieldID(VideoTransCodeParamsClass,
                                                                      "needCallBackVideo",
                                                                      "Z"));
    BZLogUtil::logD("needCallBackVideo=%d", needCallBackVideo);
    transCodeParams->needCallBackVideo = needCallBackVideo;

    if (needCallBackVideo) {
        transCodeParams->videoTextureCallBack = videoTextureCallBack;
    }

    jboolean doWithAudio = env->GetBooleanField(videoTransCodeParamsObj,
                                                env->GetFieldID(VideoTransCodeParamsClass,
                                                                "doWithAudio",
                                                                "Z"));
    BZLogUtil::logD("doWithAudio=%d", doWithAudio);
    transCodeParams->doWithAudio = doWithAudio;
    if (doWithAudio) {
        transCodeParams->pcmCallBack = videoTransCodePCMCallBack;
    }


    jint maxWidth = env->GetIntField(videoTransCodeParamsObj,
                                     env->GetFieldID(VideoTransCodeParamsClass, "maxWidth", "I"));
    transCodeParams->maxWidth = maxWidth;
    BZLogUtil::logD("maxWidth=%d", maxWidth);


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

    jint videoRotate = env->GetIntField(videoTransCodeParamsObj,
                                        env->GetFieldID(VideoTransCodeParamsClass, "videoRotate",
                                                        "I"));
    transCodeParams->videoRotate = videoRotate;
    BZLogUtil::logD("videoRotate=%d", videoRotate);


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
Java_com_luoye_bzmedia_BZMedia_videoIsSupport(JNIEnv *env, jclass type, jstring videoPath_,
                                                  jboolean isSoftDecode) {
    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);

    bool result = VideoUtil::videoIsSupport(videoPath, isSoftDecode);

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

extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_BZMedia_initVideoTransCode(JNIEnv *env, jclass type) {
    VideoTransCode *videoTransCode = new VideoTransCode();
    return reinterpret_cast<jlong>(videoTransCode);
}


extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_startVideoTransCode(JNIEnv *env, jclass type, jlong nativeHandel,
                                                       jobject videoTransCodeParamsObj,
                                                       jobject callBackObj) {

    if (nativeHandel == 0) {
        return -1;
    }
    VideoTransCode *videoTransCode = reinterpret_cast<VideoTransCode *>(nativeHandel);

    VideoTransCodeParams *transCodeParams = parseVideoTransCodeParamsObj(env,
                                                                         videoTransCodeParamsObj);

    if (nullptr != callBackObj) {
        TransCodeMethodInfo *transCodeMethodInfo = new TransCodeMethodInfo();
        jclass callBackClass = env->GetObjectClass(callBackObj);
        transCodeMethodInfo->classObj = env->NewGlobalRef(callBackObj);
        transCodeMethodInfo->progressCallBackMethodId = env->GetMethodID(callBackClass,
                                                                         "videoTransCodeProgress",
                                                                         "(F)V");

        transCodeMethodInfo->pcmCallBackCallBackMethodId = env->GetMethodID(callBackClass,
                                                                            "onPcmCallBack",
                                                                            "([B)[B");

        transCodeMethodInfo->finishCallBackMethodId = env->GetMethodID(callBackClass,
                                                                       "videoTransCodeFinish",
                                                                       "()V");

        transCodeMethodInfo->videoTextureCallBackMethodId = env->GetMethodID(callBackClass,
                                                                             "onTextureCallBack",
                                                                             "(IIIJJ)I");


        videoTransCode->setMethodInfoHandle((int64_t) transCodeMethodInfo);
    }

    if (NULL == transCodeParams) {
        return -1;
    }

    int ret = videoTransCode->startTransCode(transCodeParams);
    delete (transCodeParams);
    return ret;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_BZMedia_stopVideoTransCode(JNIEnv *env, jclass type,
                                                      jlong nativeHandel) {

    if (nativeHandel == 0) {
        return 0;
    }
    VideoTransCode *videoTransCode = reinterpret_cast<VideoTransCode *>(nativeHandel);
    int64_t methodInfoHandle = videoTransCode->getMethodInfoHandle();
    videoTransCode->stopVideoTransCode();

    if (methodInfoHandle != 0) {
        TransCodeMethodInfo *transCodeMethodInfo = reinterpret_cast<TransCodeMethodInfo *>(methodInfoHandle);
        if (nullptr != transCodeMethodInfo->classObj) {
            env->DeleteGlobalRef(transCodeMethodInfo->classObj);
        }
        delete (transCodeMethodInfo);
    }
    delete (videoTransCode);
    return 0;
}

