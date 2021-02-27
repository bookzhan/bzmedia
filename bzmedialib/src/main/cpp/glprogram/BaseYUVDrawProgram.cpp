//
/**
 * Created by zhandalin on 2018-12-05 19:09.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include <mediaedit/VideoUtil.h>
#include "BaseYUVDrawProgram.h"
#include "../common/bz_time.h"

AVFrame *BaseYUVDrawProgram::getAlignAVFrame(AVFrame *inputAVFrame) {
    if (nullptr == inputAVFrame || nullptr == inputAVFrame->linesize || inputAVFrame->width <= 0 ||
        inputAVFrame->height <= 0 || nullptr == inputAVFrame->data ||
        nullptr == inputAVFrame->data[0] ||
        nullptr == inputAVFrame->data[1]) {
        return nullptr;
    }
    AVFrame *finalAVFrame = inputAVFrame;
    if (inputAVFrame->linesize[0] != inputAVFrame->width) {
        //需要重新创建
        if (nullptr != normalAVFrame) {
            if (normalAVFrame->width != inputAVFrame->width ||
                normalAVFrame->height != inputAVFrame->height ||
                normalAVFrame->format != inputAVFrame->format) {
                av_frame_free(&normalAVFrame);
                normalAVFrame = nullptr;
            }
        }
        if (nullptr == normalAVFrame) {
            normalAVFrame = VideoUtil::allocVideoFrame(
                    static_cast<AVPixelFormat>(inputAVFrame->format), inputAVFrame->width,
                    inputAVFrame->height);
        }
        int64_t startTime = getCurrentTime();
        for (int i = 0; i < normalAVFrame->height; ++i) {
            if (i < normalAVFrame->height / 2) {
                memcpy(normalAVFrame->data[1] + i * normalAVFrame->linesize[1],
                       inputAVFrame->data[1] + i * inputAVFrame->linesize[1],
                       static_cast<size_t>(normalAVFrame->linesize[1]));

                if (inputAVFrame->format == AV_PIX_FMT_YUV420P &&
                    nullptr != inputAVFrame->data[2]) {
                    memcpy(normalAVFrame->data[2] + i * normalAVFrame->linesize[2],
                           inputAVFrame->data[2] + i * inputAVFrame->linesize[2],
                           static_cast<size_t>(normalAVFrame->linesize[2]));
                }
            }
            memcpy(normalAVFrame->data[0] + i * normalAVFrame->linesize[0],
                   inputAVFrame->data[0] + i * inputAVFrame->linesize[0],
                   static_cast<size_t>(normalAVFrame->linesize[0]));
        }
        if (logCount % 30 == 0) {
            BZLogUtil::logV("getAlignYUVAVFrame copy pix 耗时=%lld format=%d AV_PIX_FMT_NV12=%d",
                            getCurrentTime() - startTime, inputAVFrame->format, AV_PIX_FMT_NV12);
        }
        logCount++;

        finalAVFrame = normalAVFrame;
    }
    return finalAVFrame;
}


int BaseYUVDrawProgram::releaseResource() {
    if (nullptr != normalAVFrame) {
        av_frame_free(&normalAVFrame);
        normalAVFrame = nullptr;
    }
    return 0;
}

