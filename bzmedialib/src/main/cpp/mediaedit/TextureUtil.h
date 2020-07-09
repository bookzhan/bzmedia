
/**
 * Created by zhandalin on 2017-06-05 15:07.
 * 说明:
 */


#ifndef BZFFMPEG_TEXTUREROTATIONUTIL_H
#define BZFFMPEG_TEXTUREROTATIONUTIL_H


#include <GLES/gl.h>
#include "../common/bzgl.h"
#include <malloc.h>

class TextureUtil {
public:
    static const float CUBE[8];

    static const float *getRotationTexture(int rotation, bool flipHorizontal,
                                           bool flipVertical);

    static const GLuint initProgram(const char *pVertexSource, const char *pFragmentSource);

    static GLuint loadShader(GLenum shaderType, const char *pSource);

    static const float TEXTURE_NO_ROTATION[8];
    static const float TEXTURE_ROTATED_90[8];
    static const float TEXTURE_ROTATED_180[8];
    static const float TEXTURE_ROTATED_270[8];
private:
    static float flip(float i) {
        if (i == 0.0f) {
            return 1.0f;
        }
        return 0.0f;
    }
};


#endif //BZFFMPEG_TEXTUREROTATIONUTIL_H
