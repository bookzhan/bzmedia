//
/**
 * Created by zhandalin on 2018-11-08 19:03.
 * 说明:
 */
//

#ifndef BZMEDIA_IMAGEEXTERNALPROGRAM_H
#define BZMEDIA_IMAGEEXTERNALPROGRAM_H

#include <GLES/gl.h>
#include <GLES/glext.h>
#include "BaseProgram.h"

class ImageExternalProgram : public BaseProgram {
public:
    ImageExternalProgram(bool needFadeShow = false);

    int initImageExternal();

    virtual int draw();

    int drawArraysBefore();

    int releaseResource();

private:
    int initProgram(const char *vertexShader, const char *fragmentShader);

    GLuint baseTextureId = 0;
    bool needFadeShow = false;
    bool lastEnableBlendState = false;
    float alpha = 0;
    GLint alphaLocation = 0;
    int64_t firstDrawTime = 0;
};


#endif //BZMEDIA_IMAGEEXTERNALPROGRAM_H
