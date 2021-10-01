/**
 * Created by bookzhan on 2017-04-17 11:18.
 * 说明:
 */

#include "MergeVideoUtil.h"
#include "../mediaedit/VideoUtil.h"

int MergeVideoUtil::startMergeVideo(char **videoPaths, int length, const char *output_path,
                                    bool needVideo, bool needAudio,
                                    OnActionListener *onActionListener) {
    this->needVideo = needVideo;
    this->needAudio = needAudio;
    this->startMergeVideo(videoPaths, length, output_path, onActionListener);
    return 0;
}

int MergeVideoUtil::startMergeVideo(char **videoPaths, int length, const char *output_path,
                                    OnActionListener *onActionListener) {
    if (NULL == videoPaths || length <= 0 || NULL == output_path) {
        BZLogUtil::logE("params is error");
        return -1;
    }
    BZLogUtil::logD("startMergeVideo needVideo=%d needAudio=%d output_path=%s", needVideo,
                    needAudio, output_path);
    this->videoPaths = videoPaths;
    this->length = length;
    this->onActionListener = onActionListener;

    size_t len = strlen(output_path) + 1;
    this->output_path = static_cast<char *>(malloc(len));
    memset(this->output_path, 0, len);
    sprintf(this->output_path, "%s", output_path);

    mergeVideo();
    return 0;
}

int MergeVideoUtil::mergeVideo() {
    if (NULL == this->videoPaths[0] || this->length <= 0 || NULL == this->output_path) {
        BZLogUtil::logE("mergeVideo params is error");
        return -1;
    }
    int ret = 0, videoIndex = 0, audioIndex = 1;
    AVPacket pkt;
    AVStream *in_stream, *out_stream;
    AVFormatContext *ifmt_ctx = NULL;
    bool outPutFileOpen = false;

    for (int i = 0; i < length; ++i) {
        ifmt_ctx = NULL;
        BZLogUtil::logD("handle path=%s", videoPaths[i]);
        //加强兼容新,防止一个文件打开失败,其它的没法写入
        if (avformat_open_input(&ifmt_ctx, videoPaths[i], 0, 0) < 0) {
            BZLogUtil::logE("Could not open input file '%s'", videoPaths[i]);
            continue;
        }

        if (avformat_find_stream_info(ifmt_ctx, 0) < 0) {
            BZLogUtil::logE("Failed to retrieve input stream information");
            continue;
        }

        if (!outPutFileOpen) {
            outPutFileOpen = true;
            ret = VideoUtil::openOutputFile(ifmt_ctx, &avFormatContext, this->output_path);
            if (ret < 0 || NULL == avFormatContext) {
                BZLogUtil::logE("openOutputFile fail");
                if (NULL != onActionListener)
                    onActionListener->fail();
                releaseResource();
                return ret;
            }
            for (int j = 0; j < avFormatContext->nb_streams; ++j) {
                AVStream *inStreamTemp = ifmt_ctx->streams[j];
                if (inStreamTemp->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    videoIndex = j;
                } else if (inStreamTemp->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                    audioIndex = j;
                }
            }
        }
        int progressTargetIndex = videoIndex;
        if (!needVideo) {
            progressTargetIndex = audioIndex;
        }

        progress = (float) (1.0 * i / length);
        int step = 0;
        float tempProgress = 0;

        int64_t tempStream0Pts = 0, tempStream1Pts = 0;

        int64_t tempStream0Dts = 0, tempStream1Dts = 0;
        while (true) {
            ret = av_read_frame(ifmt_ctx, &pkt);
            if (ret < 0) {
                stream0Pts += tempStream0Pts;
                stream1Pts += tempStream1Pts;
                stream0Dts += tempStream0Dts;
                stream1Dts += tempStream1Dts;
                BZLogUtil::logD(
                        "合并第%d视频, 上一个视频的信息 stream0Pts=%lld--stream1Pts=%lld--stream0Dts=%lld--stream1Dts=%lld",
                        i + 1, stream0Pts,
                        stream1Pts, stream0Dts, stream1Dts);
                break;
            }
            in_stream = ifmt_ctx->streams[pkt.stream_index];
            if ((in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && !needVideo) ||
                (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && !needAudio)) {
                continue;
            }

            if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                out_stream = avFormatContext->streams[videoIndex];
                pkt.stream_index = videoIndex;
            } else {
                out_stream = avFormatContext->streams[audioIndex];
                pkt.stream_index = audioIndex;
            }

            step++;
            /* copy packet */
            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;
            if (pkt.dts > 0 && pkt.stream_index == progressTargetIndex) {
                tempProgress = (float) (1.0 * pkt.dts / in_stream->duration) / length;
                if (tempProgress < 0)tempProgress = 0;
                if (tempProgress > 1)tempProgress = 1;

                if (step % 10 == 0) {//防止频繁回调
                    if (nullptr != onActionListener)
                        onActionListener->progress(progress + tempProgress);
                }
                step++;
            }

            //防止负值带来的丢包影响
            if (pkt.pts <= 0)
                pkt.pts = 1;
            if (pkt.dts <= 0)
                pkt.dts = 1;

            //延续上一个的时间戳
            if (pkt.stream_index == 0) {
                //防止重复的pts与dts
                if (tempStream0Pts == pkt.pts)
                    pkt.pts += 1;
                if (tempStream0Dts == pkt.dts)
                    pkt.dts += 1;

                tempStream0Pts = pkt.pts;
                tempStream0Dts = pkt.dts;

                pkt.pts += stream0Pts;
                pkt.dts += stream0Dts;
            } else if (pkt.stream_index == 1) {
                //防止重复的pts与dts
                if (tempStream1Pts == pkt.pts)
                    pkt.pts += 1;
                if (tempStream1Dts == pkt.dts)
                    pkt.dts += 1;

                tempStream1Pts = pkt.pts;
                tempStream1Dts = pkt.dts;

                pkt.pts += stream1Pts;
                pkt.dts += stream1Dts;
            }
//            BZLogUtil::logD("pkt.stream_index=%d--pkt.pts=%lld,pkt.dts=%lld", pkt.stream_index, pkt.pts,
//                 pkt.dts);
            ret = av_interleaved_write_frame(avFormatContext, &pkt);
            if (ret < 0) {
                BZLogUtil::logE("Error muxing packet\n");
                av_packet_unref(&pkt);
                continue;
            }
            av_packet_unref(&pkt);
        }

        if (nullptr != ifmt_ctx) {
            avformat_close_input(&ifmt_ctx);
            ifmt_ctx = nullptr;
        }
    }

    if (nullptr != avFormatContext) {
        ret = av_write_trailer(avFormatContext);
        if (ret != 0) {
            BZLogUtil::logE("av_write_trailer fail");
        }
    }
    releaseResource();
    BZLogUtil::logD("mergeVideo success");
    return 0;
}


int MergeVideoUtil::releaseResource() {
    /* close output */
    if (NULL != avFormatContext) {
        /* Close the output file. */
        if (!(avFormatContext->oformat->flags & AVFMT_NOFILE))
            avio_closep(&avFormatContext->pb);
        //打开会报错，原因不明 是因为stream->avStream->codec = avCodecContext; 导致重复释放，新版本不需要这么指定了
        avformat_free_context(avFormatContext);
        avFormatContext = NULL;
    }

//    if (NULL != videoPaths) {
//        delete videoPaths;
//        videoPaths = NULL;
//    }

    if (NULL != output_path) {
        free(output_path);
        output_path = NULL;
    }
    return 0;
}


