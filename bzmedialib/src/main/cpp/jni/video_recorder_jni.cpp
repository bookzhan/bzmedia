//
/**
 * Created by bookzhan on 2018-12-10 14:09.
 * 说明:
 */
//
#include <jni.h>
#include <common/BZLogUtil.h>
#include <recorder/VideoRecorder.h>


extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_recorder_VideoRecorder_initVideoRecorder(JNIEnv *env, jobject instance) {
    return reinterpret_cast<jlong>(new VideoRecorder());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_recorder_VideoRecorder_startRecord(JNIEnv *env, jobject instance,
                                                          jlong nativeHandle,
                                                          jobject videoRecordParamsObj) {
    if (nativeHandle == 0) {
        return -1;
    }
    VideoRecordParams videoRecordParams;

    jclass videoRecordParamsClass = env->GetObjectClass(videoRecordParamsObj);
    jstring output_path_ = (jstring) env->GetObjectField(videoRecordParamsObj,
                                                         env->GetFieldID(videoRecordParamsClass,
                                                                         "outputPath",
                                                                         "Ljava/lang/String;"));
    const char *output_path = NULL;
    if (NULL != output_path_) {
        output_path = env->GetStringUTFChars(output_path_, 0);
        videoRecordParams.output_path = output_path;
        BZLogUtil::logD("output_path=%s", output_path);
    } else {
        BZLogUtil::logE("output_path is NULL");
        return -1;
    }

    jint targetWidth = env->GetIntField(videoRecordParamsObj,
                                        env->GetFieldID(videoRecordParamsClass, "targetWidth",
                                                        "I"));
    videoRecordParams.targetWidth = targetWidth;
    BZLogUtil::logD("targetWidth=%d", targetWidth);

    jint targetHeight = env->GetIntField(videoRecordParamsObj,
                                         env->GetFieldID(videoRecordParamsClass, "targetHeight",
                                                         "I"));
    videoRecordParams.targetHeight = targetHeight;
    BZLogUtil::logD("targetHeight=%d", targetHeight);

    jint videoRate = env->GetIntField(videoRecordParamsObj,
                                      env->GetFieldID(videoRecordParamsClass, "videoFrameRate",
                                                      "I"));
    videoRecordParams.videoFrameRate = videoRate;
    BZLogUtil::logD("videoFrameRate=%d", videoRate);
    jint nbSamples = env->GetIntField(videoRecordParamsObj,
                                      env->GetFieldID(videoRecordParamsClass, "nbSamples", "I"));
    videoRecordParams.nbSamples = nbSamples;
    BZLogUtil::logD("nbSamples=%d", nbSamples);

    jint sampleRate = env->GetIntField(videoRecordParamsObj,
                                       env->GetFieldID(videoRecordParamsClass, "sampleRate", "I"));
    videoRecordParams.sampleRate = sampleRate;
    BZLogUtil::logD("sampleRate=%d", sampleRate);

    jint videoRotate = env->GetIntField(videoRecordParamsObj,
                                        env->GetFieldID(videoRecordParamsClass, "videoRotate",
                                                        "I"));
    videoRecordParams.videoRotate = videoRotate;
    BZLogUtil::logD("videoRotate=%d", videoRotate);

    jint pixelFormat = env->GetIntField(videoRecordParamsObj,
                                        env->GetFieldID(videoRecordParamsClass, "pixelFormat",
                                                        "I"));
    videoRecordParams.pixelFormat = pixelFormat;
    BZLogUtil::logD("pixelFormat=%d", pixelFormat);

    jboolean hasAudio = env->GetBooleanField(videoRecordParamsObj,
                                             env->GetFieldID(videoRecordParamsClass, "needAudio",
                                                             "Z"));
    videoRecordParams.hasAudio = hasAudio;
    BZLogUtil::logD("hasAudio=%d", hasAudio);

    jboolean synEncode = env->GetBooleanField(videoRecordParamsObj,
                                              env->GetFieldID(videoRecordParamsClass, "synEncode",
                                                              "Z"));
    videoRecordParams.synEncode = synEncode;
    BZLogUtil::logD("synEncode=%d", synEncode);

    jboolean needFlipVertical = env->GetBooleanField(videoRecordParamsObj,
                                                     env->GetFieldID(videoRecordParamsClass,
                                                                     "needFlipVertical",
                                                                     "Z"));
    videoRecordParams.needFlipVertical = needFlipVertical;
    BZLogUtil::logD("needFlipVertical=%d", needFlipVertical);


    jboolean allFrameIsKey = env->GetBooleanField(videoRecordParamsObj,
                                                  env->GetFieldID(videoRecordParamsClass,
                                                                  "allFrameIsKey",
                                                                  "Z"));
    videoRecordParams.allFrameIsKey = allFrameIsKey;
    BZLogUtil::logD("allFrameIsKey=%d", needFlipVertical);


    jlong bitRate = env->GetLongField(videoRecordParamsObj,
                                      env->GetFieldID(videoRecordParamsClass, "bitRate",
                                                      "J"));
    videoRecordParams.bit_rate = bitRate;
    BZLogUtil::logD("bitRate=%lld", bitRate);

    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    int ret = videoRecorder->startRecord(videoRecordParams);

    if (NULL != output_path_ && NULL != output_path)
        env->ReleaseStringUTFChars(output_path_, output_path);

    env->DeleteLocalRef(videoRecordParamsObj);
    env->DeleteLocalRef(videoRecordParamsClass);
    return ret;
}


extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_recorder_VideoRecorder_addAudioData(JNIEnv *env, jobject instance,
                                                           jlong nativeHandle,
                                                           jbyteArray data_, jint dataLength,
                                                           jlong audioPts) {
    if (nativeHandle == 0 || nullptr == data_) {
        return -1;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    jbyte *data = env->GetByteArrayElements(data_, NULL);

    int64_t ret = videoRecorder->addAudioData(reinterpret_cast<unsigned char *>(data), dataLength,
                                              audioPts);

    env->ReleaseByteArrayElements(data_, data, 0);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_recorder_VideoRecorder_updateTexture(JNIEnv *env, jobject instance,
                                                            jlong nativeHandle,
                                                            jint textureId,
                                                            jlong videoPts) {
    if (nativeHandle == 0) {
        return -1;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    return videoRecorder->updateTexture(textureId, videoPts);

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_recorder_VideoRecorder_setStopRecordFlag(JNIEnv *env, jobject instance,
                                                                jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    videoRecorder->setStopRecordFlag();
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_recorder_VideoRecorder_stopRecord(JNIEnv *env, jobject instance,
                                                         jlong nativeHandle) {

    if (nativeHandle == 0) {
        return -1;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    return videoRecorder->stopRecord();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_recorder_VideoRecorder_releaseRecorder(JNIEnv *env, jobject instance,
                                                              jlong nativeHandle) {
    if (nativeHandle == 0) {
        return;
    }
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(nativeHandle);
    delete (videoRecorder);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_luoye_bzmedia_recorder_VideoRecorder_addYUV420Data(JNIEnv *env, jclass clazz,
                                                            jlong native_handle, jbyteArray data_,
                                                            jlong pts) {
    if (native_handle == 0) {
        return -1;
    }
    unsigned char *buffer = (unsigned char *) env->GetByteArrayElements(data_, NULL);
    VideoRecorder *videoRecorder = reinterpret_cast<VideoRecorder *>(native_handle);
    long ret = videoRecorder->addVideoData(buffer, pts);
    env->ReleaseByteArrayElements(data_, reinterpret_cast<jbyte *>(buffer), 0);
    return ret;
}

