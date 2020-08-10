//
/**
 * Created by zhandalin on 2018-11-28 16:36.
 * 说明:
 */
//

#ifndef SLIDESHOWSDK_PCMPLAYERNATIVE_H
#define SLIDESHOWSDK_PCMPLAYERNATIVE_H

#include <jni.h>

class PCMPlayerNative {
public:
    PCMPlayerNative();

    void onPCMDataAvailable(const char *pcmData, int length);

    void setVideoPlayerVolume(float volume);

    void pause();

    void start();

    void stopAudioTrack();

    static jclass pcmPlayerClass;
private:
    jobject pcmPlayerObj = nullptr;
    jmethodID onPCMDataAvailableMethodId = nullptr;
    jmethodID setVideoPlayerVolumeMethodId = nullptr;
    jmethodID stopAudioTrackMethodId = nullptr;
    jmethodID pauseMethodId = nullptr;
    jmethodID startMethodId = nullptr;
    jbyteArray pcmjbyteArray = nullptr;
};


#endif //SLIDESHOWSDK_PCMPLAYERNATIVE_H
