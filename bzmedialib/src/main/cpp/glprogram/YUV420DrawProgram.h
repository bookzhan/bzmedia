//
/**
 * Created by bookzhan on 2018-11-01 13:54.
 * 说明:
 */
//

#ifndef SLIDESHOWSDK_AVFRAMEPROGRAM_H
#define SLIDESHOWSDK_AVFRAMEPROGRAM_H

#include "BaseYUVDrawProgram.h"
#include <GLES2/gl2.h>

extern "C" {
#include <libavutil/frame.h>
};

//支持yuv420p与YUV420package数据
class YUV420DrawProgram : public BaseYUVDrawProgram {
public:
    YUV420DrawProgram();

    void init();

    void setRotation(int rotation);

    void setFlip(bool flipHorizontal, bool flipVertical);

    int draw(AVFrame *avFrame);
    
    int draw(const void *pixels,int width,int height);

    int releaseResource();

private:
    GLuint mProgram = 0;

    int initProgram(const char *vertexShader, const char *fragmentShader);

    void updateCoordinateBuffer();

    const int COORDS_PER_VERTEX = 2;
    const int VERTEX_STRIDE = COORDS_PER_VERTEX * 4; // 4 bytes per vertex
    const float TEXTURE_VERTICES[8] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
    };
    const short drawOrder[6] = {0, 1, 2, 1, 2, 3}; // order to draw vertices

    const char *VERTEX_SHADER = nullptr;

    const char *FRAGMENT_SHADER = nullptr;

    int rotate = 0;
    bool needFlipHorizontal = false;
    bool needFlipVertical = false;

    GLuint textures_y = 0, textures_u = 0, textures_v = 0;
    GLuint vPositionLocation = 0;
    GLint inputTextureCoordinateLocation = 0;
    GLint inputImageTextureLocation = 0;
    GLint m_textureYLocation = 0;
    GLint m_textureULocation = 0;
    GLint m_textureVLocation = 0;
    GLuint positionBuffer = 0;
    GLuint coordinateBuffer = 0;
};


#endif //SLIDESHOWSDK_AVFRAMEPROGRAM_H
