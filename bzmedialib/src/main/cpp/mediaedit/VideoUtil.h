
/**
 * Created by zhandalin on 2017-08-09 20:46.
 * 说明:
 */


#ifndef BZFFMPEG_VIDEOUTIL_H
#define BZFFMPEG_VIDEOUTIL_H

extern "C" {
#include <libavformat/avformat.h>
#include <include/libswscale/swscale.h>
};

#include <list>
#include <common/BZLogUtil.h>
#include <jni/OnActionListener.h>

using namespace std;

class VideoUtil {
public:
    static int getVideoInfo(const char *videoPath, long handle, void (*sendMediaInfoCallBack)(
            long, int, int));

    static int getVideoWidth(const char *videoPath);

    static AVFrame *allocVideoFrame(enum AVPixelFormat pix_fmt, int width, int height);

    static AVFrame *allocAudioFrame(enum AVSampleFormat sample_fmt,
                                    uint64_t channel_layout,
                                    int sample_rate, int nb_samples);

    static int getVideoHeight(const char *videoPath);

    static int getVideoRotate(const char *videoPath);

    static int printVideoTimeStamp(const char *videoPath);

    static int printVideoFrameInfo(const char *videoPath);

    static int64_t getMediaDuration(const char *mediaPath);

    static int openInputFile(const char *filename, AVFormatContext **in_fmt_ctx);

    static int openInputFileForSoft(const char *filename, AVFormatContext **in_fmt_ctx,
                                    bool openVideoDecode = true,
                                    bool openAudioDecode = true);

    //打开输入文件,同时探测解码器是否可用,并打开对应的编码器
    static int
    openInputFileForHard(const char *filename, AVFormatContext **in_fmt_ctx, bool openVideoDecode,
                         bool openAudioDecode);

    static int openAVCodecContext(AVStream *inStream);


    //根据一个已有的视频信息打开一个视频
    static int openOutputFile(AVFormatContext *in_fmt_ctx, AVFormatContext **out_fmt_ctx,
                              const char *output_path, bool needAudio = true);

    static int64_t getBitRate(int videoWidth, int videoHeight, bool allFrameIsKey);

    static bool videoPacketSort(AVPacket *avPacket_1, AVPacket *avPacket_2) {
        if (NULL == avPacket_1 || NULL == avPacket_2)return false;
        return avPacket_1->dts < avPacket_2->dts;
    }

    static bool videoPtsSort(int64_t pts_1, int64_t pts_2) {
        return pts_1 < pts_2;
    }

    static int getVideoPts(const char *videoPath, list<int64_t> *ptsList);

    static float getVideoAverageDuration(const char *videoPath);

    static bool videoIsSupport(const char *videoPath, bool isSoftDecode);

    static bool audioIsSupport(const char *audioPath);

    static int
    clipAudio(const char *audioPath, const char *outPath, int64_t startTime, int64_t endTime);

    static int
    clipVideo(const char *videoPath, const char *outPath, int64_t startTime, int64_t endTime);

    static bool hasAudio(const char *path);

    static bool hasVideo(const char *path);

    static int
    mixAudios2Video(const char *outPath, const char *videoStreamInputPath, char **audioPaths,
                    size_t audioCount, OnActionListener *onActionListener);

private:
    static bool str_ends_with(const char *str, const char *suffix) {
        if (str == NULL || suffix == NULL)
            return 0;
        size_t str_len = strlen(str);
        size_t suffix_len = strlen(suffix);
        if (suffix_len > str_len)
            return 0;
        return 0 == strncmp(str + str_len - suffix_len, suffix, suffix_len);
    }
};


#endif //BZFFMPEG_VIDEOUTIL_H
