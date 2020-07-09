
/**
 * Created by zhandalin on 2017-06-14 15:57.
 * 说明:
 */

#ifndef BZFFMPEG_ADDBACKGROUNDMUSIC_H
#define BZFFMPEG_ADDBACKGROUNDMUSIC_H

#include <string>
#include <stdio.h>
#include <list>

extern "C" {
#include <include/libavformat/avformat.h>
#include "../common/ffmpeg_cmd.h"
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/audio_fifo.h>
#include <libavcodec/avcodec.h>
};

using namespace std;

class BackgroundMusicUtil {
public:
    int startAddBackgroundMusic(const char *inputPath, const char *outputPath,
                                const char *musicPath, float srcMusicVolume,
                                float bgMusicVolume,
                                OnActionListener *onActionListener, bool needFade = false);

    int mixMusic(const char *inputMusicPath_1, const char *inputMusicPath_2, const char *outputPath,
                 float musicVolume_1,
                 float musicVolume_2);

    int replaceBackgroundMusic(const char *videoPath, const char *musicPath, const char *outputPath,
                               OnActionListener *onActionListener);

    int
    replaceBackgroundMusicOnly(const char *videoPath, const char *musicPath, const char *outputPath,
                               OnActionListener *onActionListener);

    //目标路径,用来存临时文件
    int
    alignmentMusic2Time(const char *musicPath, const char *targetOutputPath, int64_t time,
                        bool needFade);

    //对传入的视频或者音频,前一段时间的音频插入静音,并保存成音频
    int
    delayMusic(const char *mediaPath, const char *outPath, int64_t startTime,
               int64_t seekStartTime = 0, int64_t seekEndTime = 0);

    int mixMusic(list<const char *> *musicList, const char *outPath,OnActionListener *onActionListener);

    //对音频做淡出处理
    int fadeMusic(const char *musicPath, const char *outputPath);

private:

    AVFilterContext *buffersink_ctx = NULL;
    AVFilterContext *buffersrc_ctx = NULL;
    AVFilterGraph *filter_graph = NULL;

    int initAudioFilters(const char *filters_descr, AVCodecContext *avCodecContext);

};


#endif //BZFFMPEG_ADDBACKGROUNDMUSIC_H
