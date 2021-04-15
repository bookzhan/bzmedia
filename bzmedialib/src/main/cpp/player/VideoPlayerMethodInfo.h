//
/**
 * Created by bookzhan on 2021-05-11 09:36.
 *description:
 */
//
#include <jni.h>
#ifndef BZMEDIA_VIDEOPLAYERMETHODINFO_H
#define BZMEDIA_VIDEOPLAYERMETHODINFO_H

typedef struct VideoPlayerMethodInfo {
    jobject listenerObj = NULL;
    jmethodID onProgressChangedMethodId = NULL;
    jmethodID onVideoPlayCompletionMethodId = NULL;
    jmethodID onVideoInfoAvailableMethodId = NULL;
    jbyteArray pcmjbyteArray = NULL;
} VideoPlayerMethodInfo;

#endif //BZMEDIA_VIDEOPLAYERMETHODINFO_H
