//
/**
 * Created by bookzhan on 2020-05-25 17:00.
 *description:
 */
//

#include <common/BZLogUtil.h>
#include "ffmpeg_base_info.h"


void printFFmpegBaseInfo() {
    char buffer[4096];
    getFFmpegConfigure(buffer);
    BZLogUtil::logD(buffer);

    memset(buffer, 0, 4096);
    getFFmpegSupportProtocol(buffer);
    BZLogUtil::logD(buffer);

    memset(buffer, 0, 4096);
    getFFmpegSupportAVFormat(buffer);
    BZLogUtil::logD(buffer);

    memset(buffer, 0, 4096);
    getFFmpegSupportAVCodec(buffer);
    BZLogUtil::logD(buffer);

    memset(buffer, 0, 4096);
    getFFmpegSupportAVFilter(buffer);
    BZLogUtil::logD(buffer);

    testLib();
}

int getFFmpegConfigure(char *info) {
    return sprintf(info, "%s\n", avcodec_configuration());;
}

int getFFmpegSupportProtocol(char *info) {
    struct URLProtocol *pup = NULL;
    int ret = 0;
    //Input
    struct URLProtocol **p_temp = &pup;
    avio_enum_protocols((void **) p_temp, 0);
    while ((*p_temp) != NULL) {
        ret = sprintf(info, "%s[In ][%10s]\n", info, avio_enum_protocols((void **) p_temp, 0));
    }
    pup = NULL;
    //Output
    avio_enum_protocols((void **) p_temp, 1);
    while ((*p_temp) != NULL) {
        ret = sprintf(info, "%s[Out][%10s]\n", info, avio_enum_protocols((void **) p_temp, 1));
    }
    return ret;
}

int getFFmpegSupportAVFormat(char *info) {
    int ret = 0;
    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    //Input
    while (if_temp != NULL) {
        ret = sprintf(info, "%s[In ][%10s]\n", info, if_temp->name);
        if_temp = if_temp->next;
    }
    //Output
    while (of_temp != NULL) {
        ret = sprintf(info, "%s[Out][%10s]\n", info, of_temp->name);
        of_temp = of_temp->next;
    }
    return ret;
}

int getFFmpegSupportAVCodec(char *info) {
    int ret = 0;
    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            ret = sprintf(info, "%s[Dec]", info);
        } else {
            ret = sprintf(info, "%s[Enc]", info);
        }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }
        ret = sprintf(info, "%s[%10s]\n", info, c_temp->name);
        c_temp = c_temp->next;
    }
    return ret;
}

int getFFmpegSupportAVFilter(char *info) {
    int ret = 0;
    AVFilter *f_temp = (AVFilter *) avfilter_next(NULL);
    while (NULL != f_temp) {
        ret = sprintf(info, "%s[%10s]\n", info, f_temp->name);
        f_temp = (AVFilter *) avfilter_next(f_temp);
    }
    return ret;
}

int testLib() {
    AVCodec *avCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (NULL == avCodec) {
        BZLogUtil::logD("avcodec_find_decoder AV_CODEC_ID_H264 未发现");
    } else {
        BZLogUtil::logD("avcodec_find_decoder AV_CODEC_ID_H264 正常");
    }
    AVCodec *avCodecAAC = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if (NULL == avCodecAAC) {
        BZLogUtil::logD("avcodec_find_decoder AV_CODEC_ID_AAC 未发现");
    } else {
        BZLogUtil::logD("avcodec_find_decoder AV_CODEC_ID_AAC 正常");
    }


    AVCodec *avCodecEncoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (NULL == avCodecEncoder) {
        BZLogUtil::logD("avcodec_find_encoder AV_CODEC_ID_H264 未发现");
    } else {
        BZLogUtil::logD("avcodec_find_encoder AV_CODEC_ID_H264 正常");
    }

    AVCodec *avCodecEncoderMp3 = avcodec_find_encoder(AV_CODEC_ID_MP3);
    if (NULL == avCodecEncoderMp3) {
        BZLogUtil::logD("avcodec_find_encoder AV_CODEC_ID_MP3 未发现");
    } else {
        BZLogUtil::logD("avcodec_find_encoder AV_CODEC_ID_MP3 正常");
    }

    AVCodec *avCodecAACEncoder = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (NULL == avCodecAACEncoder) {
        BZLogUtil::logD("avcodec_find_encoder AV_CODEC_ID_AAC 未发现");
    } else {
        BZLogUtil::logD("avcodec_find_encoder AV_CODEC_ID_AAC 正常");
    }
    return 0;
}

