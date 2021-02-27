//
/**
 * Created by zhandalin on 2017-10-12 16:00.
 * 说明:
 */
//

#include <GLES2/gl2.h>
#include "BaseParticleProgram.h"

int BaseParticleProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    int program = BaseMatrixProgram::initProgram(vertexShader, fragmentShader);
    alphaLocation = glGetUniformLocation((GLuint) program, "alpha");
    return program;
}

void BaseParticleProgram::setAlpha(float alpha) {
    this->alpha = alpha;
}

int BaseParticleProgram::drawArraysBefore() {
    BaseMatrixProgram::drawArraysBefore();
    glUniform1f(alphaLocation, alpha);
    return 0;
}

BaseParticleProgram::BaseParticleProgram() {
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
            "uniform float alpha;\n"
            "void main() {\n"
            "    vec4 temp=texture2D(inputImageTexture, textureCoordinate/4.0);\n"
            "    gl_FragColor =vec4(temp.rgb*alpha,1.0);\n"
            "}";
}
