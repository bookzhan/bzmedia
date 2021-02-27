/**
 * Created by zhandalin on 2017-03-29 17:55.
 * 说明:
 */

#ifndef BZFFMPEG_VIDEO_RECORDER_OBJ_H
#define BZFFMPEG_VIDEO_RECORDER_OBJ_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <inttypes.h>
#include <libavutil/avassert.h>
#include <libavutil/audio_fifo.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
};

#include "OutputStream.h"
#include <mutex>
#include <queue>
#include <deque>
#include <thread>
#include <atomic>
#include <math.h>
#include <list>
#include <string>
#include "../common/bz_time.h"
#include "VideoRecordParams.h"
#include <bean/PixelFormat.h>
#include <glutils/TextureConvertYUVUtil.h>

#define H264_NALU_TYPE_NON_IDR_PICTURE                                  1
#define H264_NALU_TYPE_IDR_PICTURE                                      5
#define H264_NALU_TYPE_SEQUENCE_PARAMETER_SET                           7
#define H264_NALU_TYPE_PICTURE_PARAMETER_SET                            8
#define H264_NALU_TYPE_SEI                                            6

#define is_start_code(code)    (((code) & 0x0ffffff) == 0x01)

typedef unsigned char byte;

using namespace std;


class VideoRecorder {
public:

    VideoRecorder();

    int startRecord(VideoRecordParams videoRecordParams);

    int64_t addVideoData(unsigned char *data, int64_t videoPts = -1);

    int64_t addVideoData(AVFrame *avFrameData, int64_t videoPts = -1);

    int64_t addAudioData(unsigned char *data, int frameSize, int64_t audioPts = -1);

    void setStopRecordFlag();

    int stopRecord();

    int64_t addVideoPacketData(const unsigned char *avPacketData, int64_t size, int64_t pts);


    int updateTexture(int textureId, int64_t videoPts = -1);

    int getVideoOutputWidth();

    int getVideoOutputHeight();

    //获取录制的总时间,以音频时间戳为准
    int64_t getRecordTime();

    ~VideoRecorder();

private:
    //将会显示详细的日志
    bool showDebugLog = false;

    int srcWidth, srcHeight;
    //录制的总时间,单位毫秒,以音频的时间为依据计算
    int64_t recorderTime;

    atomic_bool isStopRecorder = {true};
    atomic_bool isAddAudioData = {false};
    atomic_bool isAddVideoData = {false};
    mutex addAudioDataLock;
    mutex addVideoDataLock;

    AVFormatContext *avFormatContext = NULL;
    BZOutputStream *video_st = NULL;
    BZOutputStream *audio_st = NULL;
    int64_t videoFrameCount = 0;
    int64_t samples_count = 0;
    int64_t videoCacheFramePts = 0;
    int64_t logIndex = 0;


    int targetWidth = 0, targetHeight = 0,
            videoRate = 24, y_size = 0, nbSamples = 1024, sampleRate = 44100, videoRotate = 90, pixelFormat = 0;
    //视频是否都是关键帧
    bool allFrameIsKey = false;
    //比特率
    int64_t bit_rate = 0;

    //外部传入的时间戳
    std::list<int64_t> *videoPts = nullptr;
    std::list<int64_t> *audioPts = nullptr;

    std::list<int64_t> *videoPtsBuffer = nullptr;

    //视频是否同步编码
    bool synEncode = false;
    bool avPacketFromMediaCodec = false;

    mutex *videoWriteMutex = NULL;
    string *output_path = NULL;

    int yuvBufferTotalCount = 0;
    int updateTextureCount = 0;

    //视频帧缓存
    deque<AVFrame *> avFrameDeque;
    //avFrameList 存取锁
    mutex mutexAVFrameDeque;

    bool encodeThreadIsRunning = false;

    //用来统计用的
    int64_t encodeTotalTime = 0;
    int64_t recorderStartTime = 0;

    int64_t addVideoPacketDataLatsPts = -1;

    TextureConvertYUVUtil *textureConvertYUVUtil = NULL;

    AVAudioFifo *audioFifo = NULL;

    unsigned char **audioData = static_cast<unsigned char **>(malloc(sizeof(unsigned char *) * 1));

    int endRecordAndReleaseResource();

    //flush编码器里面的buffer
    void flushBuffer();

    int closeStream(BZOutputStream *stream);

    int addStream(BZOutputStream *stream, AVMediaType mediaType, AVCodecID codec_id);

    int openVideo(BZOutputStream *stream, const char *extraFilterParam);

    int openAudio(BZOutputStream *stream);

    AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);

    AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                               uint64_t channel_layout,
                               int sample_rate, int nb_samples);

    int encodeFrame(AVFrame *avFrame, int64_t videoPts);

    int writeVideoPacket(AVPacket *avPacket, int got_picture, int64_t videoPts);

    int writeAudioFrame(AVFrame *frame, int64_t audioPts);

    void encodeThread();

    bool isWriteHeaderSuccess = false;
    unsigned char *bufferHeader = nullptr;
    int64_t headerSize = 0;
};


#endif //BZFFMPEG_VIDEO_RECORDER_OBJ_H
