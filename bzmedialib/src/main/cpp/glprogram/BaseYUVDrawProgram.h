//
/**
 * Created by zhandalin on 2018-11-01 15:12.
 * 说明:
 */
//

#ifndef SLIDESHOWSDK_BASEYUVDRAWPROGRAM_H
#define SLIDESHOWSDK_BASEYUVDRAWPROGRAM_H

extern "C" {
#include <libavutil/frame.h>
};

class BaseYUVDrawProgram {
public:
    virtual void init()=0;

    virtual void setRotation(int rotation)=0;

    virtual void setFlip(bool flipHorizontal, bool flipVertical)=0;

    virtual int draw(AVFrame *avFrame)=0;

    virtual int releaseResource();

protected:
    int64_t logCount = 0;

    AVFrame *getAlignAVFrame(AVFrame *inputAVFrame);

    //标准NV的一帧所包含的信息
    AVFrame *normalAVFrame = nullptr;
};


#endif //SLIDESHOWSDK_BASEYUVDRAWPROGRAM_H
