//
/**
 * Created by zhandalin on 2019-01-21 17:43.
 * 说明:
 */
//

#ifndef BZMEDIA_TEXTUREYUVPROGRAM_H
#define BZMEDIA_TEXTUREYUVPROGRAM_H


#include "BaseProgram.h"

class TextureYUVProgram : public BaseProgram {
public:
    TextureYUVProgram();

    void setTextureId(int textureIdY, int textureIdUV);

protected:
    int initProgram(const char *vertexShader, const char *fragmentShader);

    int drawArraysBefore();

private:
    int textureIdUV = 0;
};


#endif //BZMEDIA_TEXTUREYUVPROGRAM_H
