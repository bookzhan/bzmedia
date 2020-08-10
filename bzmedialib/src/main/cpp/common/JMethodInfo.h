//
/**
 * Created by zhandalin on 2018-11-14 14:15.
 * 说明:
 */
//

#ifndef BZMEDIA_JMETHODINFO_H
#define BZMEDIA_JMETHODINFO_H

#include <jni.h>

typedef struct JMethodInfo {
    jobject obj = nullptr;
    jmethodID methodID = nullptr;
} JMethodInfo;

#endif //BZMEDIA_JMETHODINFO_H
