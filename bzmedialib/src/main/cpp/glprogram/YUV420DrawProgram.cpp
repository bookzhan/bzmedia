//
/**
 * Created by zhandalin on 2018-11-01 13:54.
 * 说明:
 */
//

#include <common/BZLogUtil.h>
#include <glutils/TextureUtil.h>
#include "YUV420DrawProgram.h"
#include "../common/GLUtil.h"

YUV420DrawProgram::YUV420DrawProgram() {
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
                      "uniform sampler2D tex_u;\n"
                      "uniform sampler2D tex_v;\n"
                      "const mat3 yuv2rgb = mat3(\n"
                      "            1.164,1.164,1.164,\n"
                      "            0.0,-0.392,2.017,\n"
                      "            1.596,-0.813,0.0\n"
                      "            );\n"
                      "void main() {\n"
                      "    vec3 yuv;\n"
                      "    yuv.x = texture2D(tex_y, textureCoordinate).r - (16.0 / 255.0);\n"
                      "    yuv.y = texture2D(tex_u, textureCoordinate).r - 0.5;\n"
                      "    yuv.z = texture2D(tex_v, textureCoordinate).r - 0.5;\n"
                      "    vec3 videoColor = yuv2rgb * yuv;\n"
                      "    gl_FragColor =vec4(videoColor.rgb, 1.0);\n"
                      "}";
}

void YUV420DrawProgram::init() {
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

int YUV420DrawProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    mProgram = TextureUtil::initProgram(vertexShader, fragmentShader);
    if (!mProgram) {
        BZLogUtil::logE("Could not create program.");
        return -1;
    }
    vPositionLocation = (GLuint) glGetAttribLocation(mProgram, "vPosition");
    BZLogUtil::logV("glGetAttribLocation(\"vPosition\") = %d\n",
                    vPositionLocation);

    inputTextureCoordinateLocation = glGetAttribLocation(mProgram, "inputTextureCoordinate");
    BZLogUtil::logV("glGetAttribLocation(\"inputTextureCoordinate\") = %d\n",
                    inputTextureCoordinateLocation);


    m_textureYLocation = glGetUniformLocation(mProgram, "tex_y");
    m_textureULocation = glGetUniformLocation(mProgram, "tex_u");
    m_textureVLocation = glGetUniformLocation(mProgram, "tex_v");

    glGenTextures(1, &textures_y);
    glBindTexture(GL_TEXTURE_2D, textures_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &textures_u);
    glBindTexture(GL_TEXTURE_2D, textures_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &textures_v);
    glBindTexture(GL_TEXTURE_2D, textures_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glUseProgram(mProgram);
    glUniform1i(inputImageTextureLocation, 0);
    glUniform1i(m_textureYLocation,
                0);     //设置纹理，按照前面设置的规则怎样将图像或纹理贴上（参数和选择的活跃纹理单元对应，GL_TEXTURE0）
    glUniform1i(m_textureULocation, 1);
    glUniform1i(m_textureVLocation, 2);
    glUseProgram(0);
    return mProgram;
}

int YUV420DrawProgram::draw(AVFrame *inputAVFrame) {
    if (nullptr == inputAVFrame || nullptr == inputAVFrame->linesize
        || nullptr == inputAVFrame->data[0]
        || nullptr == inputAVFrame->data[1]
        || nullptr == inputAVFrame->data[2]) {
        BZLogUtil::logE("YUV420DrawProgram::draw AVFrame data Error");
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, finalAVFrame->linesize[0], finalAVFrame->height, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, finalAVFrame->data[0]);
    //U
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, finalAVFrame->linesize[1],
                 finalAVFrame->height / 2,
                 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, finalAVFrame->data[1]);
    //V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, finalAVFrame->linesize[2],
                 finalAVFrame->height / 2,
                 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE, finalAVFrame->data[2]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawOrder);
    glDisableVertexAttribArray((GLuint) vPositionLocation);

    glDisableVertexAttribArray((GLuint) inputTextureCoordinateLocation);
    glUseProgram(0);

    return 0;
}

void YUV420DrawProgram::updateCoordinateBuffer() {
    if (coordinateBuffer <= 0)
        glGenBuffers(1, &coordinateBuffer);
    //存放顶点位置数据
    glBindBuffer(GL_ARRAY_BUFFER, coordinateBuffer);
    const float *buffer = TextureUtil::getRotationTexture(rotate, needFlipHorizontal,
                                                          needFlipVertical);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_VERTICES),
                 buffer,
                 GL_STATIC_DRAW);
    free((void *) buffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void YUV420DrawProgram::setRotation(int rotation) {
    this->rotate = rotation;
    updateCoordinateBuffer();
}

void YUV420DrawProgram::setFlip(bool flipHorizontal, bool flipVertical) {
    this->needFlipHorizontal = flipHorizontal;
    this->needFlipVertical = flipVertical;
    updateCoordinateBuffer();
}

int YUV420DrawProgram::releaseResource() {
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
    if (textures_u > 0 && glIsTexture(textures_u)) {
        glDeleteTextures(1, &textures_u);
        textures_u = 0;
    }
    if (textures_v > 0 && glIsTexture(textures_v)) {
        glDeleteTextures(1, &textures_v);
        textures_v = 0;
    }
    GLUtil::checkGlError("AVFrameProgram::releaseResource end");
    BZLogUtil::logD("AVFrameProgram releaseResource finish");
    return 0;
}
