//
/**
 * Created by bookzhan on 2018-11-01 15:26.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include "AVFrameDrawProgram.h"
#include "YUV420DrawProgram.h"
#include "YUVNVDrawProgram.h"

AVFrameDrawProgram::AVFrameDrawProgram(YUV_Type yuv_type) {
    if (yuv_type == YUV_TYPE_YUV420P) {
        baseYUVDrawProgram = new YUV420DrawProgram();
    } else if (yuv_type == YUV_TYPE_NV21_NV12) {
        baseYUVDrawProgram = new YUVNVDrawProgram();
    } else {
        BZLogUtil::logE("AVFrameDrawProgram yuv_type error");
    }
    BZLogUtil::logD("AVFrameDrawProgram yuv_type=%d", yuv_type);
    if (nullptr != baseYUVDrawProgram) {
        baseYUVDrawProgram->init();
    }
    this->yuv_type = yuv_type;
}

void AVFrameDrawProgram::setRotation(int rotation) {
    if (nullptr != baseYUVDrawProgram) {
        baseYUVDrawProgram->setRotation(rotation);
    }
}

void AVFrameDrawProgram::setFlip(bool flipHorizontal, bool flipVertical) {
    if (nullptr != baseYUVDrawProgram) {
        baseYUVDrawProgram->setFlip(flipHorizontal, flipVertical);
    }
}

int AVFrameDrawProgram::draw(AVFrame *avFrame) {
    if (nullptr != baseYUVDrawProgram) {
        return baseYUVDrawProgram->draw(avFrame);
    }
    return -1;
}

int AVFrameDrawProgram::releaseResource() {
    if (nullptr != baseYUVDrawProgram) {
        baseYUVDrawProgram->releaseResource();
    }
    return 0;
}

AVFrameDrawProgram::~AVFrameDrawProgram() {
    if (nullptr != baseYUVDrawProgram) {
        delete (baseYUVDrawProgram);
        baseYUVDrawProgram = nullptr;
    }
}

YUV_Type AVFrameDrawProgram::getYUVType() {
    return yuv_type;
}
