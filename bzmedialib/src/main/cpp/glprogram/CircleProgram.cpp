//
/**
 * Created by bookzhan on 2018-05-10 15:56.
 * 说明:
 */
//

#include <GLES2/gl2.h>
#include "CircleProgram.h"

CircleProgram::CircleProgram() {
    FRAGMENT_SHADER = "precision mediump float;\n"
            "uniform sampler2D inputImageTexture;\n"
            "uniform float width;\n"
            "uniform float height;\n"
            "varying vec2 textureCoordinate;\n"
            "vec4 color;\n"
            "void main() {\n"
            "vec2 centerPoint=textureCoordinate-vec2(0.5);\n"
            "float length=1.0;\n"
            "float hwRate=width>height?width/height:height/width;\n"
            "hwRate=hwRate*hwRate;\n"
            "if(width>height){\n"
            "    length=hwRate*centerPoint.x*centerPoint.x+centerPoint.y*centerPoint.y;\n"
            "}else{\n"
            "    length=centerPoint.x*centerPoint.x+hwRate*centerPoint.y*centerPoint.y;\n"
            "}\n"
            "if(length>0.25){\n"
            "        color = vec4(vec3(1.0),0.0);\n"
            "    }else{\n"
            "        color=texture2D(inputImageTexture, textureCoordinate);\n"
            "    }\n"
            "  gl_FragColor = color;\n"
            "}";
}

int CircleProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    int program = BaseProgram::initProgram(vertexShader, fragmentShader);
    widthLoc = glGetUniformLocation(program, "width");
    heightLoc = glGetUniformLocation(program, "height");
    return program;
}

int CircleProgram::drawArraysBefore() {
    int ret = BaseProgram::drawArraysBefore();
    glUniform1f(widthLoc, width);
    glUniform1f(heightLoc, height);
    return ret;
}

void CircleProgram::setTextureSize(float width, float height) {
    this->width = width;
    this->height = height;
}
