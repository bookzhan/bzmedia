//
/**
 * Created by bookzhan on 2021-05-11 09:34.
 *description:
 */
//
#include <jni.h>
#include <common/BZLogUtil.h>
#include <player/VideoPlayerMethodInfo.h>
#include <player/VideoPlayerInitParams.h>
#include <cstring>
#include <malloc.h>
#include <common/JvmManager.h>
#include <player/VideoPlayer.h>

void videoPlayerProgressCallBack(int64_t methodHandle, float progress) {
    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (methodHandle == 0 || NULL == jniEnv) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    VideoPlayerMethodInfo *videoPlayerMethodInfo = (VideoPlayerMethodInfo *) methodHandle;

    jniEnv->CallVoidMethod(videoPlayerMethodInfo->listenerObj,
                           videoPlayerMethodInfo->onProgressChangedMethodId, progress);
    jniEnv = NULL;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void onVideoPlayCompletionCallBack(int64_t methodHandle, int isCompletion) {
    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (methodHandle == 0 || NULL == jniEnv) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    VideoPlayerMethodInfo *videoPlayerMethodInfo = (VideoPlayerMethodInfo *) methodHandle;

    jniEnv->CallVoidMethod(videoPlayerMethodInfo->listenerObj,
                           videoPlayerMethodInfo->onVideoPlayCompletionMethodId, isCompletion);

    jniEnv = NULL;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}


void
onVideoInfoAvailableCallBack(int64_t methodHandle, int videoWidth, int videoHeight, int videoRotate,
                             int64_t videoDuration, float videoFps) {
    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    JvmManager::getJavaVM()->AttachCurrentThread(&jniEnv, NULL);
    if (methodHandle == 0 || NULL == jniEnv) {
        jniEnv = NULL;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    VideoPlayerMethodInfo *videoPlayerMethodInfo = (VideoPlayerMethodInfo *) methodHandle;

    jniEnv->CallVoidMethod(videoPlayerMethodInfo->listenerObj,
                           videoPlayerMethodInfo->onVideoInfoAvailableMethodId, videoWidth,
                           videoHeight,
                           videoRotate, videoDuration, videoFps);
    jniEnv = NULL;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

VideoPlayerInitParams *getVideoPlayerInitParams(int64_t handle) {
    if (handle != 0) {
        VideoPlayer *videoPlayer = (VideoPlayer *) handle;
        return videoPlayer->getVideoPlayerInitParams();
    } else {
        BZLogUtil::logE("videoPlayerOnDrawFrame NULL == videoPlayer");
    }
    return nullptr;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_init(JNIEnv *env, jclass type,
                                               jstring videoPath_, jobject listener,
                                               jboolean userSoftDecode) {
    if (nullptr == videoPath_) {
        BZLogUtil::logE("videoPlayerInit nullptr == videoPath");
        return 0;
    }

    const char *videoPath = env->GetStringUTFChars(videoPath_, 0);
    VideoPlayerMethodInfo *videoPlayerMethodInfo = new VideoPlayerMethodInfo();

    if (nullptr != listener) {
        jobject listenerObj = env->NewGlobalRef(listener);
        videoPlayerMethodInfo->listenerObj = listenerObj;
        jclass listenerClass = env->GetObjectClass(listener);
        videoPlayerMethodInfo->onProgressChangedMethodId = env->GetMethodID(listenerClass,
                                                                            "onProgressChanged",
                                                                            "(F)V");
        videoPlayerMethodInfo->onVideoPlayCompletionMethodId = env->GetMethodID(listenerClass,
                                                                                "onVideoPlayCompletion",
                                                                                "(I)V");
        videoPlayerMethodInfo->onVideoInfoAvailableMethodId = env->GetMethodID(listenerClass,
                                                                               "onVideoInfoAvailable",
                                                                               "(IIIJF)V");
        env->DeleteLocalRef(listenerClass);
    }


    VideoPlayerInitParams *playerInitParams = new VideoPlayerInitParams();
    size_t len = strlen(videoPath) + 1;
    char *buffer = static_cast<char *>(malloc(len));
    memset(buffer, 0, len);
    memcpy(buffer, videoPath, len - 1);

    playerInitParams->videoPath = buffer;
    playerInitParams->userSoftDecode = userSoftDecode;
    playerInitParams->methodHandle = (int64_t) videoPlayerMethodInfo;
    playerInitParams->onVideoInfoAvailableCallBack = onVideoInfoAvailableCallBack;
    playerInitParams->progressCallBack = videoPlayerProgressCallBack;
    playerInitParams->onVideoPlayCompletionCallBack = onVideoPlayCompletionCallBack;

    VideoPlayer *videoPlayer = new VideoPlayer();
    int ret = videoPlayer->init(playerInitParams);
    env->ReleaseStringUTFChars(videoPath_, videoPath);
    if (ret < 0) {
        videoPlayer->release();
        return 0;
    }
    return reinterpret_cast<jlong>(videoPlayer);
}


extern "C" JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_onDrawFrame(JNIEnv *env, jclass type,
                                                      jlong nativeHandle,
                                                      jlong time) {
    if (nativeHandle != 0) {
        VideoPlayer *videoPlayer = reinterpret_cast<VideoPlayer *>(nativeHandle);
        return videoPlayer->onDraw(time);
    }
    return -1;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_release(JNIEnv *env, jclass type,
                                                  jlong nativeHandle) {
    if (nativeHandle == 0) {
        return -1;
    }
    int64_t methodHandle = 0;
    VideoPlayerInitParams *videoPlayerInitParams = getVideoPlayerInitParams(nativeHandle);
    if (nullptr != videoPlayerInitParams) {
        methodHandle = videoPlayerInitParams->methodHandle;
    }

    int ret = 0;
    VideoPlayer *videoPlayer = (VideoPlayer *) nativeHandle;
    ret = videoPlayer->release();
    delete (videoPlayer);

    JNIEnv *jniEnv = NULL;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    if (methodHandle != 0) {
        VideoPlayerMethodInfo *videoPlayerMethodInfo = (VideoPlayerMethodInfo *) methodHandle;
        BZLogUtil::logD("release videoPlayerMethodInfo=%lld", (int64_t) videoPlayerMethodInfo);

        if (NULL != videoPlayerMethodInfo->listenerObj) {
            jniEnv->DeleteGlobalRef(videoPlayerMethodInfo->listenerObj);
        }
        videoPlayerMethodInfo->listenerObj = NULL;
        delete (videoPlayerMethodInfo);
    }
    jniEnv = NULL;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
    return ret;
}
extern "C" JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_pause(JNIEnv *env, jclass type,
                                                jlong nativeHandle,
                                                jboolean isPause) {

    if (nativeHandle != 0) {
        VideoPlayer *videoPlayer = reinterpret_cast<VideoPlayer *>(nativeHandle);
        if (isPause) {
            videoPlayer->pause();
        } else {
            videoPlayer->start();
        }
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_seek(JNIEnv *env, jclass type,
                                               jlong nativeHandle,
                                               jlong videoTime) {

    if (nativeHandle != 0) {
        VideoPlayer *videoPlayer = reinterpret_cast<VideoPlayer *>(nativeHandle);
        videoPlayer->seek(videoTime);
    }
}


extern "C" JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_getCurrentAudioPts(JNIEnv *env,
                                                             jclass type,
                                                             jlong nativeHandle) {

    if (nativeHandle != 0) {
        VideoPlayer *videoPlayer = reinterpret_cast<VideoPlayer *>(nativeHandle);
        return videoPlayer->getCurrentAudioPts();
    }
    return -1;
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_onPause(JNIEnv *env, jclass type,
                                                  jlong nativeHandle) {
    if (nativeHandle != 0) {
        VideoPlayer *videoPlayer = reinterpret_cast<VideoPlayer *>(nativeHandle);
        videoPlayer->onPause();
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_setVolume(JNIEnv *env, jclass type,
                                                    jlong nativeHandle,
                                                    jfloat volume) {

    if (nativeHandle != 0) {
        VideoPlayer *videoPlayer = reinterpret_cast<VideoPlayer *>(nativeHandle);
        videoPlayer->setAudioVolume(volume);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_setPlayLoop(JNIEnv *env, jclass type,
                                                      jlong nativeHandle,
                                                      jboolean isLoop) {

    if (nativeHandle != 0) {
        VideoPlayer *videoPlayer = reinterpret_cast<VideoPlayer *>(nativeHandle);
        videoPlayer->setPlayLoop(isLoop);
    }
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_luoye_bzmedia_player_VideoPlayer_isPlaying(JNIEnv *env, jclass clazz,
                                                    jlong native_handle) {
    if (native_handle == 0) {
        return JNI_FALSE;
    }
    VideoPlayer *videoPlayer = reinterpret_cast<VideoPlayer *>(native_handle);
    return static_cast<jboolean>(videoPlayer->isPlaying());
}

