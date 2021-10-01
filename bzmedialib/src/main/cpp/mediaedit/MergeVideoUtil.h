/**
 * Created by bookzhan on 2017-04-17 11:18.
 * 说明:
 */

#ifndef BZFFMPEG_MERGEVIDEOUTIL_H
#define BZFFMPEG_MERGEVIDEOUTIL_H

extern "C" {
#include <libavformat/avformat.h>
#include "../recorder/OutputStream.h"
#include <libswresample/swresample.h>
};

#include "../common/bzcommon.h"
#include <thread>
#include <mutex>
#include <jni/OnActionListener.h>

using namespace std;

class MergeVideoUtil {
public:
    int startMergeVideo(char **videoPaths, int length, const char *output_path,
                        OnActionListener *onActionListener);

    int startMergeVideo(char **videoPaths, int length, const char *output_path, bool needVideo,
                        bool needAudio,
                        OnActionListener *onActionListener);

private:
    char **videoPaths = nullptr;
    int length = 0;
    char *output_path = nullptr;
    float progress = 0;
    OnActionListener *onActionListener = nullptr;

    int64_t stream0Pts = 0, stream1Pts = 0;

    int64_t stream0Dts = 0, stream1Dts = 0;

    AVFormatContext *avFormatContext = nullptr;

    int mergeVideo();

    int releaseResource();

    bool needVideo = true;
    bool needAudio = true;
};


#endif //BZFFMPEG_MERGEVIDEOUTIL_H
