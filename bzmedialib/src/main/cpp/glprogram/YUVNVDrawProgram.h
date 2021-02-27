//
/**
 * Created by zhandalin on 2018-11-01 14:26.
 * 说明:
 */
//

#ifndef SLIDESHOWSDK_YUVNVDRAWPROGRAM_H
#define SLIDESHOWSDK_YUVNVDRAWPROGRAM_H

#include <GLES2/gl2.h>
#include "BaseYUVDrawProgram.h"

//支持NV21,12
class YUVNVDrawProgram : public BaseYUVDrawProgram {
public:
    YUVNVDrawProgram();

    void init();

    void setRotation(int rotation);

    void setFlip(bool flipHorizontal, bool flipVertical);

    int draw(AVFrame *avFrame);

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

    GLuint textures_y = 0, textures_uv = 0;
    GLuint vPositionLocation = 0;
    GLint inputTextureCoordinateLocation = 0;
    GLint m_textureYLocation = 0;
    GLuint positionBuffer = 0;
    GLuint coordinateBuffer = 0;
    GLint m_textureUVLocation;
};


#endif //SLIDESHOWSDK_YUVNVDRAWPROGRAM_H
