#include <jni.h>
#include <string>
#include <common/BZLogUtil.h>
#include "ffmpeg_base_info.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
const char *TAG = "bz_";

void log_call_back(void *ptr, int level, const char *fmt, va_list vl) {
    //Custom log
    if (level == 3) {
        __android_log_vprint(ANDROID_LOG_ERROR, TAG, fmt, vl);
    } else if (level == 2) {
        __android_log_vprint(ANDROID_LOG_DEBUG, TAG, fmt, vl);
    } else if (level == 1) {
        __android_log_vprint(ANDROID_LOG_VERBOSE, TAG, fmt, vl);
    } else {
        if (level <= 16) {//logs from ffmpeg
            __android_log_vprint(ANDROID_LOG_ERROR, TAG, fmt, vl);
        } else {
//            __android_log_vprint(ANDROID_LOG_VERBOSE, TAG, fmt, vl);
        }
    }
}


extern "C"
JNIEXPORT jint JNICALL
Java_bz_luoye_bzmedia_BZMedia_initNative(JNIEnv *env, jclass clazz, jobject context,
                                         jboolean is_debug, jint sdk_int) {
    BZLogUtil::enableLog = is_debug;
    __android_log_write(ANDROID_LOG_DEBUG, BZLOG_TAG, "welcome bzmedia ^_^");
    if (is_debug) {
        av_log_set_callback(log_call_back);
    }
    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_bz_luoye_bzmedia_BZMedia_test(JNIEnv *env, jclass clazz) {
    printFFmpegBaseInfo();
    return 0;
}