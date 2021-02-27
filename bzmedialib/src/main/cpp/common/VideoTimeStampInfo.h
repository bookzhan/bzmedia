//
/**
 * Created by zhandalin on 2017-09-29 11:15.
 * 说明:
 */
//

#ifndef BZFFMPEG_VIDEOTIMESTAMPINFO_H
#define BZFFMPEG_VIDEOTIMESTAMPINFO_H

#include <cstdint>

typedef struct VideoTimeStampInfo {
    int64_t pts;
    int64_t dts;
    int64_t duration;
} VideoTimeStampInfo;


#endif //BZFFMPEG_VIDEOTIMESTAMPINFO_H
