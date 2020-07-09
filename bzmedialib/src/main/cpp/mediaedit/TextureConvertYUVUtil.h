
/**
 * Created by zhandalin on 2017-08-02 17:09.
 * 说明:输入一个纹理ID,纹理的宽,高,输出YUV420p数据
 * 目前要在OpenGL线程里面调用
 */


#ifndef BZFFMPEG_TEXTURECONVERTYUVUTIL_H
#define BZFFMPEG_TEXTURECONVERTYUVUTIL_H


#include <GLES/gl.h>

extern "C" {
#include <libavutil/frame.h>
};

class TextureConvertYUVUtil {
public:
    //主要是初始化OpenGL相关的东西
    int init(int width, int height);

    AVFrame *textureConvertYUV(GLint textureId);

    //主要是回收OpenGL相关的东西
    int destroyResource();

    void setTextureFlip(bool flipHorizontal, bool flipVertical);

    //有些手机对highp支持的不好,需要区分对待
    static bool useHDShader;
private:
    GLuint program = 0;
    static const char *defaultVertexSource;
    static const char *defaultHighpFragmentSource;
    static const char *defaultMediumpFragmentSource;

    GLuint imageTextureLocation, widthLocation, heightLocation;
    int width, height;
    GLuint positionBuffer = 0, coordinateBuffer = 0;

    const int COORDS_PER_VERTEX = 2;
    const int VERTEX_STRIDE = COORDS_PER_VERTEX * 4; // 4 bytes per vertex
    const float TEXTURE_VERTICES[8] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
    };
    const float CUBE[8] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f};
    const short drawOrder[6] = {0, 1, 2, 1, 2, 3};


    //不采用像素缓存,适用于4.0~4.3的设备
    AVFrame *captureYUVByDirectReadPix();

    GLuint frameBuffer = 0;
    GLuint frameBuffersTextureId = 0;
    GLint vPositionLocation = 0;
    GLint inputTextureCoordinateLocation = 0;

    GLuint *pixBuffers = NULL;

    int64_t totalTime = 0;
    int64_t count = 1;
};


#endif //BZFFMPEG_TEXTURECONVERTYUVUTIL_H
