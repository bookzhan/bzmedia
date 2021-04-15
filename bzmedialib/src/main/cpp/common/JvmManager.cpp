//
/**
 * Created by bookzhan on 2018-04-10 12:23.
 * 说明:
 */
//
#include <android/log.h>
#include "JvmManager.h"

extern "C" {
#include <libavcodec/ffmpeg_jni.h>
}
JavaVM *bzJavaVM = nullptr;

int32_t JvmManager::JNI_VERSION = JNI_VERSION_1_6;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    bzJavaVM = vm;
    __android_log_print(ANDROID_LOG_DEBUG, "bz_", "JNI_OnLoad success");
    av_jni_set_java_vm(vm, NULL);
    if (vm->GetEnv(reinterpret_cast<void **>(&vm), JNI_VERSION_1_6) != JNI_OK) {
        JvmManager::JNI_VERSION = JNI_VERSION_1_4;
        return JNI_VERSION_1_4;
    }
    return JNI_VERSION_1_6;
}

bool JvmManager::getJNIEnv(JNIEnv **pJNIEnv) {
    if (NULL == bzJavaVM) {
        return false;
    }
    bzJavaVM->GetEnv((void **) pJNIEnv, JNI_VERSION);
    if (NULL != *pJNIEnv) {
        return false;
    } else {
        bzJavaVM->AttachCurrentThread(pJNIEnv, NULL);
        return true;
    }
}

JavaVM *JvmManager::getJavaVM() {
    return bzJavaVM;
}
