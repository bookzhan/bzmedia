//
/**
 * Created by bookzhan on 2018-01-11 17:12.
 * 说明:
 */
//

#ifndef BZFFMPEG_VIDEOTRANSCODEPARAMS_H
#define BZFFMPEG_VIDEOTRANSCODEPARAMS_H

#include <clocale>

typedef struct VideoTranscodeParams {
    const char *inputPath = NULL;
    const char *outputPath = NULL;
    int gopSize = 30;

    int64_t startTime = -1;
    int64_t endTime = -1;

    //优先级最高
    int targetWidth = -1;
    int targetHeight = -1;

    //帧率控制
    int frameRate = -1;
} VideoTransCodeParams;

#endif //BZFFMPEG_VIDEOTRANSCODEPARAMS_H
