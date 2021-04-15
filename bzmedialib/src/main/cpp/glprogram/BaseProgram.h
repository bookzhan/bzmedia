//
/**
 * Created by bookzhan on 2017-10-11 18:57.
 * 说明:
 */
//

#ifndef BZFFMPEG_BASEPROGRAM_H
#define BZFFMPEG_BASEPROGRAM_H


#include <GLES/gl.h>

class BaseProgram {
public:
    BaseProgram();

    //对象初始化后,请调用该方法
    void init();

    void setRotation(int rotation);

    void setFlip(bool flipHorizontal, bool flipVertical);

    virtual int setTextureId(int textureId);

    virtual int draw();

    virtual int releaseResource();

protected:
    virtual int initProgram(const char *vertexShader, const char *fragmentShader);

    const float VERTEXT_COOD[8] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
    };

    const float TEXTURE_COOD[8] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
    };
    const int COORDS_PER_VERTEX = 2;
    const int VERTEX_STRIDE = COORDS_PER_VERTEX * 4; // 4 bytes per vertex

    virtual int drawArraysBefore();

    virtual int drawArraysAfter();

    const char *VERTEX_SHADER = nullptr;

    const char *FRAGMENT_SHADER = nullptr;

    void updateCoordinateBuffer();

    GLuint textureId = 0;
protected:
    GLuint mProgram = 0;
private:
    GLuint positionBuffer = 0;
    GLuint coordinateBuffer = 0;

    GLint vPositionLocation = 0, inputTextureCoordinateLocation = 0;

    int rotation = 0;
    bool flipHorizontal = false;
    bool flipVertical = false;
};


#endif //BZFFMPEG_BASEPROGRAM_H
