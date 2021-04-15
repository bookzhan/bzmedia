//
/**
 * Created by bookzhan on 2019-01-22 14:24.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include "BZRenderEngine.h"


BZRenderEngine::BZRenderEngine() {
    videoTextureManger = new VideoTextureManger();
}

void BZRenderEngine::setFlip(bool flipHorizontal, bool flipVertical) {
    this->flipHorizontal = flipHorizontal;
    this->flipVertical = flipVertical;
    if (nullptr != baseProgram) {
        baseProgram->setFlip(flipHorizontal, flipVertical);
    }
    if (nullptr != textureYUVProgram) {
        textureYUVProgram->setFlip(flipHorizontal, flipVertical);
    }
}

void BZRenderEngine::setRotation(int rotation) {
    this->rotation = rotation;
    if (nullptr != baseProgram) {
        baseProgram->setRotation(rotation);
    }
    if (nullptr != textureYUVProgram) {
        textureYUVProgram->setRotation(rotation);
    }
}

int BZRenderEngine::setTextureId(int textureId) {
    if (nullptr == baseProgram) {
        baseProgram = new BaseProgram();
        baseProgram->setRotation(rotation);
        baseProgram->setFlip(flipHorizontal, flipVertical);
        baseProgram->init();
    }
    baseProgram->setTextureId(textureId);
    return 0;
}

void BZRenderEngine::setTextureId(int textureIdY, int textureIdUV) {
    if (nullptr == textureYUVProgram) {
        textureYUVProgram = new TextureYUVProgram();
        textureYUVProgram->setRotation(rotation);
        textureYUVProgram->setFlip(flipHorizontal, flipVertical);
        textureYUVProgram->init();
    }
    textureYUVProgram->setTextureId(textureIdY, textureIdUV);
}


int BZRenderEngine::draw() {
    if (nullptr == frameBufferUtils) {
        frameBufferUtils = new FrameBufferUtils();
        frameBufferUtils->initFrameBuffer(textureWidth, textureHeight);
    }
    glViewport(0, 0, textureWidth, textureHeight);
    frameBufferUtils->bindFrameBuffer();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (nullptr != baseProgram) {
        baseProgram->draw();
    }
    if (nullptr != textureYUVProgram) {
        textureYUVProgram->draw();
    }
    frameBufferUtils->unbindFrameBuffer();

    if (finalWidth > 0 && finalHeight > 0) {
        textureInfo.textureID = frameBufferUtils->getBuffersTextureId();
        textureInfo.textureWidth = frameBufferUtils->getWidth();
        textureInfo.textureHeight = frameBufferUtils->getHeight();
        TextureInfo *textureInfoRet = videoTextureManger->getUniformTexture(&textureInfo);
        return textureInfoRet->textureID;
    }
    return frameBufferUtils->getBuffersTextureId();
}

int BZRenderEngine::releaseResource() {
    if (nullptr != baseProgram) {
        baseProgram->releaseResource();
        delete (baseProgram);
        baseProgram = nullptr;
    }
    if (nullptr != textureYUVProgram) {
        textureYUVProgram->releaseResource();
        delete (textureYUVProgram);
        textureYUVProgram = nullptr;
    }
    if (nullptr != frameBufferUtils) {
        frameBufferUtils->releaseFrameBuffer();
        delete (frameBufferUtils);
        frameBufferUtils = nullptr;
    }
    if (nullptr != videoTextureManger) {
        videoTextureManger->release();
        delete (videoTextureManger);
        videoTextureManger = nullptr;
    }
    return 0;
}

void BZRenderEngine::setFinalSize(int finalWidth, int finalHeight) {
    this->finalWidth = finalWidth;
    this->finalHeight = finalHeight;
    if (nullptr != videoTextureManger) {
        videoTextureManger->setUniformTextureSize(finalWidth, finalHeight);
    }
}

void BZRenderEngine::setTextureSize(int textureWidth, int textureHeight) {
    if (rotation == 90 || rotation == 270) {
        int temp = textureWidth;
        textureWidth = textureHeight;
        textureHeight = temp;
    }
    if (nullptr != frameBufferUtils && (this->textureWidth != textureWidth ||
                                        this->textureHeight != textureHeight)) {
        frameBufferUtils->releaseFrameBuffer();
        delete (frameBufferUtils);
        frameBufferUtils = nullptr;
        BZLogUtil::logV("setTextureSize will new FrameBufferUtils");
    }
    this->textureWidth = textureWidth;
    this->textureHeight = textureHeight;
    if (nullptr == frameBufferUtils) {
        frameBufferUtils = new FrameBufferUtils();
        frameBufferUtils->initFrameBuffer(this->textureWidth, this->textureHeight);
    }
}

void BZRenderEngine::setTextureHandleInfo(TextureHandleInfo *textureHandleInfo) {
    if (nullptr != videoTextureManger) {
        videoTextureManger->setTextureHandleInfo(textureHandleInfo);
    }
}

BZRenderEngine::~BZRenderEngine() {
    if (nullptr != videoTextureManger) {
        videoTextureManger->release();
        delete (videoTextureManger);
        videoTextureManger = nullptr;
    }
}

