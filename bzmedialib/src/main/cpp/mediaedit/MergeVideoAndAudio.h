//
/**
 * Created by bookzhan on 2019-01-14 10:10.
 * 说明:
 */
//

#ifndef BZMEDIA_MERGEVIDEOANDAUDIO_H
#define BZMEDIA_MERGEVIDEOANDAUDIO_H

#include "VideoUtil.h"

class MergeVideoAndAudio {
public:
    int
    startMergeVideoAndAudio(const char *videoPath, const char *audioPath, const char *outputPath);

private:
    int openOutputFile(AVFormatContext **out_fmt_ctx,AVStream *videoInStream, AVStream *audioInStream,const char *outputPath);
};


#endif //BZMEDIA_MERGEVIDEOANDAUDIO_H
