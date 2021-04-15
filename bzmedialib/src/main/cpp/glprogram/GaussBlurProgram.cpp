//
/**
 * Created by bookzhan on 2018-08-13 11:11.
 * 说明:
 */
//

#include <GLES2/gl2.h>
#include "GaussBlurProgram.h"

GaussBlurProgram::GaussBlurProgram() {
    VERTEX_SHADER = "attribute vec4 vPosition;\n"
                    "attribute vec2 inputTextureCoordinate;\n"
                    "\n"
                    "uniform int flipHorizontal;\n"
                    "uniform int flipVertical;\n"
                    "uniform float texelWidthOffset;\n"
                    "uniform float texelHeightOffset;\n"
                    "\n"
                    "varying vec2 blurCoordinates[5];\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "        gl_Position = vPosition;\n"
                    "\n"
                    "        vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);\n"
                    "       vec2 textureCoordinate=inputTextureCoordinate;"
                    "       if(flipHorizontal==1)\n"
                    "           textureCoordinate.x=1.0-textureCoordinate.x;\n"
                    "       if(flipVertical==1)\n"
                    "           textureCoordinate.y=1.0-textureCoordinate.y;\n"
                    "        blurCoordinates[0] = textureCoordinate.xy;\n"
                    "        blurCoordinates[1] = textureCoordinate.xy + singleStepOffset * 1.407333;\n"
                    "        blurCoordinates[2] = textureCoordinate.xy - singleStepOffset * 1.407333;\n"
                    "        blurCoordinates[3] = textureCoordinate.xy + singleStepOffset * 3.294215;\n"
                    "        blurCoordinates[4] = textureCoordinate.xy - singleStepOffset * 3.294215;\n"
                    "}";

    FRAGMENT_SHADER = "precision mediump float;"
                      "uniform sampler2D inputImageTexture;\n"
                      "uniform mediump float texelWidthOffset;\n"
                      "uniform mediump float texelHeightOffset;\n"
                      "\n"
                      "varying mediump vec2 blurCoordinates[5];\n"
                      "\n"
                      "void main()\n"
                      "{\n"
                      "        lowp vec4 sum = vec4(0.0);\n"
                      "        sum += texture2D(inputImageTexture, blurCoordinates[0]) * 0.204164;\n"
                      "        sum += texture2D(inputImageTexture, blurCoordinates[1]) * 0.304005;\n"
                      "        sum += texture2D(inputImageTexture, blurCoordinates[2]) * 0.304005;\n"
                      "        sum += texture2D(inputImageTexture, blurCoordinates[3]) * 0.093913;\n"
                      "        sum += texture2D(inputImageTexture, blurCoordinates[4]) * 0.093913;\n"
                      "        gl_FragColor = sum;\n"
                      "}";
}

int GaussBlurProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    int program = BaseProgram::initProgram(vertexShader, fragmentShader);
    texelWidthOffsetLocation = glGetUniformLocation((GLuint) program, "texelWidthOffset");
    texelHeightLocation = glGetUniformLocation((GLuint) program, "texelHeightOffset");

    return program;
}

void GaussBlurProgram::setDrawHorizontal(bool drawHorizontal) {
    if (mProgram > 0 && viewPortWidth > 0 && viewPortHeight > 0) {
        glUseProgram(mProgram);
        if (drawHorizontal) {
            glUniform1f(texelWidthOffsetLocation, radius / viewPortWidth);
            glUniform1f(texelHeightLocation, 0);
        } else {
            glUniform1f(texelWidthOffsetLocation, 0);
            glUniform1f(texelHeightLocation, radius / viewPortHeight);
        }
        glUseProgram(0);
    }
}

void GaussBlurProgram::setRadius(float radius) {
    this->radius = radius;
}

int GaussBlurProgram::draw() {
    int count = static_cast<int>(radius);
    if (count < 1) {
        count = 1;
    }
    for (int i = 0; i < count; ++i) {
        frameBufferUtils_1->bindFrameBuffer();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (i == 0) {
            glViewport(finalViewPortX, finalViewPortY, finalViewPortWidth, finalViewPortHeight);
        } else {
            glViewport(0, 0, viewPortWidth, viewPortHeight);
        }
        setDrawHorizontal(true);
        BaseProgram::draw();
        frameBufferUtils_1->unbindFrameBuffer();

        setTextureId(frameBufferUtils_1->getBuffersTextureId());

        frameBufferUtils_2->bindFrameBuffer();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, viewPortWidth, viewPortHeight);
        setDrawHorizontal(false);
        BaseProgram::draw();
        frameBufferUtils_2->unbindFrameBuffer();

        setTextureId(frameBufferUtils_2->getBuffersTextureId());
    }
    return frameBufferUtils_2->getBuffersTextureId();
}


int GaussBlurProgram::releaseResource() {
    if (nullptr != frameBufferUtils_1) {
        frameBufferUtils_1->releaseFrameBuffer();
        delete (frameBufferUtils_1);
        frameBufferUtils_1 = nullptr;
    }
    if (nullptr != frameBufferUtils_2) {
        frameBufferUtils_2->releaseFrameBuffer();
        delete (frameBufferUtils_2);
        frameBufferUtils_2 = nullptr;
    }
    return BaseProgram::releaseResource();
}

int GaussBlurProgram::setSize(int textureWidth, int textureHeight, int viewPortWidth,
                              int viewPortHeight, bool fitFullViewPort) {

    if (this->viewPortWidth != viewPortWidth || this->viewPortHeight != viewPortHeight) {
        if (nullptr != frameBufferUtils_1) {
            frameBufferUtils_1->releaseFrameBuffer();
            delete (frameBufferUtils_1);
            frameBufferUtils_1 = nullptr;
        }
        if (nullptr != frameBufferUtils_2) {
            frameBufferUtils_2->releaseFrameBuffer();
            delete (frameBufferUtils_2);
            frameBufferUtils_2 = nullptr;
        }

        frameBufferUtils_1 = new FrameBufferUtils();
        frameBufferUtils_1->initFrameBuffer(viewPortWidth, viewPortHeight);

        frameBufferUtils_2 = new FrameBufferUtils();
        frameBufferUtils_2->initFrameBuffer(viewPortWidth, viewPortHeight);
    }
    this->textureWidth = textureWidth;
    this->textureHeight = textureHeight;
    this->viewPortWidth = viewPortWidth;
    this->viewPortHeight = viewPortHeight;
    this->fitFullViewPort = fitFullViewPort;


    float scaling = textureWidth / (float) textureHeight;;

    float viewRatio = viewPortWidth / (float) viewPortHeight;
    float s = scaling / viewRatio;

    int w, h;

    if (fitFullViewPort) {
        //撑满全部view(内容大于view)
        if (s > 1.0) {
            w = static_cast<int>(viewPortHeight * scaling);
            h = viewPortHeight;
        } else {
            w = viewPortWidth;
            h = static_cast<int>(viewPortWidth / scaling);
        }
    } else {
        //显示全部内容(内容小于view)
        if (s > 1.0) {
            w = viewPortWidth;
            h = static_cast<int>(viewPortWidth / scaling);
        } else {
            h = viewPortHeight;
            w = static_cast<int>(viewPortHeight * scaling);
        }
    }
    //需要转换成2的整数倍
    finalViewPortWidth = w;
    finalViewPortHeight = h;
    finalViewPortX = (viewPortWidth - finalViewPortWidth) / 2;
    finalViewPortY = (viewPortHeight - finalViewPortHeight) / 2;
    return 0;
}

