//
/**
 * Created by zhandalin on 2018-07-11 18:03.
 * 说明:
 */
//

#ifndef BZMEDIA_CALLBACKMETHODIDINFO_H
#define BZMEDIA_CALLBACKMETHODIDINFO_H

#include <jni.h>

typedef struct CallbackMethodIDInfo {
    jobject callbackObject = nullptr;
    jmethodID callbackMethodId = nullptr;
} CallbackMethodIDInfo;

typedef struct CommonProgressMethodIDInfo {
    jobject callbackObject = nullptr;
    jmethodID progressMethodId = nullptr;
    jmethodID failMethodId = nullptr;
    jmethodID successMethodId = nullptr;
} CommonProgressMethodIDInfo;


#endif //BZMEDIA_CALLBACKMETHODIDINFO_H
