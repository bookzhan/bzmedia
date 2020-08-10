//
/**
 * Created by zhandalin on 2020-08-10 17:33.
 *description:
 */
//

#include <jni.h>
#include <common/BZLogUtil.h>
#include <common/JMethodInfo.h>
#include <player/AudioPlayer.h>
#include <common/JvmManager.h>

void
onAudioPlayerProgressCallBack(int64_t methodHandle, float progress) {
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    JvmManager::getJavaVM()->AttachCurrentThread(&jniEnv, nullptr);
    if (methodHandle == 0 || nullptr == jniEnv) {
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
        return;
    }
    auto *jMethodInfo = (JMethodInfo *) methodHandle;

    jniEnv->CallVoidMethod(jMethodInfo->obj, jMethodInfo->methodID, progress);
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_init(JNIEnv *env, jobject thiz, jstring audio_path_,
                                                     jobject on_progress_changed_listener) {
    if (nullptr == audio_path_) {
        BZLogUtil::logE("nullptr==audio_path");
        return 0;
    }
    auto *jMethodInfo = new JMethodInfo();
    jMethodInfo->obj = env->NewGlobalRef(on_progress_changed_listener);
    jclass progressChangedClass = env->GetObjectClass(on_progress_changed_listener);
    jMethodInfo->methodID = env->GetMethodID(progressChangedClass,
                                             "onProgressChanged",
                                             "(F)V");
    auto *audioPlayer = new AudioPlayer();
    const char *audio_path = env->GetStringUTFChars(audio_path_, 0);
    int ret = audioPlayer->init(audio_path, reinterpret_cast<int64_t>(jMethodInfo),
                                onAudioPlayerProgressCallBack);
    if (ret < 0) {
        delete (audioPlayer);
        return 0;
    }
    env->ReleaseStringUTFChars(audio_path_, audio_path);
    return reinterpret_cast<jlong>(audioPlayer);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_seek(JNIEnv *env, jobject thiz, jlong handle,
                                                     jlong time) {
    if (handle == 0) {
        return -1;
    }
    auto *audioPlayer = reinterpret_cast<AudioPlayer *>(handle);
    return audioPlayer->seek(time);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_setPlayLoop(JNIEnv *env, jobject thiz, jlong handle,
                                                            jboolean is_loop) {
    if (handle == 0) {
        return -1;
    }
    auto *audioPlayer = reinterpret_cast<AudioPlayer *>(handle);
    return audioPlayer->setPlayLoop(is_loop);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_setAudioVolume(JNIEnv *env, jobject thiz,
                                                               jlong handle, jfloat volume) {
    if (handle == 0) {
        return;
    }
    auto *audioPlayer = reinterpret_cast<AudioPlayer *>(handle);
    return audioPlayer->setAudioVolume(volume);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_getDuration(JNIEnv *env, jobject thiz,
                                                            jlong handle) {
    if (handle == 0) {
        return -1;
    }
    auto *audioPlayer = reinterpret_cast<AudioPlayer *>(handle);
    return audioPlayer->getDuration();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_pause(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) {
        return -1;
    }
    auto *audioPlayer = reinterpret_cast<AudioPlayer *>(handle);
    return audioPlayer->pause();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_start(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) {
        return -1;
    }
    auto *audioPlayer = reinterpret_cast<AudioPlayer *>(handle);
    return audioPlayer->start();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_stop(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) {
        return -1;
    }
    auto *audioPlayer = reinterpret_cast<AudioPlayer *>(handle);
    return audioPlayer->stop();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_player_AudioNativePlayer_release(JNIEnv *env, jobject thiz, jlong handle) {
    if (handle == 0) {
        return -1;
    }
    auto *audioPlayer = reinterpret_cast<AudioPlayer *>(handle);
    int ret = audioPlayer->release();
    delete (audioPlayer);
    return ret;
}