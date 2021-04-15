//
/**
 * Created by bookzhan on 2019-02-18 10:16.
 * 说明:
 */
//

#include <common/JvmManager.h>
#include "OnActionListener.h"

OnActionListener::OnActionListener(jobject obj) {
    if (nullptr != obj) {
        JNIEnv *jniEnv = nullptr;
        bool needDetach = JvmManager::getJNIEnv(&jniEnv);
        jclass actionClass = jniEnv->GetObjectClass(obj);
        actionObj = jniEnv->NewGlobalRef(obj);
        progressMID = jniEnv->GetMethodID(actionClass, "progress", "(F)V");
        failMID = jniEnv->GetMethodID(actionClass, "fail", "()V");
        successMID = jniEnv->GetMethodID(actionClass, "success", "()V");

        jniEnv->DeleteLocalRef(actionClass);
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
    }
}

void OnActionListener::progress(float progress) {
    if (nullptr == actionObj)return;
    if (progress > 1)progress = 1;
    if (progress < 0)progress = 0;
    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    jniEnv->CallVoidMethod(actionObj, progressMID, progress);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void OnActionListener::fail() {
    if (nullptr == actionObj)return;

    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    jniEnv->CallVoidMethod(actionObj, failMID);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

void OnActionListener::success() {
    if (nullptr == actionObj)return;

    JNIEnv *jniEnv = nullptr;
    bool needDetach = JvmManager::getJNIEnv(&jniEnv);

    jniEnv->CallVoidMethod(actionObj, successMID);

    jniEnv = nullptr;
    if (needDetach)
        JvmManager::getJavaVM()->DetachCurrentThread();
}

OnActionListener::~OnActionListener() {
    if (nullptr != actionObj) {
        JNIEnv *jniEnv = nullptr;
        bool needDetach = JvmManager::getJNIEnv(&jniEnv);

        jniEnv->DeleteGlobalRef(actionObj);
        actionObj = nullptr;
        jniEnv = nullptr;
        if (needDetach)
            JvmManager::getJavaVM()->DetachCurrentThread();
    }
}

void OnActionListener::progressCallBack(int64_t handle, int what, float progress) {
    if (handle != 0) {
        auto *onActionListener = reinterpret_cast<OnActionListener *>(handle);
        onActionListener->progress(progress);
    }
}
