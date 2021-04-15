//
/**
 * Created by bookzhan on 2022-03-06 09:14.
 *description:
 */
//

#include <GLES2/gl2.h>
#include "MosaicProgram.h"

MosaicProgram::MosaicProgram() {
    FRAGMENT_SHADER = "precision highp float;\n"
                      "varying vec2 textureCoordinate;\n"
                      "uniform sampler2D inputImageTexture;\n"
                      "uniform vec2 textureSize;\n"
                      "uniform vec2 mosaicSize;\n"
                      "uniform vec2 circleCenter;\n"
                      "uniform float radius;\n"
                      "uniform int mosaicType;\n"
                      "uniform float rectangleWidth;\n"
                      "uniform float rectangleHeight;\n"
                      "\n"
                      "bool isMosaicArea(vec2 intXY){\n"
                      "    if (mosaicType==0){\n"
                      "        float temp_distance=distance(circleCenter, intXY);\n"
                      "        return temp_distance<radius;\n"
                      "    } else if (mosaicType==1){\n"
                      "        vec2 f1=vec2(circleCenter.x-radius, circleCenter.y);\n"
                      "        vec2 f2=vec2(circleCenter.x+radius, circleCenter.y);\n"
                      "        float value_a=1.2*radius;\n"
                      "        float distance_1=distance(f1, intXY);\n"
                      "        float distance_2=distance(f2, intXY);\n"
                      "        return distance_1+distance_2<2.0*value_a;\n"
                      "    } else if (mosaicType==2){\n"
                      "        vec2 f1=vec2(circleCenter.x, circleCenter.y-radius);\n"
                      "        vec2 f2=vec2(circleCenter.x, circleCenter.y+radius);\n"
                      "        float value_a=1.2*radius;\n"
                      "        float distance_1=distance(f1, intXY);\n"
                      "        float distance_2=distance(f2, intXY);\n"
                      "        return distance_1+distance_2<2.0*value_a;\n"
                      "    } else if (mosaicType==3){\n"
                      "        bool inXArea=intXY.x>circleCenter.x-rectangleWidth/2.0&&intXY.x<circleCenter.x+rectangleWidth/2.0;\n"
                      "        bool inYArea=intXY.y>circleCenter.y-rectangleHeight/2.0&&intXY.y<circleCenter.y+rectangleHeight/2.0;\n"
                      "        return inXArea&&inYArea;\n"
                      "    }\n"
                      "    return false;\n"
                      "}\n"
                      "\n"
                      "void main()\n"
                      "{\n"
                      "    vec2 intXY = vec2(textureCoordinate.x*textureSize.x, textureCoordinate.y*textureSize.y);\n"
                      "    if (isMosaicArea(intXY)){\n"
                      "        vec2 XYMosaic = vec2(floor(intXY.x/mosaicSize.x)*mosaicSize.x, floor(intXY.y/mosaicSize.y)*mosaicSize.y);\n"
                      "        vec2 UVMosaic = vec2(XYMosaic.x/textureSize.x, XYMosaic.y/textureSize.y);\n"
                      "        vec4 color = texture2D(inputImageTexture, UVMosaic);\n"
                      "        gl_FragColor = color;\n"
                      "    } else {\n"
                      "        gl_FragColor = texture2D(inputImageTexture, textureCoordinate);\n"
                      "    }\n"
                      "}";

}

int MosaicProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    int program = BaseProgram::initProgram(vertexShader, fragmentShader);
    textureSizeLoc = glGetUniformLocation(program, "textureSize");
    mosaicSizeLoc = glGetUniformLocation(program, "mosaicSize");
    circleCenterLoc = glGetUniformLocation(program, "circleCenter");
    radiusLoc = glGetUniformLocation(program, "radius");
    mosaicTypeLoc = glGetUniformLocation(program, "mosaicType");
    rectangleWidthLoc = glGetUniformLocation(program, "rectangleWidth");
    rectangleHeightLoc = glGetUniformLocation(program, "rectangleHeight");
    return program;
}

int MosaicProgram::drawArraysBefore() {
    int ret = BaseProgram::drawArraysBefore();
    glUniform2f(textureSizeLoc, (float)textureWidth, (float)textureHeight);
    glUniform2f(mosaicSizeLoc, mosaicWidth, mosaicHeight);
    glUniform2f(circleCenterLoc, circleCenterX, circleCenterY);
    glUniform1f(radiusLoc, radius);
    glUniform1i(mosaicTypeLoc, mosaicType);
    glUniform1f(rectangleWidthLoc, (float) rectangleWidth);
    glUniform1f(rectangleHeightLoc, (float) rectangleHeight);
    return ret;
}

void MosaicProgram::setTextureSize(int width, int height) {
    this->textureWidth = width;
    this->textureHeight = height;
}

void MosaicProgram::setMosaicSize(float width, float height) {
    this->mosaicWidth = width;
    this->mosaicHeight = height;
}

void MosaicProgram::setCenterPoint(float x, float y) {
    this->circleCenterX = x;
    this->circleCenterY = y;
}

void MosaicProgram::setRadius(float radius) {
    this->radius = radius;
}

void MosaicProgram::setMosaicType(int mosaicType) {
    this->mosaicType = mosaicType;
}

void MosaicProgram::setRectangleSize(int width, int height) {
    this->rectangleWidth = width;
    this->rectangleHeight = height;
}


