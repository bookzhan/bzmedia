
/**
 * Created by zhandalin on 2017-06-05 15:07.
 * 说明:
 */


#include <cstring>
#include <common/BZLogUtil.h>
#include "TextureUtil.h"

const float TextureUtil::TEXTURE_NO_ROTATION[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f};
const float  TextureUtil::TEXTURE_ROTATED_90[8] = {
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f};
//这个就相当于TEXTURE_NO_ROTATION,上下,左右都翻转
const float  TextureUtil::TEXTURE_ROTATED_180[8] = {
        1.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f};
const float  TextureUtil::TEXTURE_ROTATED_270[8] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f};

const float TextureUtil::CUBE[8] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f};

const float *
TextureUtil::getRotationTexture(int rotation, bool flipHorizontal, bool flipVertical) {
    size_t len = sizeof(float) * 8;
    float *rotatedTex = static_cast<float *>(malloc(len));
    switch (rotation) {
        case 90:
            memcpy(rotatedTex, TEXTURE_ROTATED_90, len);
            break;
        case 180:
            memcpy(rotatedTex, TEXTURE_ROTATED_180, len);
            break;
        case 270:
            memcpy(rotatedTex, TEXTURE_ROTATED_270, len);
            break;
        default:
            memcpy(rotatedTex, TEXTURE_NO_ROTATION, len);
            break;
    }
    if (flipHorizontal) {
        rotatedTex[0] = flip(rotatedTex[0]);
        rotatedTex[2] = flip(rotatedTex[2]);
        rotatedTex[4] = flip(rotatedTex[4]);
        rotatedTex[6] = flip(rotatedTex[6]);
    }
    if (flipVertical) {
        rotatedTex[1] = flip(rotatedTex[1]);
        rotatedTex[3] = flip(rotatedTex[3]);
        rotatedTex[5] = flip(rotatedTex[5]);
        rotatedTex[7] = flip(rotatedTex[7]);
    }
    return rotatedTex;
}

const GLuint
TextureUtil::initProgram(const char *pVertexSource, const char *pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }
    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }
    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char *buf = (char *) malloc((size_t) bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    BZLogUtil::logE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint TextureUtil::loadShader(GLenum shaderType, const char *pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char *buf = (char *) malloc((size_t) infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    BZLogUtil::logE("Could not compile shader %d:\n%s\n",
                                    shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

