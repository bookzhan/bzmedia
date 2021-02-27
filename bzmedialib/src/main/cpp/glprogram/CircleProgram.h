//
/**
 * Created by zhandalin on 2018-05-10 15:56.
 * 说明:
 */
//

#ifndef BZMEDIA_CIRCLEPROGRAM_H
#define BZMEDIA_CIRCLEPROGRAM_H


#include "BaseProgram.h"

class CircleProgram : public BaseProgram {
public:
    CircleProgram();

    void setTextureSize(float width, float height);

protected:
    int initProgram(const char *vertexShader, const char *fragmentShader);

    int drawArraysBefore();

    GLint widthLoc;
    GLint heightLoc;
    float width, height;
};


#endif //BZMEDIA_CIRCLEPROGRAM_H
