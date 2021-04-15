//
/**
 * Created by bookzhan on 2018-06-12 11:31.
 * 说明:
 */
//

#include <GLES2/gl2.h>
#include <common/BZLogUtil.h>
#include "CropTextureUtil.h"

int
CropTextureUtil::cropTexture(int srcTexture, int srcWidth, int srcHeight, int startX, int startY,
                             int targetWidth, int targetHeight) {
    if (srcTexture <= 0 || srcHeight <= 0 || srcWidth <= 0 ||
        targetWidth > srcWidth || targetHeight > srcHeight || startX >= srcWidth ||
        startY >= srcHeight) {
        BZLogUtil::logE("cropTexture srcTexture <= 0 || srcHeight <= 0 || srcWidth <= 0 ||\n"
                        "            targetWidth > srcWidth || targetHeight > srcHeight || startX >= srcWidth || startY >= srcHeight");
        return -1;
    }
    if (nullptr != frameBufferUtils && (frameBufferUtils->getWidth() != targetWidth ||
                                        frameBufferUtils->getHeight() != targetHeight)) {
        frameBufferUtils->releaseFrameBuffer();
        delete frameBufferUtils;
        frameBufferUtils = nullptr;
    }
    if (nullptr == frameBufferUtils) {
        frameBufferUtils = new FrameBufferUtils();
        frameBufferUtils->initFrameBuffer(targetWidth, targetHeight);
    }

    if (nullptr == baseProgram) {
        baseProgram = new BaseProgram();
    }
    baseProgram->setTextureId(srcTexture);

    frameBufferUtils->bindFrameBuffer();
    glViewport(-startX - 1, -startY - 1, srcWidth + 1,
               srcHeight + 1);
    baseProgram->draw();
    frameBufferUtils->unbindFrameBuffer();

    return frameBufferUtils->getBuffersTextureId();
}

int CropTextureUtil::releaseCropTexture() {
    if (nullptr != frameBufferUtils) {
        frameBufferUtils->releaseFrameBuffer();
        delete (frameBufferUtils);
        frameBufferUtils = nullptr;
    }
    if (nullptr != baseProgram) {
        baseProgram->releaseResource();
        delete (baseProgram);
        baseProgram = nullptr;
    }
    return 0;
}
