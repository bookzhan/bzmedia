//
/**
 * Created by bookzhan on 2018-11-08 19:03.
 * 说明:
 */
//
#include <common/BZLogUtil.h>
#include <common/GLUtil.h>
#include <GLES2/gl2.h>
#include <common/bz_time.h>
#include "ImageExternalProgram.h"

ImageExternalProgram::ImageExternalProgram(bool needFadeShow) {
    VERTEX_SHADER = "attribute vec4 vPosition;\n"
                    "attribute vec2 inputTextureCoordinate;\n"
                    "varying vec2 textureCoordinate;\n"
                    "void main()\n"
                    "{\n"
                    "    gl_Position = vPosition;\n"
                    "    textureCoordinate = inputTextureCoordinate;\n"
                    "}";
    FRAGMENT_SHADER = "#extension GL_OES_EGL_image_external : require\n"
                      "precision mediump float;\n"
                      "varying vec2 textureCoordinate;\n"
                      "uniform float alpha;\n"
                      "uniform samplerExternalOES inputImageTexture;\n"
                      "void main() {\n"
                      "    vec4 materialColor =texture2D(inputImageTexture, textureCoordinate);\n"
                      "    gl_FragColor = vec4(materialColor.rgb, materialColor.a * alpha);"
                      "}";
    this->needFadeShow = needFadeShow;
}


int ImageExternalProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    int program = BaseProgram::initProgram(vertexShader, fragmentShader);
    alphaLocation = glGetUniformLocation((GLuint) program, "alpha");
    return program;
}

int ImageExternalProgram::initImageExternal() {
    glGenTextures(1, &baseTextureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, baseTextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return baseTextureId;
}

int ImageExternalProgram::releaseResource() {
    BZLogUtil::logD("ImageExternalProgram::releaseResource()");
    GLUtil::checkGlError("ImageExternalProgram::releaseResource start");
    if (baseTextureId > 0) {
        glDeleteTextures(1, &baseTextureId);
        baseTextureId = 0;
    }
    int ret = BaseProgram::releaseResource();
    GLUtil::checkGlError("ImageExternalProgram::releaseResource end");
    return ret;
}

int ImageExternalProgram::drawArraysBefore() {
    BaseProgram::drawArraysBefore();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, baseTextureId);
    if (needFadeShow) {
        if (firstDrawTime <= 0) {
            firstDrawTime = getCurrentTime();
        }
        //200ms的动画
        alpha = (getCurrentTime() - firstDrawTime) / 200.0f;
        if (alpha > 1)alpha = 1;
        if (alpha < 0)alpha = 0;
        glUniform1f(alphaLocation, alpha);
    } else {
        glUniform1f(alphaLocation, 1);
    }
    return 0;
}

int ImageExternalProgram::draw() {
    if (needFadeShow) {
        glGetBooleanv(GL_BLEND, reinterpret_cast<GLboolean *>(&lastEnableBlendState));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    BaseProgram::draw();
    if (needFadeShow && !lastEnableBlendState) {
        glDisable(GL_BLEND);
    }
    return baseTextureId;
}
