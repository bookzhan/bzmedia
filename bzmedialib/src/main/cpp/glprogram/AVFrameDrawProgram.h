//
/**
 * Created by bookzhan on 2018-11-01 15:26.
 * 说明:
 */
//

#ifndef SLIDESHOWSDK_AVFRAMEDRAWPROGRAM_H
#define SLIDESHOWSDK_AVFRAMEDRAWPROGRAM_H

#include "BaseYUVDrawProgram.h"

enum YUV_Type {
    YUV_TYPE_YUV420P, YUV_TYPE_NV21_NV12
};

class AVFrameDrawProgram {
public:
    AVFrameDrawProgram(YUV_Type yuv_type);

    void setRotation(int rotation);

    void setFlip(bool flipHorizontal, bool flipVertical);

    int draw(AVFrame *avFrame);

    YUV_Type getYUVType();

    int releaseResource();

    ~AVFrameDrawProgram();

private:
    BaseYUVDrawProgram *baseYUVDrawProgram = nullptr;
    YUV_Type yuv_type = YUV_Type::YUV_TYPE_YUV420P;
};


#endif //SLIDESHOWSDK_AVFRAMEDRAWPROGRAM_H
