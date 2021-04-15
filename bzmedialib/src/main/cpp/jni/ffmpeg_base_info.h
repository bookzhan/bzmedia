//
/**
 * Created by bookzhan on 2020-05-25 17:00.
 *description:
 */
//

#ifndef BZMEDIA_FFMPEG_BASE_INFO_H
#define BZMEDIA_FFMPEG_BASE_INFO_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
}

void printFFmpegBaseInfo();

int getFFmpegConfigure(char *info);

int getFFmpegSupportProtocol(char *info);

int getFFmpegSupportAVFormat(char *info);

int getFFmpegSupportAVCodec(char *info);

int getFFmpegSupportAVFilter(char *info);

int testLib();

#endif //BZMEDIA_FFMPEG_BASE_INFO_H
