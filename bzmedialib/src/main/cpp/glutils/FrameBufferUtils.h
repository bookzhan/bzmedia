//
/**
 * Created by bookzhan on 2017-10-11 18:10.
 * 说明:
 */
//

#ifndef BZFFMPEG_FRAMEBUFFERUTILS_H
#define BZFFMPEG_FRAMEBUFFERUTILS_H

#include <GLES/gl.h>

class FrameBufferUtils {
public:
    int initFrameBuffer(int width, int height);

    int releaseFrameBuffer();

    int bindFrameBuffer();

    int unbindFrameBuffer();

    int getBuffersTextureId();

    int getWidth();

    int getHeight();

private:
    GLuint frameBuffer = 0;
    GLuint frameBuffersTextureId = 0;
    GLint lastBindFrameBuffer = 0;
    int width = 0, height = 0;
};


#endif //BZFFMPEG_FRAMEBUFFERUTILS_H
