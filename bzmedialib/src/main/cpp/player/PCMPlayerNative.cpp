//
/**
 * Created by bookzhan on 2018-11-28 16:36.
 * 说明:
 */
//

#include <cstdint>
#include <clocale>
#include <common/JvmManager.h>
#include "PCMPlayerNative.h"

jclass PCMPlayerNative::pcmPlayerClass = nullptr;

PCMPlayerNative::PCMPlayerNative() {
    JNIEnv *env = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&env);
    if (nullptr != env && nullptr != pcmPlayerClass) {
        jmethodID constructorMid = env->GetMethodID(pcmPlayerClass, "<init>", "()V");
        jobject pcmPlayerObjTemp = env->NewObject(pcmPlayerClass, constructorMid);
        pcmPlayerObj = env->NewGlobalRef(pcmPlayerObjTemp);
        env->DeleteLocalRef(pcmPlayerObjTemp);

        initMethodId = env->GetMethodID(pcmPlayerClass,
                                        "init",
                                        "(II)V");

        onPCMDataAvailableMethodId = env->GetMethodID(pcmPlayerClass,
                                                      "onPCMDataAvailable",
                                                      "([BI)V");
        setVideoPlayerVolumeMethodId = env->GetMethodID(pcmPlayerClass,
                                                        "setVideoPlayerVolume",
                                                        "(F)V");

        stopAudioTrackMethodId = env->GetMethodID(pcmPlayerClass, "stopAudioTrack",
                                                  "()V");
        pauseMethodId = env->GetMethodID(pcmPlayerClass, "pause",
                                         "()V");
        startMethodId = env->GetMethodID(pcmPlayerClass, "start",
                                         "()V");
        env = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
    }
}

void PCMPlayerNative::onPCMDataAvailable(const char *pcmData, int length) {
    if (nullptr == pcmPlayerClass) {
        return;
    }
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == jniEnv) {
        return;
    }
    if (nullptr == pcmjbyteArray) {
        jbyteArray pcmjbyteArrayTemp = jniEnv->NewByteArray(length);
        pcmjbyteArray = (jbyteArray) jniEnv->NewGlobalRef(pcmjbyteArrayTemp);
        jniEnv->DeleteLocalRef(pcmjbyteArrayTemp);
    }
    jniEnv->SetByteArrayRegion(pcmjbyteArray, 0, length,
                               (const jbyte *) pcmData);


    jniEnv->CallVoidMethod(pcmPlayerObj,
                           onPCMDataAvailableMethodId,
                           pcmjbyteArray,
                           length);
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void PCMPlayerNative::setVideoPlayerVolume(float volume) {
    if (nullptr == pcmPlayerClass) {
        return;
    }
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == jniEnv) {
        return;
    }
    jniEnv->CallVoidMethod(pcmPlayerObj,
                           setVideoPlayerVolumeMethodId, volume);
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void PCMPlayerNative::stopAudioTrack() {
    if (nullptr == pcmPlayerClass) {
        return;
    }
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == jniEnv) {
        return;
    }

    jniEnv->CallVoidMethod(pcmPlayerObj,
                           stopAudioTrackMethodId);
    if (nullptr != pcmjbyteArray) {
        jniEnv->DeleteGlobalRef(pcmjbyteArray);
        pcmjbyteArray = nullptr;
    }
    if (nullptr != pcmPlayerObj) {
        jniEnv->DeleteGlobalRef(pcmPlayerObj);
        pcmPlayerObj = nullptr;
    }
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void PCMPlayerNative::pause() {
    if (nullptr == pcmPlayerClass) {
        return;
    }
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == jniEnv) {
        return;
    }
    jniEnv->CallVoidMethod(pcmPlayerObj,
                           pauseMethodId);
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void PCMPlayerNative::start() {
    if (nullptr == pcmPlayerClass) {
        return;
    }
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == jniEnv) {
        return;
    }
    jniEnv->CallVoidMethod(pcmPlayerObj,
                           startMethodId);
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void PCMPlayerNative::init(int sampleRateInHz, int channelCount) {
    if (nullptr == pcmPlayerClass) {
        return;
    }
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);
    if (nullptr == jniEnv) {
        return;
    }
    jniEnv->CallVoidMethod(pcmPlayerObj,
                           initMethodId, sampleRateInHz, channelCount);
    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

