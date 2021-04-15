//
/**
*Created by bookzhan on {YEAR}-YEAR−{MONTH}-04 {HOUR}:HOUR:{MINUTE}.
*description:
*/
//

#include <GLES2/gl2.h>
#include <glutils/FrameBufferUtils.h>
#include <cstring>
#include "ZoomProgram.h"

ZoomProgram::ZoomProgram() {
    FRAGMENT_SHADER ="precision highp float;\n"
                     "varying vec2 textureCoordinate;\n"
                     "uniform sampler2D inputImageTexture;\n"
                     "uniform vec2 textureSize;\n"
                     "uniform vec2 circleCenter;\n"
                     "uniform float radius;\n"
                     "uniform float zoomRatio;\n"
                     "vec2 warpEyes(vec2 centerPos, vec2 curPos, float radius, float scaleRatio)\n"
                     "{\n"
                     "    vec2 result = curPos;\n"
                     "    vec2 imgCurPos = curPos * textureSize;\n"
                     "    float d = distance(imgCurPos, centerPos);\n"
                     "\n"
                     "    if (d < radius)\n"
                     "    {\n"
                     "        result = centerPos + (1.0/scaleRatio) * (imgCurPos - centerPos);\n"
                     "        result = result / textureSize;\n"
                     "\n"
                     "    }\n"
                     "    return result;\n"
                     "\n"
                     "}\n"
                     "void main()\n"
                     "{\n"
                     "    gl_FragColor = texture2D(inputImageTexture, warpEyes(circleCenter, textureCoordinate, radius, zoomRatio));\n"
                     "}";
}

int ZoomProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    int program = BaseProgram::initProgram(vertexShader, fragmentShader);
    textureSizeLoc = glGetUniformLocation(program, "textureSize");
    circleCenterLoc = glGetUniformLocation(program, "circleCenter");
    radiusLoc = glGetUniformLocation(program, "radius");
    zoomRatioLoc = glGetUniformLocation(program, "zoomRatio");
    return program;
}

int ZoomProgram::drawArraysBefore() {
    int ret = BaseProgram::drawArraysBefore();
    glUniform2f(textureSizeLoc, (float) textureWidth, (float) textureHeight);
    glUniform2f(circleCenterLoc, centerPointX,
                centerPointY);
    glUniform1f(radiusLoc, radius);
    glUniform1f(zoomRatioLoc, zoomRatio);
    return ret;
}

void ZoomProgram::setCenterPoint(float x, float y) {
    this->centerPointX = x;
    this->centerPointY = y;
}

void ZoomProgram::setRadius(float radius) {
    this->radius = radius;
}

void ZoomProgram::setTextureSize(int width, int height) {
    this->textureWidth = width;
    this->textureHeight = height;

}

void ZoomProgram::setZoomRatio(float zoomRatio) {
    this->zoomRatio = zoomRatio;
}

