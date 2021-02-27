//
/**
 * Created by zhandalin on 2017-10-11 18:10.
 * 说明:FrameBuffer 的封装
 */
//

#include <GLES3/gl3.h>
#include <clocale>
#include <common/GLUtil.h>
#include "FrameBufferUtils.h"

int FrameBufferUtils::initFrameBuffer(int width, int height) {
    this->width = width;
    this->height = height;
    glGenFramebuffers(1, &frameBuffer);
    glGenTextures(1, &frameBuffersTextureId);
    glBindTexture(GL_TEXTURE_2D, frameBuffersTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, frameBuffersTextureId, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return frameBuffersTextureId;
}


int FrameBufferUtils::releaseFrameBuffer() {
    GLUtil::checkGlError("FrameBufferUtils::releaseFrameBuffer start");
    if (frameBuffer > 0 && glIsFramebuffer(frameBuffer)) {
        glDeleteFramebuffers(1, &frameBuffer);
        frameBuffer = 0;
    }
    if (frameBuffersTextureId > 0 && glIsTexture(frameBuffersTextureId)) {
        glDeleteTextures(1, &frameBuffersTextureId);
        frameBuffersTextureId = 0;
    }
    GLUtil::checkGlError("FrameBufferUtils::releaseFrameBuffer end");
    return 0;
}

int FrameBufferUtils::bindFrameBuffer() {
    //找出当前绑定的FrameBuffer,最后需要还原的
    lastBindFrameBuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastBindFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
//    glViewport(0, 0, width, height);
    return 0;
}

int FrameBufferUtils::unbindFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (lastBindFrameBuffer > 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(lastBindFrameBuffer));
    }
    return 0;
}

int FrameBufferUtils::getBuffersTextureId() {
    return frameBuffersTextureId;
}

int FrameBufferUtils::getWidth() {
    return width;
}

int FrameBufferUtils::getHeight() {
    return height;
}
