//
/**
 * Created by bookzhan on 2017-10-12 14:37.
 * 说明:
 */
//

#include <GLES2/gl2.h>
#include <cstring>
#include "BaseMatrixProgram.h"

int BaseMatrixProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    int program = BaseProgram::initProgram(vertexShader, fragmentShader);
    vMatrixLocation = glGetUniformLocation((GLuint) program, "vMatrix");

    return program;
}

int BaseMatrixProgram::drawArraysBefore() {
    BaseProgram::drawArraysBefore();
    if (matrix != nullptr) {
        glUniformMatrix4fv(vMatrixLocation, 1, GL_FALSE, matrix);
    }
    return 0;
}

void BaseMatrixProgram::setMatrix(float *matrix) {
    memcpy(this->matrix, matrix, 16 * sizeof(float));
}

BaseMatrixProgram::BaseMatrixProgram() {
    VERTEX_SHADER = "attribute vec4 vPosition;\n"
                    "attribute vec2 inputTextureCoordinate;\n"
                    "varying vec2 textureCoordinate;\n"
                    "uniform mat4 vMatrix;\n"
                    "void main()\n"
                    "{\n"
                    "    gl_Position = vMatrix*vPosition;\n"
                    "    textureCoordinate = inputTextureCoordinate;\n"
                    "}";
    FRAGMENT_SHADER = "precision mediump float;\n"
                      "varying vec2 textureCoordinate;\n"
                      "uniform sampler2D inputImageTexture;\n"
                      "void main() {\n"
                      "    gl_FragColor =texture2D(inputImageTexture, textureCoordinate);\n"
                      "}";
}

