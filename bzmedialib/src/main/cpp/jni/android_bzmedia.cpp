#include <jni.h>
#include <string>
#include <common/BZLogUtil.h>
#include "ffmpeg_base_info.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


extern "C"
JNIEXPORT jint JNICALL
Java_bz_luoye_bzmedia_BZMedia_initNative(JNIEnv *env, jclass clazz, jobject context,
                                         jboolean is_debug, jint sdk_int) {
    BZLogUtil::enableLog = is_debug;
    BZLogUtil::logD("welcome bzmedia ^_^");
    printFFmpegBaseInfo();

    return 0;
}