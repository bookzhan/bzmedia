/**
 * Created by zhandalin on 2017-05-15 14:22.
 * 说明:
 */



#include <thread>
#include "VideoTransCode.h"
#include "VideoUtil.h"
#include "../common/VideoTimeStampInfo.h"
#include "../common/bz_time.h"
#include "../common/bzcommon.h"


int VideoTransCode::startTransCode(VideoTransCodeParams *videoTransCodeParams) {
    this->progressCallBack = videoTransCodeParams->progressCallBack;
    this->pcmCallBack = videoTransCodeParams->pcmCallBack;
    this->videoTransCodeParams = videoTransCodeParams;
    isStopVideoTransCode = false;
    int ret = 0;
    //校验参数
    if (NULL == videoTransCodeParams || NULL == videoTransCodeParams->inputPath ||
        NULL == videoTransCodeParams->outputPath) {
        BZLogUtil::logE("param is error inputPath||outputPath");
        isTraning = false;
        return -1;
    }
    if (videoTransCodeParams->startTime > videoTransCodeParams->endTime) {
        BZLogUtil::logE(
                "param is error videoTransCodeParams->startTime>videoTransCodeParams->endTime");
        isTraning = false;
        return -1;
    }
    if (videoTransCodeParams->videoRotate != 0 && videoTransCodeParams->videoRotate != 90 &&
        videoTransCodeParams->videoRotate != 180 && videoTransCodeParams->videoRotate != 270) {
        BZLogUtil::logE("param is error videoRotate=%d", videoTransCodeParams->videoRotate);
        isTraning = false;
        return -1;
    }
    //软硬解
    if (videoTransCodeParams->userSoftDecode) {
        if ((ret = VideoUtil::openInputFileForSoft(videoTransCodeParams->inputPath, &in_fmt_ctx)) <
            0) {
            BZLogUtil::logE("openInputFile fail %s", videoTransCodeParams->inputPath);
            in_fmt_ctx = nullptr;
            releaseResource();
            isTraning = false;
            return ret;
        }
    } else {
        if (VideoUtil::openInputFile(videoTransCodeParams->inputPath, &in_fmt_ctx) < 0) {
            in_fmt_ctx = nullptr;
            BZLogUtil::logE("openInputFile fail 切换到软解 %s", videoTransCodeParams->inputPath);
            if ((ret = VideoUtil::openInputFileForSoft(videoTransCodeParams->inputPath,
                                                       &in_fmt_ctx)) < 0) {
                BZLogUtil::logE("openInputFileForSoft fail %s", videoTransCodeParams->inputPath);
                in_fmt_ctx = nullptr;
                releaseResource();
                isTraning = false;
                return ret;
            }
        }
    }

    BZLogUtil::logD("---------------------------------------");
    if ((ret = openOutputFile(videoTransCodeParams->outputPath)) < 0) {
        BZLogUtil::logE("openOutputFile fail");
        releaseResource();
        isTraning = false;
        return ret;
    }
    AVStream *maxDurationStream = nullptr;
    int64_t videoStartPts = 0, videoEndPts = 0;
    int64_t audioStartPts = 0, audioEndPts = 0;
    int64_t maxDuration = 0;
    for (int i = 0; i < out_fmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = out_fmt_ctx->streams[i];
        int64_t tempDuration = in_stream->duration * 1000 * in_stream->time_base.num /
                               in_stream->time_base.den;
        if (maxDuration < tempDuration) {
            maxDuration = tempDuration;
            maxDurationStream = in_stream;
        }
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoEndPts = in_stream->duration;
            videoStream = in_stream;
            if (videoTransCodeParams->startTime > 0) {
                int64_t pts =
                        static_cast<int64_t>(1.0f * videoTransCodeParams->startTime /
                                             (1000.0f * in_stream->time_base.num /
                                              in_stream->time_base.den));
                if (pts < in_stream->duration) {
                    videoStartPts = pts;
                }
            }
            if (videoTransCodeParams->endTime > 0) {
                videoEndPts = static_cast<int64_t>(1.0f * videoTransCodeParams->endTime /
                                                   (1000.0f * in_stream->time_base.num /
                                                    in_stream->time_base.den));
            }

        } else if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioEndPts = in_stream->duration;
            if (videoTransCodeParams->startTime > 0) {
                int64_t pts =
                        static_cast<int64_t>(1.0f * videoTransCodeParams->startTime /
                                             (1000.0f * in_stream->time_base.num /
                                              in_stream->time_base.den));
                if (pts < in_stream->duration) {
                    audioStartPts = pts;
                }
            }
            if (videoTransCodeParams->endTime > 0) {
                audioEndPts = static_cast<int64_t>(1.0f * videoTransCodeParams->endTime /
                                                   (1000.0f * in_stream->time_base.num /
                                                    in_stream->time_base.den));
            }

        }
    }
    if (nullptr == maxDurationStream) {
        isTraning = false;
        return -1;
    }
    //seek to start
    if (videoTransCodeParams->startTime > 0) {
        //转换成pts
        int64_t pts =
                static_cast<int64_t>(1.0f * videoTransCodeParams->startTime /
                                     (1000.0f * maxDurationStream->time_base.num /
                                      maxDurationStream->time_base.den));
        videoTransCodeParams->startTime = pts;
        av_seek_frame(in_fmt_ctx, maxDurationStream->index, pts,
                      AVSEEK_FLAG_BACKWARD);
    }

    AVPacket *decode_pkt = NULL, *encode_pkt = av_packet_alloc(), *final_pkt = NULL;
    AVStream *in_stream, *out_stream;
    int got_picture_ptr = 0, videoFrameCount = 0;
    int64_t bufferCount = 5;
    if (nullptr != videoStream && videoStream->nb_frames < 20) {
        isTraning = false;
        BZLogUtil::logE("videoStream->nb_frames <20");
        return -1;
    }

    int step = 0;
    float tempProgress = 0;

    //由于编码有缓存,所以时间戳也必须缓存下来
    list<VideoTimeStampInfo *> videoTimeStampList;

    //由于有些视频的dts不是自动增长的,所以要创建缓冲来决定先解码那个包
    list<AVPacket *> videoPacketList;
    while (true) {
        if (isStopVideoTransCode) {
            BZLogUtil::logD("isStopTransCoding");
            releaseResource();
            isTraning = false;
            return 0;
        }
        if (NULL != decode_pkt)
            av_packet_free(&decode_pkt);

        decode_pkt = av_packet_alloc();
        av_init_packet(decode_pkt);
        av_init_packet(encode_pkt);
        ret = av_read_frame(in_fmt_ctx, decode_pkt);

        if (ret < 0) {
            if (videoPacketList.size() > 0) {
                BZLogUtil::logD("取 videoPacketList 的数据继续");
                decode_pkt = videoPacketList.front();
            } else {
                BZLogUtil::logD("数据读取完毕");
                break;
            }
        }
        in_stream = in_fmt_ctx->streams[decode_pkt->stream_index];
        out_stream = out_fmt_ctx->streams[decode_pkt->stream_index];

        if (ret >= 0 && in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (decode_pkt->pts - videoStartPts < 0) {
                videoStartPts = decode_pkt->pts;
            }
            if (decode_pkt->pts > videoEndPts) {
                if (videoPacketList.empty()) {
                    BZLogUtil::logD("decode_pkt->pts>videoEndPts break");
                    break;
                } else {
                    //不再添加了,刷新videoPacketList里面的数据
                    ret = -1;
                }
            }
            decode_pkt->pts -= videoStartPts;
            decode_pkt->dts -= videoStartPts;
        } else if (ret >= 0 && in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (decode_pkt->pts - audioStartPts < 0) {
                audioStartPts = decode_pkt->pts;
            }
            if (decode_pkt->pts > audioEndPts) {
                if (videoPacketList.empty()) {
                    BZLogUtil::logD("decode_pkt->pts>audioEndPts break");
                    break;
                } else {
                    //不再添加音频数据
                    av_packet_unref(decode_pkt);
                    av_packet_unref(encode_pkt);
                    continue;
                }
            }
            decode_pkt->pts -= audioStartPts;
            decode_pkt->dts -= audioStartPts;
        }

        final_pkt = decode_pkt;
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
            videoTransCodeParams->doWithVideo) {
            if (videoFrameCount++ < bufferCount) {//先创建5个缓冲区
                videoPacketList.push_back(decode_pkt);
                decode_pkt = NULL;
                BZLogUtil::logD("pkt 缓冲区创建");
                continue;
            } else if (ret >= 0) {//正常解码流程,结束了就不再向集合里面添加数据了
                videoPacketList.push_back(decode_pkt);
            }
            videoPacketList.sort(VideoUtil::videoPacketSort);

            decode_pkt = videoPacketList.front();
            videoPacketList.pop_front();

            /* save pts */
            VideoTimeStampInfo *videoTimeStampInfo = new VideoTimeStampInfo();
            //avformat_write_header 后时间钟会变,这里转换一下
            videoTimeStampInfo->dts = av_rescale_q(decode_pkt->dts, in_stream->time_base,
                                                   out_stream->time_base);
            videoTimeStampInfo->duration = av_rescale_q(decode_pkt->duration, in_stream->time_base,
                                                        out_stream->time_base);

            videoTimeStampInfo->pts = av_rescale_q(decode_pkt->dts, in_stream->time_base,
                                                   out_stream->time_base);
            videoTimeStampList.push_back(videoTimeStampInfo);


            BZLogUtil::logD("pts=%lld--dts=%lld--duration=%lld", videoTimeStampInfo->pts,
                            videoTimeStampInfo->dts,
                            videoTimeStampInfo->duration);

            int64_t startTime = getCurrentTime();
            ret = avcodec_decode_video2(in_stream->codec, videoFrame, &got_picture_ptr,
                                        decode_pkt);
            if (ret < 0) {
                BZLogUtil::logD("avcodec_decode_video2 fail");
            }
            if (!got_picture_ptr) {
                BZLogUtil::logD("got_picture_ptr fail continue");
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            //降帧率逻辑
            if (videoTransCodeParams->frameRate > 0 &&
                videoTransCodeParams->frameRate < avg_frame_rate && spaceFrame > 0 &&
                videoDecodeFrameCount == (int) (skipFreameCount * spaceFrame)) {
                skipFreameCount++;
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            videoDecodeFrameCount++;

            BZLogUtil::logV("解码 耗时--=%lld", (getCurrentTime() - startTime));
            startTime = getCurrentTime();

            videoFrame->pts = in_stream->codec->pts_correction_last_pts;
            videoFrame->pkt_dts = in_stream->codec->pts_correction_last_pts;
            videoFrame->pict_type = AV_PICTURE_TYPE_NONE;


            AVFrame *avFrameFinal = videoFrameDoWith(in_stream->codec, videoFrame);
            if (NULL == avFrameFinal) {
                BZLogUtil::logD("NULL == avFrameFinal continue");
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            ret = avcodec_encode_video2(out_stream->codec, encode_pkt, avFrameFinal,
                                        &got_picture_ptr);
            if (avFrameFinal != videoFrame) {
                av_frame_free(&avFrameFinal);
            }
            if (ret < 0) {
                BZLogUtil::logD("avcodec_encode_video2 fail");
            }
            if (!got_picture_ptr) {
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            BZLogUtil::logD("编码 耗时--=%lld", (getCurrentTime() - startTime));
            final_pkt = encode_pkt;

            if (!videoTimeStampList.empty()) {
                VideoTimeStampInfo *videoTimeStampInfoTemp = videoTimeStampList.front();
                final_pkt->pts = videoTimeStampInfoTemp->pts;
                final_pkt->duration = videoTimeStampInfoTemp->duration;
                final_pkt->dts = videoTimeStampInfoTemp->dts;
                final_pkt->pos = -1;
                videoTimeStampList.pop_front();
                delete (videoTimeStampInfoTemp);
            }
            //需要跳帧那么时间戳就需要重新计算了
            if (videoTransCodeParams->frameRate > 0 && spaceFrame > 0) {
                final_pkt->pts = videoEncodeFrameCount * avg_frame_duration;
                final_pkt->duration = avg_frame_duration;
                final_pkt->dts = final_pkt->pts;
                final_pkt->pos = -1;
            }
            videoEncodeFrameCount++;
        } else if (in_stream->codec->codec_type == AVMEDIA_TYPE_AUDIO &&
                   videoTransCodeParams->doWithAudio) {

            ret = avcodec_decode_audio4(in_stream->codec, audioFrame, &got_picture_ptr,
                                        decode_pkt);

            if (ret < 0) {
                BZLogUtil::logD("avcodec_decode_audio4 fail");
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            if (!got_picture_ptr) {
                BZLogUtil::logD("avcodec_decode_audio4 got_picture_ptr fail continue");
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            //转换格式
            int samplesCount = swr_convert(swr_audio_ctx_2_pcm, audioFramePcm->data,
                                           audioFramePcm->nb_samples,
                                           (const uint8_t **) audioFrame->data,
                                           audioFrame->nb_samples);


            av_audio_fifo_write(audioFifo, (void **) &audioFramePcm->data[0], samplesCount);
            const char *pcmData = NULL;
            if (av_audio_fifo_size(audioFifo) >= NB_SAMPLES) {
                av_audio_fifo_read(audioFifo, (void **) &audioFramePcm->data[0],
                                   NB_SAMPLES);
                if (nullptr != pcmCallBack && methodInfoHandle != 0) {
                    pcmData = pcmCallBack(methodInfoHandle, (const char *) audioFramePcm->data[0],
                                          audioFramePcm->linesize[0]);
                }
            } else {
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            if (NULL == pcmData) {
                BZLogUtil::logE("pcmCallBack NULL==pcmData");
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            audioFramePcmTemp->data[0] = (uint8_t *) pcmData;
            ret = swr_convert(swr_audio_ctx_2_aac, audioFrameAac->data,
                              avAudioEncodeCodecContext->frame_size,
                              (const uint8_t **) audioFramePcmTemp->data,
                              avAudioEncodeCodecContext->frame_size);
            if (ret < 0) {
                BZLogUtil::logE("swr_convert ret<0 break");
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            //编码
            audioFrameAac->pts = decode_pkt->pts;
            audioFrameAac->pkt_dts = decode_pkt->dts;

            ret = avcodec_encode_audio2(avAudioEncodeCodecContext, encode_pkt,
                                        audioFrameAac,
                                        &got_picture_ptr);
            if (ret < 0) {
                BZLogUtil::logE("avcodec_encode_audio2 Failed to encode!\n");
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            if (got_picture_ptr != 1) {
                BZLogUtil::logD("got_packet_ptr!=1");
                av_packet_unref(decode_pkt);
                av_packet_unref(encode_pkt);
                continue;
            }
            final_pkt = encode_pkt;
            final_pkt->pts = decode_pkt->pts;
            final_pkt->duration = decode_pkt->duration;
            final_pkt->dts = decode_pkt->dts;
            final_pkt->pos = -1;

            free((void *) pcmData);
        }
        /* copy packet */
        if (in_stream->index == maxDurationStream->index && final_pkt->pts > 0) {
            if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                tempProgress = (float) (1.0 * final_pkt->pts / (videoEndPts - videoStartPts));
            } else {
                tempProgress = (float) (1.0 * final_pkt->pts / (audioEndPts - audioStartPts));
            }
//            BZLogUtil::logD("progress=%f", tempProgress);
            if (NULL != progressCallBack && methodInfoHandle != 0) {//防止频繁回调
                progressCallBack(methodInfoHandle, tempProgress);
            }
            step++;
        }
        final_pkt->stream_index = decode_pkt->stream_index;
        ret = av_interleaved_write_frame(out_fmt_ctx, final_pkt);
        if (ret < 0) {
            BZLogUtil::logE("Error muxing packet\n");
            av_packet_unref(decode_pkt);
            av_packet_unref(encode_pkt);
            continue;
        }
        av_packet_unref(decode_pkt);
        av_packet_unref(encode_pkt);
    }
    av_packet_free(&encode_pkt);

    //buffer 里面的时间戳就用自增长的就可以了
    flushBuffer();

    if (NULL != out_fmt_ctx) {
        ret = av_write_trailer(out_fmt_ctx);
        if (ret != 0) {
            BZLogUtil::logE("av_write_trailer fail");
        }
    }
    if (NULL != videoTransCodeParams->videoTransCodeFinish && methodInfoHandle != 0) {
        videoTransCodeParams->videoTransCodeFinish(methodInfoHandle);
    }
    BZLogUtil::logD("VideoTransCoding end");
    releaseResource();
    isTraning = false;
    return ret;
}

int VideoTransCode::openOutputFile(const char *outputPath) {
    int ret = 0;
    ret = avformat_alloc_output_context2(&out_fmt_ctx, NULL, NULL, outputPath);
    if (ret < 0) {
        BZLogUtil::logE("alloc_output_context2 fail");
        return ret;
    }
    if (!out_fmt_ctx) {
        return -1;
    }
    AVRational inVideoTimeBase = {-1, -1};
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = in_fmt_ctx->streams[i];

        AVStream *out_stream = avformat_new_stream(out_fmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            BZLogUtil::logD("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return ret;
        }
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            inVideoTimeBase.den = in_stream->time_base.den;
            inVideoTimeBase.num = in_stream->time_base.num;
        }
        out_stream->time_base = (AVRational) {in_stream->time_base.num,
                                              in_stream->time_base.den};

        out_stream->duration = in_stream->duration;
        out_stream->nb_frames = in_stream->nb_frames;
        out_stream->id = in_stream->id;
        out_fmt_ctx->duration = in_fmt_ctx->duration;

        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
            videoTransCodeParams->doWithVideo) {//这种需要处理视频
            AVCodec *avCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
            if (!avCodec) {
                BZLogUtil::logD("Could not find encoder for '%s'\n",
                                avcodec_get_name(AV_CODEC_ID_H264));
                return -1;
            }
            AVDictionaryEntry *entry = av_dict_get(in_stream->metadata, "rotate", NULL,
                                                   AV_DICT_IGNORE_SUFFIX);
            if (NULL != entry) {
                srcVideoRotate = atoi(entry->value);
                //不用设置,参数是copy过去的,包含旋转信息了,统一由OpenGL处理成没有旋转角度的
//                av_dict_set(&out_stream->metadata, "rotate", entry->value, 0);
                av_dict_set(&out_stream->metadata, "rotate", "0", 0);
            }
            BZLogUtil::logD("VideoRotate =%d", srcVideoRotate);
            if (srcVideoRotate <= 0) {
                srcVideoRotate += 360;
            }
            if (srcVideoRotate > 0) {
                finalVideoRotate = srcVideoRotate + videoTransCodeParams->videoRotate;
                if (finalVideoRotate >= 360) {
                    finalVideoRotate -= 360;
                }
            } else {
                finalVideoRotate = videoTransCodeParams->videoRotate;
            }
            videoWidth = in_stream->codecpar->width;
            videoHeight = in_stream->codecpar->height;

            avVideoEncodeCodecContext = avcodec_alloc_context3(avCodec);

            avg_frame_rate =
                    1.0f * in_stream->avg_frame_rate.num / in_stream->avg_frame_rate.den;

            if (videoTransCodeParams->frameRate > 0 &&
                videoTransCodeParams->frameRate < avg_frame_rate) {
                //转换成总时长,单位秒
                float totalSecond = (in_stream->duration * 1.0f * in_stream->time_base.num /
                                     in_stream->time_base.den);
                long currentNbFrames = static_cast<long>(totalSecond *
                                                         videoTransCodeParams->frameRate);

                spaceFrame =
                        (1.0f * in_stream->nb_frames) / (in_stream->nb_frames - currentNbFrames);
                avg_frame_duration = in_stream->duration / currentNbFrames;
                avVideoEncodeCodecContext->time_base = (AVRational) {1,
                                                                     videoTransCodeParams->frameRate};
            } else {
                avg_frame_duration = in_stream->duration / in_stream->nb_frames;
            }

            //这个对码率控制很有作用
            //越大画质越差
//            avVideoEncodeCodecContext->qmin = 15;
//            avVideoEncodeCodecContext->qmax = 25;

//            这个设置后在apple系统上播放会抖动
//            avVideoEncodeCodecContext->max_b_frames = 3;

            avVideoEncodeCodecContext->gop_size = videoTransCodeParams->gopSize;

            if (out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                avVideoEncodeCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            AVDictionary *param = NULL;
            av_dict_set(&param, "preset", "ultrafast", 0);
//            av_dict_set(&param, "tune", "zerolatency", 0);

            avVideoEncodeCodecContext->codec_id = AV_CODEC_ID_H264;
            avVideoEncodeCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

            if (finalVideoRotate == 90 || finalVideoRotate == 270) {
                avVideoEncodeCodecContext->width = videoHeight;
                avVideoEncodeCodecContext->height = videoWidth;
            } else {
                avVideoEncodeCodecContext->width = videoWidth;
                avVideoEncodeCodecContext->height = videoHeight;
            }
            //需要缩放
            if (videoTransCodeParams->maxWidth > 0 &&
                avVideoEncodeCodecContext->width > videoTransCodeParams->maxWidth) {

                videoTransCodeParams->maxWidth = videoTransCodeParams->maxWidth / 8 * 8;
                avVideoEncodeCodecContext->height =
                        (videoTransCodeParams->maxWidth * avVideoEncodeCodecContext->height /
                         avVideoEncodeCodecContext->width) / 8 * 8;
                avVideoEncodeCodecContext->width = videoTransCodeParams->maxWidth;

                //保证最小边必须大于320,否则图像采集会出现模糊的问题
                int minSize = 320;
                if (avVideoEncodeCodecContext->width > avVideoEncodeCodecContext->height) {
                    if (avVideoEncodeCodecContext->height < minSize) {
                        int temp = avVideoEncodeCodecContext->height;
                        avVideoEncodeCodecContext->height = minSize;
                        avVideoEncodeCodecContext->width =
                                minSize * avVideoEncodeCodecContext->width / temp;
                    }
                } else {
                    if (avVideoEncodeCodecContext->width < minSize) {
                        int temp = avVideoEncodeCodecContext->width;
                        avVideoEncodeCodecContext->width = minSize;
                        avVideoEncodeCodecContext->height =
                                minSize * avVideoEncodeCodecContext->height / temp;
                    }
                }
                avVideoEncodeCodecContext->width = avVideoEncodeCodecContext->width / 8 * 8;
                avVideoEncodeCodecContext->height = avVideoEncodeCodecContext->height / 8 * 8;
            }
            if (videoTransCodeParams->targetWidth > 0 && videoTransCodeParams->targetHeight > 0) {
                videoTransCodeParams->targetWidth = videoTransCodeParams->targetWidth / 8 * 8;
                videoTransCodeParams->targetHeight = videoTransCodeParams->targetHeight / 8 * 8;
                avVideoEncodeCodecContext->width = videoTransCodeParams->targetWidth;
                avVideoEncodeCodecContext->height = videoTransCodeParams->targetHeight;
            }

            if (videoTransCodeParams->frameRate > 0 &&
                videoTransCodeParams->frameRate < avg_frame_rate) {
                avVideoEncodeCodecContext->framerate = (AVRational) {1,
                                                                     videoTransCodeParams->frameRate};
            }

            //bit_rate 设置
            int64_t bit_rate = VideoUtil::getBitRate(avVideoEncodeCodecContext->width,
                                                     avVideoEncodeCodecContext->height,
                                                     videoTransCodeParams->gopSize < 3);
            avVideoEncodeCodecContext->bit_rate = bit_rate;
            avVideoEncodeCodecContext->bit_rate_tolerance = (int) (bit_rate * 2);
            avVideoEncodeCodecContext->time_base = (AVRational) {in_stream->avg_frame_rate.num,
                                                                 in_stream->avg_frame_rate.den};

            ret = avcodec_open2(avVideoEncodeCodecContext, avCodec, &param);
            if (ret < 0) {
                BZLogUtil::logD("avCodecContext Could not open video codec: %s\n", av_err2str(ret));
                return -1;
            }
            av_dict_free(&param);
            avcodec_parameters_from_context(out_stream->codecpar, avVideoEncodeCodecContext);

            videoFrame = av_frame_alloc();
            out_stream->codec = avVideoEncodeCodecContext;
        } else if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
                   videoTransCodeParams->doWithAudio) {//这种需要处理音频

            swr_audio_ctx_2_pcm = swr_alloc();
            if (!swr_audio_ctx_2_pcm) {
                BZLogUtil::logD("Could not allocate resampler context\n");
                return -1;
            }

            /* set options */
            av_opt_set_int(swr_audio_ctx_2_pcm, "out_channel_count", 1, 0);
            av_opt_set_int(swr_audio_ctx_2_pcm, "out_sample_rate", 44100, 0);
            av_opt_set_sample_fmt(swr_audio_ctx_2_pcm, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

            av_opt_set_int(swr_audio_ctx_2_pcm, "in_channel_count", in_stream->codec->channels, 0);
            av_opt_set_int(swr_audio_ctx_2_pcm, "in_sample_rate", in_stream->codec->sample_rate, 0);
            av_opt_set_sample_fmt(swr_audio_ctx_2_pcm, "in_sample_fmt",
                                  in_stream->codec->sample_fmt, 0);

            /* initialize the resampling context */
            if ((ret = swr_init(swr_audio_ctx_2_pcm)) < 0) {
                BZLogUtil::logD("Failed to initialize the resampling context\n");
                return ret;
            }


            /* create resampler context */
            swr_audio_ctx_2_aac = swr_alloc();
            if (!swr_audio_ctx_2_aac) {
                BZLogUtil::logD("Could not allocate resampler context\n");
                return -1;
            }
            /* set options */
            av_opt_set_int(swr_audio_ctx_2_aac, "in_channel_count", 1, 0);
            av_opt_set_int(swr_audio_ctx_2_aac, "in_sample_rate", 44100, 0);
            av_opt_set_sample_fmt(swr_audio_ctx_2_aac, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);

            av_opt_set_int(swr_audio_ctx_2_aac, "out_channel_count",
                           av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO), 0);
            av_opt_set_int(swr_audio_ctx_2_aac, "out_sample_rate", 44100,
                           0);
            av_opt_set_sample_fmt(swr_audio_ctx_2_aac, "out_sample_fmt",
                                  in_stream->codec->sample_fmt, 0);
            /* initialize the resampling context */
            if ((ret = swr_init(swr_audio_ctx_2_aac)) < 0) {
                BZLogUtil::logD("Failed to initialize the resampling context\n");
                return ret;
            }

            /* find the encoder */
            AVCodec *avCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
            if (!avCodec) {
                BZLogUtil::logD("Could not find encoder for '%s'\n",
                                avcodec_get_name(AV_CODEC_ID_AAC));
                return -1;
            }
            avAudioEncodeCodecContext = avcodec_alloc_context3(avCodec);
            if (!avAudioEncodeCodecContext) {
                BZLogUtil::logD("Could not alloc an encoding context\n");
                return -1;
            }
            avAudioEncodeCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
            avAudioEncodeCodecContext->bit_rate = 128000;
            avAudioEncodeCodecContext->sample_rate = 44100;
            avAudioEncodeCodecContext->channel_layout = AV_CH_LAYOUT_MONO;
            avAudioEncodeCodecContext->channels = av_get_channel_layout_nb_channels(
                    avAudioEncodeCodecContext->channel_layout);

            if (out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                avAudioEncodeCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            ret = avcodec_open2(avAudioEncodeCodecContext, avCodec, NULL);
            if (ret < 0) {
                BZLogUtil::logD("Could not open audio codec: %s\n", av_err2str(ret));
                return -1;
            }
            avcodec_parameters_from_context(out_stream->codecpar, avAudioEncodeCodecContext);
            out_stream->codec = avAudioEncodeCodecContext;
            audioFrame = av_frame_alloc();
            audioFramePcm = VideoUtil::allocAudioFrame(AV_SAMPLE_FMT_S16,
                                                       AV_CH_LAYOUT_MONO,
                                                       44100, NB_SAMPLES);


            audioFramePcmTemp = av_frame_alloc();
            audioFramePcmTemp->linesize[0] = audioFramePcm->linesize[0];
            audioFramePcmTemp->format = audioFramePcm->format;
            audioFramePcmTemp->channel_layout = audioFramePcm->channel_layout;
            audioFramePcmTemp->sample_rate = audioFramePcm->sample_rate;
            audioFramePcmTemp->nb_samples = audioFramePcm->nb_samples;

            audioFrameAac = VideoUtil::allocAudioFrame(AV_SAMPLE_FMT_FLTP,
                                                       AV_CH_LAYOUT_MONO,
                                                       44100, NB_SAMPLES);
            audioFifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_S16, 1,
                                            NB_SAMPLES);

        } else {
            avcodec_copy_context(out_stream->codec, in_stream->codec);

            if (out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

//            ret = avcodec_open2(out_stream->codec, NULL, NULL);
//            if (ret < 0) {
//                BZLogUtil::logD("avCodecContext Could not open video codec: %s\n", av_err2str(ret));
//                return -1;
//            }
            ret = avcodec_parameters_from_context(out_stream->codecpar, in_stream->codec);
            if (ret < 0) {
                BZLogUtil::logD("Could not copy the stream parameters\n");
                return -1;
            }
        }
    }
//    av_dump_format(out_fmt_ctx, 0, outputPath, 1);

    /* open the output file, if needed */
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_fmt_ctx->pb, outputPath, AVIO_FLAG_WRITE);
        if (ret < 0) {
            BZLogUtil::logD("Could not open '%s': %s\n", outputPath,
                            av_err2str(ret));
            return ret;
        }
    }
    /* Write the stream header, if any. */
    ret = avformat_write_header(out_fmt_ctx, NULL);
    if (ret < 0) {
        BZLogUtil::logD("Error occurred when opening output file: %s\n",
                        av_err2str(ret));
        return ret;
    }
    if (inVideoTimeBase.num > 0 && inVideoTimeBase.den > 0) {
        for (int i = 0; i < out_fmt_ctx->nb_streams; ++i) {
            AVStream *avStream = out_fmt_ctx->streams[i];
            if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (inVideoTimeBase.den != avStream->time_base.den ||
                    inVideoTimeBase.num != avStream->time_base.num) {
                    BZLogUtil::logD("时间钟变了,需要重新计算");
                    avStream->duration = av_rescale_q(avStream->duration, inVideoTimeBase,
                                                      avStream->time_base);
                    if (videoTransCodeParams->frameRate > 0 &&
                        videoTransCodeParams->frameRate < avg_frame_rate) {
                        //转换成总时长,单位秒
                        float totalSecond = (avStream->duration * 1.0f * avStream->time_base.num /
                                             avStream->time_base.den);
                        long currentNbFrames = static_cast<long>(totalSecond *
                                                                 videoTransCodeParams->frameRate);

                        spaceFrame =
                                (1.0f * avStream->nb_frames) /
                                (avStream->nb_frames - currentNbFrames);
                        avg_frame_duration = avStream->duration / currentNbFrames;
                        avVideoEncodeCodecContext->time_base = (AVRational) {1,
                                                                             videoTransCodeParams->frameRate};
                    } else {
                        avg_frame_duration = avStream->duration / avStream->nb_frames;
                    }
                }
            }
        }
    }
    return 0;
}

int VideoTransCode::releaseResource() {
    BZLogUtil::logD("VideoTransCoding releaseResource");
    if (NULL != in_fmt_ctx) {
        for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
            avcodec_close(in_fmt_ctx->streams[i]->codec);
        }
        avformat_close_input(&in_fmt_ctx);
        in_fmt_ctx = NULL;
    }
    if (NULL != out_fmt_ctx) {
        for (int i = 0; i < out_fmt_ctx->nb_streams; i++) {
            avcodec_close(out_fmt_ctx->streams[i]->codec);
        }
        avformat_close_input(&out_fmt_ctx);
        out_fmt_ctx = NULL;
    }
    if (NULL != videoFrame) {
        av_frame_free(&videoFrame);
        videoFrame = NULL;
    }
    if (nullptr != audioFrame) {
        av_frame_free(&audioFrame);
        audioFrame = nullptr;
    }
    if (nullptr != audioFrameAac) {
        av_frame_free(&audioFrameAac);
        audioFrameAac = nullptr;
    }
    if (nullptr != audioFramePcm) {
        av_frame_free(&audioFramePcm);
        audioFramePcm = nullptr;
    }
    if (nullptr != audioFramePcmTemp) {
        av_frame_free(&audioFramePcmTemp);
        audioFramePcmTemp = nullptr;
    }
    if (nullptr != swr_audio_ctx_2_pcm) {
        swr_free(&swr_audio_ctx_2_pcm);
        swr_audio_ctx_2_pcm = nullptr;
    }
    if (nullptr != audioFifo) {
        av_audio_fifo_free(audioFifo);
        audioFifo = nullptr;
    }
    if (NULL != avFrameDrawProgram) {
        avFrameDrawProgram->releaseResource();
        delete (avFrameDrawProgram);
        avFrameDrawProgram = NULL;
    }
    if (NULL != textureConvertYUVUtil) {
        textureConvertYUVUtil->destroyResource();
        textureConvertYUVUtil = NULL;
    }
    if (NULL != eglContextUtil) {
        eglContextUtil->releaseEGLContext();
        eglContextUtil = NULL;
    }

    //应该要delete this 但是怕有问题,暂时不处理,用不了多少内存
    return 0;
}

int VideoTransCode::flushBuffer() {
    if (!videoTransCodeParams->doWithVideo) {
        BZLogUtil::logD("------flushBuffer !videoTransCodeParams->doWithVideo-------");
        return -1;
    }
    BZLogUtil::logD("------flush_video start-------");
    int got_picture = 0, ret = 0;
    int64_t startTime = 0;
    AVStream *video_stream = NULL;
    for (int i = 0; i < out_fmt_ctx->nb_streams; ++i) {
        if (out_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = out_fmt_ctx->streams[i];
            break;
        }
    }
    AVPacket *encode_pkt = av_packet_alloc();
    while (true) {
        av_init_packet(encode_pkt);
        startTime = getCurrentTime();
        //Encode
        ret = avcodec_encode_video2(avVideoEncodeCodecContext, encode_pkt, NULL,
                                    &got_picture);
        if (ret < 0 || !got_picture) {
            BZLogUtil::logV("-----flush_video end----");
            break;
        }
        int64_t time = getCurrentTime() - startTime;
        BZLogUtil::logV("avcodec_encode_video2 耗时=%lld", time);
        if (encode_pkt->pts < 0) {
            BZLogUtil::logD("-----flush_video--- encode_pkt->pts<0");
            encode_pkt->pts = video_stream->cur_dts;
        }
        while (encode_pkt->pts <= video_stream->cur_dts) {
            encode_pkt->pts += avg_frame_duration;
            encode_pkt->dts = encode_pkt->pts;
            encode_pkt->duration = avg_frame_duration;
        }
        encode_pkt->stream_index = video_stream->index;
        ret = av_interleaved_write_frame(out_fmt_ctx, encode_pkt);
        if (ret < 0) {
            BZLogUtil::logE("av_interleaved_write_frame fail");
        }
        av_packet_unref(encode_pkt);
    }
    av_packet_free(&encode_pkt);
    BZLogUtil::logD("------flush  end-------");
    return 0;
}

void VideoTransCode::stopVideoTransCode() {
    this->isStopVideoTransCode = true;
    std::chrono::milliseconds dura(100);
    while (isTraning) {
        BZLogUtil::logD("stopVideoTransCode isTraning waiting");
        std::this_thread::sleep_for(dura);
    }
}

AVFrame *VideoTransCode::videoFrameDoWith(AVCodecContext *decodeContext, AVFrame *srcFrame) {
    if (nullptr == decodeContext || nullptr == srcFrame) {
        return srcFrame;
    }
    //这种需要把视频画成纹理回传上层
    if (nullptr != videoTransCodeParams && videoTransCodeParams->doWithVideo
        && nullptr != avVideoEncodeCodecContext) {
        if (NULL == eglContextUtil) {
            int targetWidth = avVideoEncodeCodecContext->width;
            int targetHeight = avVideoEncodeCodecContext->height;

            eglContextUtil = new EGLContextUtil();
            eglContextUtil->initEGLContext(targetWidth, targetHeight);

            if (nullptr == avFrameDrawProgram) {
                YUV_Type yuv_type;
                if (decodeContext->pix_fmt == AV_PIX_FMT_NV12 ||
                    decodeContext->pix_fmt == AV_PIX_FMT_NV21) {
                    yuv_type = YUV_TYPE_NV21_NV12;
                } else {
                    yuv_type = YUV_TYPE_YUV420P;
                }
                avFrameDrawProgram = new AVFrameDrawProgram(yuv_type);
                avFrameDrawProgram->setRotation(finalVideoRotate);
                avFrameDrawProgram->setFlip(false, true);
            }

            textureConvertYUVUtil = new TextureConvertYUVUtil();
            textureConvertYUVUtil->init(targetWidth, targetHeight);

            videoSrcframeBufferUtils = new FrameBufferUtils();
            if (videoTransCodeParams->targetHeight > 0 && videoTransCodeParams->targetWidth > 0) {
                int srcWidth = videoWidth;
                int srcHeight = videoHeight;
                if (finalVideoRotate == 90 || finalVideoRotate == 270) {
                    srcWidth = videoHeight;
                    srcHeight = videoWidth;
                }
                videoSrcframeBufferUtils->initFrameBuffer(srcWidth, srcHeight);
            } else {
                videoSrcframeBufferUtils->initFrameBuffer(targetWidth, targetHeight);
            }
        }
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //画到frameBuffer 上
        glViewport(0, 0, videoSrcframeBufferUtils->getWidth(),
                   videoSrcframeBufferUtils->getHeight());
        videoSrcframeBufferUtils->bindFrameBuffer();

        //画视频帧
        avFrameDrawProgram->draw(srcFrame);

        videoSrcframeBufferUtils->unbindFrameBuffer();

        int textureId = videoSrcframeBufferUtils->getBuffersTextureId();
        if (nullptr != videoTransCodeParams->videoTextureCallBack && methodInfoHandle != 0 &&
            videoTransCodeParams->needCallBackVideo) {
            int64_t videoTime = 0;
            if (nullptr != videoStream) {
                videoTime = 1000 * srcFrame->pts * videoStream->time_base.num /
                            videoStream->time_base.den;
            }
            textureId = videoTransCodeParams->videoTextureCallBack(methodInfoHandle,
                                                                   videoSrcframeBufferUtils->getBuffersTextureId(),
                                                                   videoSrcframeBufferUtils->getWidth(),
                                                                   videoSrcframeBufferUtils->getHeight(),
                                                                   srcFrame->pts, videoTime);
        }
        AVFrame *avFrame = textureConvertYUVUtil->textureConvertYUV(textureId);
        return avFrame;
    }
    return srcFrame;
}

void VideoTransCode::setMethodInfoHandle(int64_t methodInfoHandle) {
    this->methodInfoHandle = methodInfoHandle;
}

int64_t VideoTransCode::getMethodInfoHandle() {
    return this->methodInfoHandle;
}

