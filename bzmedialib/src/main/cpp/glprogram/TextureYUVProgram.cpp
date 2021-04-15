//
/**
 * Created by bookzhan on 2019-01-21 17:43.
 * 说明:
 */
//

#include <GLES2/gl2.h>
#include "TextureYUVProgram.h"

TextureYUVProgram::TextureYUVProgram() {
    FRAGMENT_SHADER = "precision mediump float;\n"
                      "varying vec2 textureCoordinate;\n"
                      "uniform sampler2D inputImageTexture;\n"
                      "uniform sampler2D tex_uv;\n"
                      "const mat3 yuv2rgb = mat3(\n"
                      "            1.164,1.164,1.164,\n"
                      "            0.0,-0.392,2.017,\n"
                      "            1.596,-0.813,0.0\n"
                      "            );\n"
                      "void main() {\n"
                      "    vec3 yuv;\n"
                      "    yuv.x = texture2D(inputImageTexture, textureCoordinate).r - (16.0 / 255.0);\n"
                      "    yuv.yz = texture2D(tex_uv, textureCoordinate).ra - vec2(0.5, 0.5);\n"
                      "    vec3 videoColor = yuv2rgb * yuv;\n"
                      "    gl_FragColor =vec4(videoColor.rgb, 1.0);\n"
                      "}";
}

int TextureYUVProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    int program = BaseProgram::initProgram(vertexShader, fragmentShader);

    GLint tex_uvLocation = glGetUniformLocation(static_cast<GLuint>(program), "tex_uv");

    glUseProgram(static_cast<GLuint>(program));
    glUniform1i(tex_uvLocation, 1);
    glUseProgram(0);
    return program;
}

int TextureYUVProgram::drawArraysBefore() {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(textureIdUV));
    return BaseProgram::drawArraysBefore();
}

void TextureYUVProgram::setTextureId(int textureIdY, int textureIdUV) {
    BaseProgram::setTextureId(textureIdY);
    this->textureIdUV = textureIdUV;
}
