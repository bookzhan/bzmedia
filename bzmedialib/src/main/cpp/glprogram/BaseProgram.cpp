//
/**
 * Created by bookzhan on 2017-10-11 18:57.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include <glutils/TextureUtil.h>
#include "BaseProgram.h"


int BaseProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    mProgram = TextureUtil::initProgram(vertexShader, fragmentShader);

    glGenBuffers(1, &coordinateBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, coordinateBuffer);
    const float*buffer= TextureUtil::getRotationTexture(rotation, flipHorizontal, flipVertical);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_COOD),
                 buffer,
                 GL_STATIC_DRAW);
    free((void*)buffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEXT_COOD),
                 VERTEXT_COOD, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vPositionLocation = glGetAttribLocation(mProgram, "vPosition");
    BZLogUtil::logV("glGetAttribLocation(\"vPosition\") = %d\n",
         vPositionLocation);

    inputTextureCoordinateLocation = glGetAttribLocation(mProgram, "inputTextureCoordinate");
    BZLogUtil::logV("glGetAttribLocation(\"inputTextureCoordinate\") = %d\n",
         inputTextureCoordinateLocation);
    GLint inputImageTextureLocation = glGetUniformLocation(mProgram, "inputImageTexture");

    glUseProgram(mProgram);
    glUniform1i(inputImageTextureLocation, 0);
    glUseProgram(0);
    return mProgram;
}


int BaseProgram::setTextureId(int textureId) {
    this->textureId = (GLuint) textureId;
    return 0;
}

int BaseProgram::draw() {
    if (mProgram <= 0) {
        initProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    }
//在同一个 draw 不能清除, 要清除就在外面清除
//    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glUseProgram(mProgram);
    glEnableVertexAttribArray((GLuint) vPositionLocation);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glVertexAttribPointer((GLuint) vPositionLocation, COORDS_PER_VERTEX, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE,
                          0);
    //一定要解绑,防止影响后面的使用
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray((GLuint) inputTextureCoordinateLocation);
    glBindBuffer(GL_ARRAY_BUFFER, coordinateBuffer);
    glVertexAttribPointer((GLuint) inputTextureCoordinateLocation, COORDS_PER_VERTEX, GL_FLOAT,
                          GL_FALSE, VERTEX_STRIDE, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);

    drawArraysBefore();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray((GLuint) vPositionLocation);
    glDisableVertexAttribArray((GLuint) inputTextureCoordinateLocation);
    glUseProgram(0);

    drawArraysAfter();
    return textureId;
}

int BaseProgram::releaseResource() {
    if (mProgram != 0) {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
    if (positionBuffer > 0) {
        glDeleteBuffers(1, &positionBuffer);
        positionBuffer = 0;
    }
    if (coordinateBuffer > 0) {
        glDeleteBuffers(1, &coordinateBuffer);
        coordinateBuffer = 0;
    }
    return 0;
}

int BaseProgram::drawArraysBefore() {
    return 0;
}

int BaseProgram::drawArraysAfter() {
    return 0;
}

BaseProgram::BaseProgram() {
    VERTEX_SHADER = "attribute vec4 vPosition;\n"
            "attribute vec2 inputTextureCoordinate;\n"
            "varying vec2 textureCoordinate;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vPosition;\n"
            "    textureCoordinate = inputTextureCoordinate;\n"
            "}";
    FRAGMENT_SHADER = "precision mediump float;\n"
            "varying vec2 textureCoordinate;\n"
            "uniform sampler2D inputImageTexture;\n"
            "void main() {\n"
            "    gl_FragColor =texture2D(inputImageTexture, textureCoordinate);\n"
            "}";
}

void BaseProgram::setRotation(int rotation) {
    this->rotation = rotation;
    updateCoordinateBuffer();
}

void BaseProgram::setFlip(bool flipHorizontal, bool flipVertical) {
    this->flipHorizontal = flipHorizontal;
    this->flipVertical = flipVertical;
    updateCoordinateBuffer();
}

void BaseProgram::init() {
    if (mProgram <= 0) {
        initProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    }
}

void BaseProgram::updateCoordinateBuffer() {
    if (coordinateBuffer > 0) {
        //存放顶点位置数据
        glBindBuffer(GL_ARRAY_BUFFER, coordinateBuffer);
        const float*buffer= TextureUtil::getRotationTexture(rotation, flipHorizontal, flipVertical);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_COOD),
                     buffer,
                     GL_STATIC_DRAW);
        free((void*)buffer);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

