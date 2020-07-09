
/**
 * Created by zhandalin on 2017-06-14 15:57.
 * 说明:
 */


#include <mediaedit/MergeVideoUtil.h>
#include "BZLogUtil.h"
#include "BackgroundMusicUtil.h"
#include "VideoUtil.h"
#include "../common/bz_time.h"

int BackgroundMusicUtil::startAddBackgroundMusic(const char *inputPath, const char *outputPath,
                                                 const char *musicPath, float srcMusicVolume,
                                                 float bgMusicVolume,
                                                 OnActionListener *onActionListener,
                                                 bool needFade) {
    if (!VideoUtil::hasVideo(inputPath)) {
        BZLogUtil::logE("!hasVideo inputPath=%s", inputPath);
        return -1;
    }
    int ret = 0;
    string outputParentDir;
    outputParentDir.append(outputPath);
    outputParentDir = outputParentDir.substr(0, outputParentDir.find_last_of("/"));

    //处理文件名字
    string tempExtraMusicPath;
    tempExtraMusicPath.append(musicPath);
    string suffixMusic = tempExtraMusicPath.substr(tempExtraMusicPath.find_last_of("."),
                                                   tempExtraMusicPath.length());
    if (needFade) {
        suffixMusic = ".m4a";
    }
    char alignmentName[128] = {0};
    sprintf(alignmentName, "/alignment_music_%lld", getMicrosecondTime());
    string alignmentOutPath;
    alignmentOutPath.append(outputParentDir);
    alignmentOutPath.append(alignmentName);
    alignmentOutPath.append(suffixMusic);

    const char *finalMusicPath = alignmentOutPath.c_str();

    ret = alignmentMusic2Time(musicPath, finalMusicPath, VideoUtil::getMediaDuration(inputPath),
                              needFade);
    if (ret < 0) {
//        if (nullptr != progressCallBack)
//            progressCallBack(CALLBACK_TYPE_ADD_BACKGROUND_MUSIC, CALLBACK_WHAT_MESSAGE_ERROR,
//                             0);
        BZLogUtil::logE("alignmentMusic fail");
        return ret;
    }
    bool hasAudio = VideoUtil::hasAudio(inputPath);
    BZLogUtil::logD("startAddBackgroundMusic hasAudio=%d", hasAudio);
    int64_t handle = 0;
    if (nullptr != onActionListener) {
        handle = reinterpret_cast<int64_t>(onActionListener);
    }
    if (hasAudio) {
        //开始合并视频与音频
        char mergeCmd[1024] = {0};
        sprintf(mergeCmd,
                "ffmpeg -y -i \"%s\" -i \"%s\" -c:v copy -filter_complex [0:a]aformat=fltp:44100:stereo,volume=%.2f,apad[0a];[1]aformat=fltp:44100:stereo,volume=%.2f[1a];[0a][1a]amerge[a] -map 0:v -map [a] -ac 2 %s",
                inputPath, finalMusicPath, srcMusicVolume, bgMusicVolume, outputPath);
        ret = executeFFmpegCommand(handle, mergeCmd, OnActionListener::progressCallBack);
        if (ret < 0) {
            BZLogUtil::logE("executeFFmpegCommand fail %s", mergeCmd);
            return ret;
        }
    } else {
        char mergeCmd[1024] = {0};
        sprintf(mergeCmd,
                "ffmpeg -y -i \"%s\" -i \"%s\" -map 0:v -vcodec copy -map 1:a -af volume=%.2f \"%s\"",
                inputPath, finalMusicPath, bgMusicVolume, outputPath);
        ret = executeFFmpegCommand(handle, mergeCmd, OnActionListener::progressCallBack);
        if (ret < 0) {
            BZLogUtil::logE("executeFFmpegCommand fail %s", mergeCmd);
            return ret;
        }
    }
    if (strcmp(finalMusicPath, musicPath) != 0) {
        //删除临时文件
        remove(finalMusicPath);
    }
    return 0;
}


int BackgroundMusicUtil::replaceBackgroundMusic(const char *videoPath, const char *musicPath,
                                                const char *outputPath,
                                                OnActionListener *onActionListener) {
    int ret = 0;
    string tempExtraMusicPath;
    tempExtraMusicPath.append(musicPath);
    string suffixMusic = tempExtraMusicPath.substr(tempExtraMusicPath.find_last_of("."),
                                                   tempExtraMusicPath.length());
    //检测是不是m4a
    if (suffixMusic.compare(".m4a") != 0) {
        BZLogUtil::logE("Music file %s 必须是m4a文件, 如果必须要用请调用 addBackgroundMusic %s", musicPath);
        return -1;
    }
    //检测视频与音频
    if (!VideoUtil::hasVideo(videoPath)) {
        BZLogUtil::logE("videoPath=%s 不是视频文件文件", videoPath);
        return -1;
    }
    if (!VideoUtil::hasAudio(musicPath)) {
        BZLogUtil::logE("musicPath=%s 不是音频文件", musicPath);
        return -1;
    }

    string outputParentDir;
    outputParentDir.append(outputPath);
    outputParentDir = outputParentDir.substr(0, outputParentDir.find_last_of("/"));

    char alignmentName[128] = {0};
    sprintf(alignmentName, "/alignment_music_%lld", getMicrosecondTime());
    string alignmentOutPath;
    alignmentOutPath.append(outputParentDir);
    alignmentOutPath.append(alignmentName);
    alignmentOutPath.append(suffixMusic);

    const char *finalMusicPath = alignmentOutPath.c_str();

    ret = alignmentMusic2Time(musicPath, finalMusicPath, VideoUtil::getMediaDuration(videoPath),
                              true);
    if (ret < 0) {
        BZLogUtil::logE("alignmentMusic fail");
        return ret;
    }
    //开始替换背景音乐
    char mergeCmd[1024] = {0};
    sprintf(mergeCmd,
            "ffmpeg -y -i \"%s\" -i \"%s\" -map 0:v -vcodec copy -map 1:a -acodec copy %s",
            videoPath, finalMusicPath, outputPath);
    int64_t handle = 0;
    if (nullptr != onActionListener) {
        handle = reinterpret_cast<int64_t>(onActionListener);
    }
    ret = executeFFmpegCommand(handle, mergeCmd, OnActionListener::progressCallBack);
    if (ret < 0) {
        BZLogUtil::logE("executeFFmpegCommand fail %s", mergeCmd);
        return ret;
    }
    if (strcmp(finalMusicPath, musicPath) != 0) {
        //删除临时文件
        remove(finalMusicPath);
    }
    return 0;
}


int BackgroundMusicUtil::alignmentMusic2Time(const char *musicPath,
                                             const char *outputPath, int64_t time, bool needFade) {

    int ret = 0;
    //先检测配乐的时长
    int64_t videoDuration = time;
    if (videoDuration <= 0) {
        BZLogUtil::logE("videoDuration<=0");
        return ret;
    }
    int64_t musicDuration = VideoUtil::getMediaDuration(musicPath);
    if (musicDuration <= 0) {
        BZLogUtil::logE("musicDuration<=0");
        return ret;
    }
    string outputParentDir;
    outputParentDir.append(outputPath);
    outputParentDir = outputParentDir.substr(0, outputParentDir.find_last_of("/"));

    //处理文件名字
    string tempExtraMusicPath;
    tempExtraMusicPath.append(musicPath);
    string suffixMusic = tempExtraMusicPath.substr(tempExtraMusicPath.find_last_of("."),
                                                   tempExtraMusicPath.length());

    char alignmentName[128] = {0};
    sprintf(alignmentName, "/temp_alignment_%lld", getMicrosecondTime());
    string temp_alignment;
    temp_alignment.append(outputParentDir);
    temp_alignment.append(alignmentName);
    temp_alignment.append(suffixMusic);

    const char *tempAlignmentFile = temp_alignment.c_str();

    std::list<string *> willDeleteFileList;
    //对齐处理
    if (musicDuration > videoDuration) {//需要裁切配乐
        char alignmentCmd[1024] = {0};
        sprintf(alignmentCmd, "ffmpeg -y -ss 0 -t %.3f -i \"%s\" -vn -acodec copy %s",
                1.0f * videoDuration / 1000, musicPath, tempAlignmentFile);
        BZLogUtil::logD("alignmentCmd=%s", alignmentCmd);
        ret = executeFFmpegCommand(0, alignmentCmd, NULL);
        if (ret < 0) {
            BZLogUtil::logE("executeFFmpegCommand fail %s", alignmentCmd);
            return ret;
        }
        if (needFade) {
            ret = fadeMusic(tempAlignmentFile, outputPath);
            if (ret < 0) {//失败用原先的文件
                rename(tempAlignmentFile, outputPath);
            } else {
                willDeleteFileList.push_back(new string(tempAlignmentFile));
            }
        } else {
            rename(tempAlignmentFile, outputPath);
        }
    } else if (musicDuration < videoDuration) {//需要合并配乐到视频长度
        const char *audioSrcPath = musicPath;
        char needFadeName[128] = {0};
        sprintf(needFadeName, "/temp_fade_%lld", getMicrosecondTime());
        string temp_needFadeName;
        temp_needFadeName.append(outputParentDir);
        temp_needFadeName.append(needFadeName);
        temp_needFadeName.append(".m4a");

        if (needFade) {
            ret = fadeMusic(musicPath, temp_needFadeName.c_str());
            if (ret >= 0) {//成功替换原先的文件
                audioSrcPath = temp_needFadeName.c_str();
                willDeleteFileList.push_back(new string(audioSrcPath));
            }
        }
        int contactNum = (int) (videoDuration / musicDuration + 1);
        BZLogUtil::logD("contactNum=%d,videoDuration=%lld,musicDuration=%lld", contactNum,
                        videoDuration, musicDuration);
        char **inputPath = (char **) malloc(sizeof(char *) * contactNum);

        for (int i = 0; i < contactNum; ++i) {
            size_t len = strlen(audioSrcPath) + 1;
            char *buffer = static_cast<char *>(malloc(len));
            memset(buffer, 0, len);
            sprintf(buffer, "%s", audioSrcPath);
            inputPath[i] = buffer;
        }

        char merge_result_name[128] = {0};
        sprintf(merge_result_name, "/merge_result_%lld", getMicrosecondTime());
        string mergeResultPath;
        mergeResultPath.append(outputParentDir);
        mergeResultPath.append(merge_result_name);

        string tempOutMusicPath;
        tempOutMusicPath.append(audioSrcPath);
        string suffixMusicTemp = tempOutMusicPath.substr(tempOutMusicPath.find_last_of("."),
                                                         tempOutMusicPath.length());
        mergeResultPath.append(suffixMusicTemp);

        const char *temp_path_2 = mergeResultPath.c_str();

        MergeVideoUtil mergeVideoUtil;
        ret = mergeVideoUtil.startMergeVideo(inputPath, contactNum, temp_path_2,
                                             NULL);
        for (int i = 0; i < contactNum; ++i) {
            free(inputPath[i]);
        }
        free(inputPath);

        if (ret < 0) {
            BZLogUtil::logE("MergeVideo fail");
            return ret;
        }

        char alignmentCmd[512] = {0};
        sprintf(alignmentCmd, "ffmpeg -y -ss 0 -t %.3f -i \"%s\" -vn -acodec copy %s",
                1.0f * videoDuration / 1000, temp_path_2, outputPath);

        BZLogUtil::logD("alignmentCmd=%s", alignmentCmd);
        ret = executeFFmpegCommand(0, alignmentCmd, NULL);
        if (ret < 0) {
            rename(temp_path_2, outputPath);
            BZLogUtil::logE("executeFFmpegCommand fail %s", alignmentCmd);
            return ret;
        } else {
            willDeleteFileList.push_back(new string(temp_path_2));
        }
    } else {
        BZLogUtil::logD("copy file");
        char copyCmd[512];
        sprintf(copyCmd, "ffmpeg -y -i \"%s\" -vn -acodec copy %s",
                musicPath, outputPath);

        BZLogUtil::logD("copyCmd=%s", copyCmd);
        ret = executeFFmpegCommand(0, copyCmd, NULL);
        if (ret < 0) {
            BZLogUtil::logE("executeFFmpegCommand fail %s", copyCmd);
            return ret;
        }
    }
    std::list<string *>::iterator list_iterator = willDeleteFileList.begin();
    while (list_iterator != willDeleteFileList.end()) {
        string *path = *list_iterator;
        BZLogUtil::logD("remove path=%s", path->c_str());
        remove(path->c_str());
        delete (path);
        list_iterator++;
    }
    return 0;
}

int BackgroundMusicUtil::fadeMusic(const char *musicPath, const char *outputPath) {
    if (nullptr == musicPath || nullptr == outputPath) {
        return -1;
    }
    //单位毫秒
    int fadeTime = 1000;
    //处理文件名字
    string tempExtraMusicPath;
    tempExtraMusicPath.append(musicPath);
    string suffixMusic = tempExtraMusicPath.substr(tempExtraMusicPath.find_last_of("."),
                                                   tempExtraMusicPath.length());
    if (suffixMusic.compare(".m4a") != 0 && suffixMusic.compare(".M4A") != 0) {
        BZLogUtil::logD("fadeMusic 原始文件 不是m4a文件不能加速处理");
        int64_t musicDuration = VideoUtil::getMediaDuration(musicPath);
        int64_t startTime = musicDuration - fadeTime;
        if (startTime < 0) {
            startTime = 0;
            fadeTime = 1000;
        }
        //做淡出
        char cmdBuffer[1024] = {0};
        sprintf(cmdBuffer, "ffmpeg -y -i \"%s\" -af afade=t=out:st=%.3f:d=%.3f -vn \"%s\"",
                musicPath, startTime * 1.0f / 1000, fadeTime * 1.0f / 1000,
                outputPath);
        BZLogUtil::logD("cmdBuffer=%s", cmdBuffer);

        int ret = executeFFmpegCommand(0, cmdBuffer, nullptr);
        if (ret < 0) {
            BZLogUtil::logE("executeFFmpegCommand fail");
        }
        BZLogUtil::logD("fadeMusic finish");
        return 0;
    }

    BZLogUtil::logD("fadeMusic %s outputPath=%s", musicPath, outputPath);

    AVFormatContext *inputAVFormatContext = NULL;
    int ret = 0;
    ret = avformat_open_input(&inputAVFormatContext, musicPath, NULL, NULL);
    if (ret < 0) {
        BZLogUtil::logE("avformat_open_input fail %s", musicPath);
        return ret;
    }
    AVStream *inputAudioAVStream = NULL;
    for (int i = 0; i < inputAVFormatContext->nb_streams; ++i) {
        AVStream *avStream = inputAVFormatContext->streams[i];
        if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            inputAudioAVStream = avStream;
            break;
        }
    }
    if (NULL == inputAudioAVStream) {
        BZLogUtil::logE("can't find music stream");
        return ret;
    }
    long musicDuration = (long) (inputAudioAVStream->duration *
                                 av_q2d(inputAudioAVStream->time_base) * 1000);
    AVCodec *avCodec = avcodec_find_decoder(inputAudioAVStream->codecpar->codec_id);
    if (NULL == avCodec) {
        BZLogUtil::logE("can't find_decoder");
        return ret;
    }
    AVCodecContext *inputAudioAVCodecContext = avcodec_alloc_context3(avCodec);
    if (NULL == inputAudioAVCodecContext) {
        BZLogUtil::logE("can't avcodec_alloc_context3");
        return ret;
    }

    avcodec_parameters_to_context(inputAudioAVCodecContext, inputAudioAVStream->codecpar);

    ret = avcodec_open2(inputAudioAVCodecContext, NULL, NULL);
    if (ret < 0) {
        BZLogUtil::logE("Failed to open decoder for stream");
        return ret;
    }
    inputAudioAVCodecContext->time_base.den = inputAudioAVStream->time_base.den;
    inputAudioAVCodecContext->time_base.num = inputAudioAVStream->time_base.num;

    //处理编码器
    AVCodec *avCodecEncoder = avcodec_find_encoder(inputAudioAVStream->codecpar->codec_id);
    if (NULL == avCodecEncoder) {
        BZLogUtil::logE("can't find_encoder");
        return ret;
    }
    AVCodecContext *outputAudioAVCodecContext = avcodec_alloc_context3(avCodecEncoder);
    if (NULL == outputAudioAVCodecContext) {
        BZLogUtil::logE("can't avcodec_alloc_context3");
        return ret;
    }
    avcodec_parameters_to_context(outputAudioAVCodecContext, inputAudioAVStream->codecpar);
    outputAudioAVCodecContext->sample_fmt = inputAudioAVCodecContext->sample_fmt;
    outputAudioAVCodecContext->time_base = inputAudioAVCodecContext->time_base;
    outputAudioAVCodecContext->frame_size = 1024;

    ret = avcodec_open2(outputAudioAVCodecContext, NULL, NULL);
    if (ret < 0) {
        BZLogUtil::logE("Failed to open encoder for stream");
        return ret;
    }
    //开始处理的时间
    int64_t startTime = musicDuration - fadeTime;
    if (startTime < 0) {
        startTime = 0;
        fadeTime = 1000;
    }

    char filters[512] = {0};
    sprintf(filters, "afade=t=out:st=%.3f:d=%.3f", startTime * 1.0f / 1000, fadeTime * 1.0f / 1000);
    //处理淡出滤镜
    initAudioFilters(filters, inputAudioAVCodecContext);

    //处理输出视频
    AVFormatContext *outputAVFormatContext = NULL;
    ret = VideoUtil::openOutputFile(inputAVFormatContext, &outputAVFormatContext,
                                    outputPath);
    if (ret < 0) {
        BZLogUtil::logE("openOutputFile fail %s", outputPath);
        return ret;
    }

    AVPacket avPacket, avPacketFilter;
    AVPacket *avPacketTemp = NULL;
    AVFrame *avFrameSrc = VideoUtil::allocAudioFrame(inputAudioAVCodecContext->sample_fmt,
                                                     inputAudioAVCodecContext->channel_layout,
                                                     inputAudioAVCodecContext->sample_rate,
                                                     outputAudioAVCodecContext->frame_size);

    AVFrame *avFrameFilter = VideoUtil::allocAudioFrame(inputAudioAVCodecContext->sample_fmt,
                                                        inputAudioAVCodecContext->channel_layout,
                                                        inputAudioAVCodecContext->sample_rate,
                                                        outputAudioAVCodecContext->frame_size);;

    int got_frame_ptr = 0;
    while (true) {
        av_init_packet(&avPacket);
        ret = av_read_frame(inputAVFormatContext, &avPacket);
        if (ret < 0) {
            break;
        }
        avPacketTemp = &avPacket;

        int64_t progressTime =
                avPacketTemp->pts * 1000 * inputAudioAVCodecContext->time_base.num /
                inputAudioAVCodecContext->time_base.den;

//        BZLogUtil::logE("av_read_frame");
        if (progressTime > startTime) {
            //解码
            ret = avcodec_decode_audio4(inputAudioAVCodecContext, avFrameSrc, &got_frame_ptr,
                                        avPacketTemp);
            if (ret < 0 || got_frame_ptr == 0) {
                BZLogUtil::logD("avcodec_decode_audio4 ret < 0 || got_frame_ptr == 0");
                continue;
            }
            //过滤
            if (av_buffersrc_add_frame_flags(buffersrc_ctx, avFrameSrc, 0) < 0) {
                BZLogUtil::logE("Error while feeding the audio filtergraph\n");
                continue;
            }
            /* pull filtered audio from the filtergraph */
            while (1) {
                ret = av_buffersink_get_frame(buffersink_ctx, avFrameFilter);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
                    break;
                }
            }
            avPacketFilter.pts = avPacket.pts;
            avPacketFilter.dts = avPacket.dts;

            av_init_packet(&avPacketFilter);

            ret = avcodec_encode_audio2(outputAudioAVCodecContext, &avPacketFilter,
                                        avFrameFilter,
                                        &got_frame_ptr);
            if (ret < 0 || got_frame_ptr == 0) {
                BZLogUtil::logD("avcodec_encode_audio2 ret < 0 || got_frame_ptr == 0");
                continue;
            }
            avPacketFilter.pts = avPacket.pts;
            avPacketFilter.dts = avPacket.dts;
            avPacketFilter.duration = avPacket.duration;

//            BZLogUtil::logD("avPacketFilter pts=%lld ddts=%lld size=%d duration=%lld",
//                 avPacketFilter.pts, avPacketFilter.dts, avPacketFilter.size,
//                 avPacketFilter.duration);

            av_interleaved_write_frame(outputAVFormatContext, &avPacketFilter);
            av_packet_unref(&avPacketFilter);
        } else {
//            BZLogUtil::logD("avPacketFilter before pts=%lld ddts=%lld size=%d duration=%lld",
//                 avPacketTemp->pts, avPacketTemp->dts, avPacketTemp->size, avPacketTemp->duration);
            av_interleaved_write_frame(outputAVFormatContext, avPacketTemp);
        }
    }
    ret = av_write_trailer(outputAVFormatContext);
    if (ret != 0) {
        BZLogUtil::logE("av_write_trailer fail");
    }
    avformat_close_input(&inputAVFormatContext);
    avformat_free_context(outputAVFormatContext);

    avcodec_close(inputAudioAVCodecContext);
    avcodec_free_context(&inputAudioAVCodecContext);

    avcodec_close(outputAudioAVCodecContext);
    avcodec_free_context(&outputAudioAVCodecContext);

    BZLogUtil::logD("fadeMusic finish");
    return ret;
}

int
BackgroundMusicUtil::initAudioFilters(const char *filters_descr, AVCodecContext *avCodecContext) {
    BZLogUtil::logD("initAudioFilters filters_descr= %s", filters_descr);
    char args[512];
    int ret = 0;
    const AVFilter *abuffersrc = avfilter_get_by_name("abuffer");
    const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    const enum AVSampleFormat out_sample_fmts[] = {avCodecContext->sample_fmt,
                                                   AV_SAMPLE_FMT_NONE};

    const int64_t out_channel_layouts[] = {(const int64_t) avCodecContext->channel_layout, -1};
    const int out_sample_rates[] = {avCodecContext->time_base.den / avCodecContext->time_base.num,
                                    -1};
    const AVFilterLink *outlink;
    AVRational time_base = avCodecContext->time_base;

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    if (!avCodecContext->channel_layout)
        avCodecContext->channel_layout = (uint64_t) av_get_default_channel_layout(
                avCodecContext->channels);
    snprintf(args, sizeof(args),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%lld",
             time_base.num, time_base.den, avCodecContext->sample_rate,
             av_get_sample_fmt_name(avCodecContext->sample_fmt), avCodecContext->channel_layout);
    ret = avfilter_graph_create_filter(&buffersrc_ctx, abuffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
        goto end;
    }

    /* buffer audio sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, abuffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "sample_fmts", out_sample_fmts, -1,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "channel_layouts", out_channel_layouts, -1,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "sample_rates", out_sample_rates, -1,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

    /* Print summary of the sink buffer
     * Note: args buffer is reused to store channel layout string */
    outlink = buffersink_ctx->inputs[0];
    av_get_channel_layout_string(args, sizeof(args), -1, outlink->channel_layout);
    av_log(NULL, AV_LOG_INFO, "Output: srate:%dHz fmt:%s chlayout:%s\n",
           outlink->sample_rate,
           (char *) av_x_if_null(av_get_sample_fmt_name((AVSampleFormat) outlink->format), "?"),
           args);

    BZLogUtil::logD("initAudioFilters success");
    end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return ret;
}

int BackgroundMusicUtil::replaceBackgroundMusicOnly(const char *videoPath, const char *musicPath,
                                                    const char *outputPath,
                                                    OnActionListener *onActionListener) {
    if (nullptr == videoPath || nullptr == musicPath || nullptr == outputPath) {
        BZLogUtil::logE("nullptr==videoPath|| nullptr==musicPath|| nullptr==outputPath");
        return -1;
    }
    int ret = 0;
    string tempExtraMusicPath;
    tempExtraMusicPath.append(musicPath);
    string suffixMusic = tempExtraMusicPath.substr(tempExtraMusicPath.find_last_of("."),
                                                   tempExtraMusicPath.length());
    //检测是不是m4a,或者mp4文件
    if (suffixMusic.compare(".m4a") != 0
        && suffixMusic.compare(".mp4") != 0
        && suffixMusic.compare(".MP4") != 0) {
        BZLogUtil::logE("Music file %s 必须是m4a,或者mp4文件, 如果必须要用请调用 addBackgroundMusic", musicPath);
        return -1;
    }
    //检测视频与音频
    if (!VideoUtil::hasVideo(videoPath)) {
        BZLogUtil::logE("videoPath=%s 不是视频文件文件", videoPath);
        return -1;
    }
    if (!VideoUtil::hasAudio(musicPath)) {
        BZLogUtil::logE("musicPath=%s 不是音频文件", musicPath);
        return -1;
    }

    //开始替换背景音乐
    char mergeCmd[1024] = {0};
    sprintf(mergeCmd,
            "ffmpeg -y -i \"%s\" -i \"%s\" -map 0:v -vcodec copy -map 1:a -acodec copy %s",
            videoPath, musicPath, outputPath);
    int64_t handle = 0;
    if (nullptr != onActionListener) {
        handle = reinterpret_cast<int64_t>(onActionListener);
    }
    ret = executeFFmpegCommand(handle, mergeCmd, OnActionListener::progressCallBack);
    if (ret < 0) {
        BZLogUtil::logE("executeFFmpegCommand fail %s", mergeCmd);
        return ret;
    }
    return 0;
}

int BackgroundMusicUtil::delayMusic(const char *musicPath, const char *outPath, int64_t startTime,
                                    int64_t seekStartTime, int64_t seekEndTime) {
    if (nullptr == musicPath || nullptr == outPath || startTime < 0) {
        return -1;
    }
    int ret = 0;
    if (seekEndTime - seekStartTime > 0) {
        char cmd[1024] = {0};
        //最大支持处理4通道
        sprintf(cmd, "ffmpeg -y -ss %.3f -t %.3f -i \"%s\" -af adelay=%lld|%lld|%lld|%lld -vn %s",
                1.0f * seekStartTime / 1000,
                1.0f * (seekEndTime - seekStartTime) / 1000,
                musicPath, startTime,
                startTime, startTime, startTime, outPath);
        ret = executeFFmpegCommand(0, cmd, NULL);
    } else {
        char cmd[1024] = {0};
        //最大支持处理4通道
        sprintf(cmd, "ffmpeg -y -i \"%s\" -af adelay=%lld|%lld|%lld|%lld -vn %s",
                musicPath, startTime,
                startTime, startTime, startTime, outPath);
        ret = executeFFmpegCommand(0, cmd, NULL);
    }
    return ret;
}

int BackgroundMusicUtil::mixMusic(list<const char *> *musicList, const char *outPath,
                                  OnActionListener *onActionListener) {
    if (nullptr == musicList || musicList->size() <= 1 || nullptr == outPath) {
        return -1;
    }
    string cmd;
    cmd.append("ffmpeg -y ");
    list<const char *>::iterator list_iterator = musicList->begin();
    int count = 0;
    while (list_iterator != musicList->end()) {
        cmd.append("-i \"");
        cmd.append(*list_iterator);
        cmd.append("\" ");
        list_iterator++;
        count++;
    }
    char buffer[512] = {0};
    sprintf(buffer, "-filter_complex amix=inputs=%d -vn \"%s\"", count, outPath);
    cmd.append(buffer);

    int64_t handle = 0;
    if (nullptr != onActionListener) {
        handle = reinterpret_cast<int64_t>(onActionListener);
    }
    return executeFFmpegCommand(handle, cmd.c_str(), OnActionListener::progressCallBack);;
}

int BackgroundMusicUtil::mixMusic(const char *inputMusicPath_1, const char *inputMusicPath_2,
                                  const char *outputPath, float musicVolume_1,
                                  float musicVolume_2) {
    char cmd[2048] = {0};
    sprintf(cmd,
            "ffmpeg -y -i \"%s\" -i \"%s\" -filter_complex [0:a]aformat=fltp:44100:stereo,volume=%.2f[0a];[1:a]aformat=fltp:44100:stereo,volume=%.2f,apad[1a];[0a][1a]amerge[a] -map [a] -vn %s",
            inputMusicPath_1, inputMusicPath_2, musicVolume_1, musicVolume_2, outputPath);
    BZLogUtil::logD("mixMusic cmd=%s", cmd);
    return executeFFmpegCommand(0, cmd, NULL);
}


