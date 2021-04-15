//
/**
 * Created by bookzhan on 2018-11-14 10:49.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include <cstring>
#include <common/bz_time.h>
#include "VideoTextureManger.h"
#include <mediaedit/VideoUtil.h>

void VideoTextureManger::release() {

    if (nullptr != uniformSizeFrameBufferUtils) {
        uniformSizeFrameBufferUtils->releaseFrameBuffer();
        delete (uniformSizeFrameBufferUtils);
        uniformSizeFrameBufferUtils = nullptr;
    }

    if (nullptr != gaussBlurProgram) {
        gaussBlurProgram->releaseResource();
        delete (gaussBlurProgram);
        gaussBlurProgram = nullptr;
    }
    if (nullptr != baseProgram) {
        baseProgram->releaseResource();
        delete (baseProgram);
        baseProgram = nullptr;
    }
    if (nullptr != imageBgGaussBlurProgram) {
        imageBgGaussBlurProgram->releaseResource();
        delete (imageBgGaussBlurProgram);
        imageBgGaussBlurProgram = nullptr;
    }
}

void VideoTextureManger::setUniformTextureSize(int uniformTextureWidth, int uniformTextureHeight) {
    this->uniformTextureWidth = uniformTextureWidth;
    this->uniformTextureHeight = uniformTextureHeight;
    if (nullptr != uniformSizeFrameBufferUtils) {
        uniformSizeFrameBufferUtils->releaseFrameBuffer();
        delete (uniformSizeFrameBufferUtils);
        uniformSizeFrameBufferUtils = nullptr;
        uniformSizeFrameBufferUtils = new FrameBufferUtils;
        uniformSizeFrameBufferUtils->initFrameBuffer(uniformTextureWidth, uniformTextureHeight);
    }
}

TextureInfo *VideoTextureManger::getUniformTexture(TextureInfo *textureInfo) {
    if (nullptr == textureInfo || textureInfo->textureWidth <= 0 ||
        textureInfo->textureHeight <= 0) {
        return textureInfo;
    }
    if (uniformTextureHeight > 0 && uniformTextureWidth > 0) {
        if (nullptr == gaussBlurProgram) {
            gaussBlurProgram = new GaussBlurProgram();
            gaussBlurProgram->setRadius(gaussBlurRadius);
        }
        if (nullptr == baseProgram) {
            baseProgram = new BaseProgram();
            baseProgram->init();
        }
        int result = 0;
        if (nullptr == textureHandleInfo ||
            (nullptr != textureHandleInfo && textureHandleInfo->bgFillType == GAUSS_BLUR
             && textureHandleInfo->scaleType != CENTER_CROP)) {
            gaussBlurProgram->setTextureId(textureInfo->textureID);
            gaussBlurProgram->setSize(textureInfo->textureWidth,
                                      textureInfo->textureHeight, uniformTextureWidth,
                                      uniformTextureHeight,
                                      true);
            result = gaussBlurProgram->draw();
        }


        if (nullptr == uniformSizeFrameBufferUtils) {
            uniformSizeFrameBufferUtils = new FrameBufferUtils;
            uniformSizeFrameBufferUtils->initFrameBuffer(uniformTextureWidth, uniformTextureHeight);
        }

        //bg Texture
        if (nullptr != textureHandleInfo && textureHandleInfo->bgFillType == IMAGE &&
            textureHandleInfo->bgTextureId > 0 && textureHandleInfo->bgTextureWidth > 0 &&
            textureHandleInfo->bgTextureHeight > 0) {
            if (nullptr == imageBgGaussBlurProgram) {
                imageBgGaussBlurProgram = new GaussBlurProgram();
                imageBgGaussBlurProgram->setFlip(false, false);
            }
            imageBgGaussBlurProgram->setRadius(textureHandleInfo->bgTextureIntensity);
            imageBgGaussBlurProgram->setTextureId(textureHandleInfo->bgTextureId);
            imageBgGaussBlurProgram->setSize(textureHandleInfo->bgTextureWidth,
                                             textureHandleInfo->bgTextureHeight,
                                             uniformTextureWidth,
                                             uniformTextureHeight,
                                             true);
            result = imageBgGaussBlurProgram->draw();
        }

        uniformSizeFrameBufferUtils->bindFrameBuffer();
        if (nullptr != textureHandleInfo) {
            glClearColor(textureHandleInfo->bgColor.r, textureHandleInfo->bgColor.g,
                         textureHandleInfo->bgColor.b, textureHandleInfo->bgColor.a);
        } else {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, uniformTextureWidth, uniformTextureHeight);

        if (result > 0) {//draw bg
            baseProgram->setTextureId(result);
            baseProgram->draw();
        }
        float scaling =
                textureInfo->textureWidth / (float) textureInfo->textureHeight;
        float viewRatio = uniformTextureWidth / (float) uniformTextureHeight;
        float s = scaling / viewRatio;
        int w, h;
        bool mFitFullView = (nullptr != textureHandleInfo &&
                             textureHandleInfo->scaleType == CENTER_CROP);
        if (mFitFullView) {
            //撑满全部view(内容大于view)
            if (s > 1.0) {
                w = static_cast<int>(uniformTextureHeight * scaling);
                h = uniformTextureHeight;
            } else {
                w = uniformTextureWidth;
                h = static_cast<int>(uniformTextureWidth / scaling);
            }
        } else {
            //显示全部内容(内容小于view)
            if (s > 1.0) {
                w = uniformTextureWidth;
                h = static_cast<int>(uniformTextureWidth / scaling);
            } else {
                h = uniformTextureHeight;
                w = static_cast<int>(uniformTextureHeight * scaling);
            }
        }
        glViewport((uniformTextureWidth - w) / 2,
                   (uniformTextureHeight - h) / 2, w, h);
        baseProgram->setTextureId(textureInfo->textureID);
        baseProgram->draw();
        uniformSizeFrameBufferUtils->unbindFrameBuffer();

        textureInfo->textureID = uniformSizeFrameBufferUtils->getBuffersTextureId();
        textureInfo->textureWidth = uniformTextureWidth;
        textureInfo->textureHeight = uniformTextureHeight;
    }
    return textureInfo;
}

void VideoTextureManger::setTextureHandleInfo(TextureHandleInfo *textureHandleInfo) {
    if (nullptr != this->textureHandleInfo) {
        delete (this->textureHandleInfo);
        this->textureHandleInfo = nullptr;
    }
    this->textureHandleInfo = textureHandleInfo;
    setGaussBlurProgramRadius(textureHandleInfo->gaussBlurRadius);
}

void VideoTextureManger::setGaussBlurProgramRadius(float gaussBlurRadius) {
    this->gaussBlurRadius = gaussBlurRadius;
}
