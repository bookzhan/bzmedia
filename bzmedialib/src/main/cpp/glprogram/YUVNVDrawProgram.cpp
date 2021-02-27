//
/**
 * Created by zhandalin on 2018-11-01 14:26.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include <glutils/TextureUtil.h>
#include "YUVNVDrawProgram.h"
#include "../common/GLUtil.h"

YUVNVDrawProgram::YUVNVDrawProgram() {
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
                      "uniform float intensity;\n"
                      "uniform sampler2D tex_y;\n"
                      "uniform sampler2D tex_uv;\n"
                      "const mat3 yuv2rgb = mat3(\n"
                      "            1.164,1.164,1.164,\n"
                      "            0.0,-0.392,2.017,\n"
                      "            1.596,-0.813,0.0\n"
                      "            );\n"
                      "void main() {\n"
                      "    vec3 yuv;\n"
                      "    yuv.x = texture2D(tex_y, textureCoordinate).r - (16.0 / 255.0);\n"
                      "    yuv.y = texture2D(tex_uv, textureCoordinate).r - 0.5;\n"
                      "    yuv.z = texture2D(tex_uv, textureCoordinate).a - 0.5;\n"
                      "    vec3 videoColor = yuv2rgb * yuv;\n"
                      "    gl_FragColor =vec4(videoColor.rgb, 1.0);\n"
                      "}";
}

void YUVNVDrawProgram::init() {
    if (mProgram <= 0) {
        initProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    }
    glGenBuffers(1, &positionBuffer);
    //存放顶点位置数据
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TextureUtil::CUBE),
                 TextureUtil::CUBE, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    updateCoordinateBuffer();
}

int YUVNVDrawProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    GLUtil::checkGlError("start initNVTexture");
    mProgram = TextureUtil::initProgram(vertexShader, fragmentShader);
    if (!mProgram) {
        BZLogUtil::logV("Could not create program.");
        return -1;
    }
    vPositionLocation = (GLuint) glGetAttribLocation(mProgram, "vPosition");
    GLUtil::checkGlError("initNVTexture vPosition");
    BZLogUtil::logV("glGetAttribLocation(\"vPosition\") = %d\n",
                    vPositionLocation);

    inputTextureCoordinateLocation = glGetAttribLocation(mProgram, "inputTextureCoordinate");
    GLUtil::checkGlError("initNVTexture inputTextureCoordinate");
    BZLogUtil::logV("glGetAttribLocation(\"inputTextureCoordinate\") = %d\n",
                    inputTextureCoordinateLocation);

    m_textureYLocation = glGetUniformLocation(mProgram, "tex_y");
    m_textureUVLocation = glGetUniformLocation(mProgram, "tex_uv");

    glGenTextures(1, &textures_y);
    glBindTexture(GL_TEXTURE_2D, textures_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &textures_uv);
    glBindTexture(GL_TEXTURE_2D, textures_uv);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(mProgram);
    glUniform1i(m_textureYLocation,
                0);     //设置纹理，按照前面设置的规则怎样将图像或纹理贴上（参数和选择的活跃纹理单元对应，GL_TEXTURE0）
    glUniform1i(m_textureUVLocation, 1);
    glUseProgram(0);

    GLUtil::checkGlError("initNVTexture finish");
    return mProgram;
}

void YUVNVDrawProgram::setRotation(int rotation) {
    this->rotate = rotation;
    updateCoordinateBuffer();
}

void YUVNVDrawProgram::setFlip(bool flipHorizontal, bool flipVertical) {
    this->needFlipHorizontal = flipHorizontal;
    this->needFlipVertical = flipVertical;
    updateCoordinateBuffer();
}

void YUVNVDrawProgram::updateCoordinateBuffer() {
    if (coordinateBuffer <= 0)
        glGenBuffers(1, &coordinateBuffer);
    //存放顶点位置数据
    glBindBuffer(GL_ARRAY_BUFFER, coordinateBuffer);
    const float*buffer= TextureUtil::getRotationTexture(rotate, needFlipHorizontal, needFlipVertical);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_VERTICES),
                 buffer,
                 GL_STATIC_DRAW);
    free((void*)buffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int YUVNVDrawProgram::releaseResource() {
    BaseYUVDrawProgram::releaseResource();
    GLUtil::checkGlError("AVFrameProgram::releaseResource start");
    if (mProgram > 0 && glIsProgram(mProgram)) {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
    if (positionBuffer > 0 && glIsBuffer(positionBuffer)) {
        glDeleteBuffers(1, &positionBuffer);
        positionBuffer = 0;
    }
    if (coordinateBuffer > 0 && glIsBuffer(coordinateBuffer)) {
        glDeleteBuffers(1, &coordinateBuffer);
        coordinateBuffer = 0;
    }
    if (textures_y > 0 && glIsTexture(textures_y)) {
        glDeleteTextures(1, &textures_y);
        textures_y = 0;
    }
    if (textures_uv > 0 && glIsTexture(textures_uv)) {
        glDeleteTextures(1, &textures_uv);
        textures_uv = 0;
    }
    GLUtil::checkGlError("AVFrameProgram::releaseResource end");
    BZLogUtil::logD("AVFrameProgram releaseResource finish");
    return 0;
}

int YUVNVDrawProgram::draw(AVFrame *inputAVFrame) {
    if (nullptr == inputAVFrame || nullptr == inputAVFrame->linesize
        || nullptr == inputAVFrame->data[0]
        || nullptr == inputAVFrame->data[1]) {
        BZLogUtil::logE("YUVNVDrawProgram::draw AVFrame data Error");
        return -1;
    }
    AVFrame *finalAVFrame = getAlignAVFrame(inputAVFrame);
    if (nullptr == finalAVFrame) {
        return 0;
    }

    if (mProgram <= 0) {
        initProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    }
    glUseProgram(mProgram);

    glEnableVertexAttribArray((GLuint) vPositionLocation);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glVertexAttribPointer((GLuint) vPositionLocation, COORDS_PER_VERTEX, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE,
                          0);
    //一定要解绑,防止影响后面的使用
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (inputTextureCoordinateLocation != -1) {
        glEnableVertexAttribArray((GLuint) inputTextureCoordinateLocation);
        glBindBuffer(GL_ARRAY_BUFFER, coordinateBuffer);
        glVertexAttribPointer((GLuint) inputTextureCoordinateLocation, COORDS_PER_VERTEX, GL_FLOAT,
                              GL_FALSE, VERTEX_STRIDE, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    //Y
    //选择当前活跃的纹理单元
    glActiveTexture(GL_TEXTURE0);
    //允许建立一个绑定到目标纹理的有名称的纹理
    glBindTexture(GL_TEXTURE_2D, textures_y);
    //根据指定的参数，生成一个2D纹理（Texture）。相似的函数还有glTexImage1D、glTexImage3D。
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, finalAVFrame->linesize[0], finalAVFrame->height,
                 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, finalAVFrame->data[0]);
    //UV
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_uv);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, finalAVFrame->linesize[1] / 2,
                 finalAVFrame->height / 2, 0,
                 GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, finalAVFrame->data[1]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawOrder);
    glDisableVertexAttribArray((GLuint) vPositionLocation);

    if (inputTextureCoordinateLocation != -1)
        glDisableVertexAttribArray((GLuint) inputTextureCoordinateLocation);

    glUseProgram(0);
    return 0;
}
