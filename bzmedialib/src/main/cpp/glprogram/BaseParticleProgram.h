//
/**
 * Created by zhandalin on 2017-10-12 16:00.
 * 说明:粒子系统的基类
 */
//

#ifndef BZFFMPEG_BASEPARTICLEPROGRAM_H
#define BZFFMPEG_BASEPARTICLEPROGRAM_H


#include "BaseMatrixProgram.h"

class BaseParticleProgram : public BaseMatrixProgram {
public:
    BaseParticleProgram();

    void setAlpha(float alpha);
protected:
    virtual int initProgram(const char *vertexShader, const char *fragmentShader);

    virtual int drawArraysBefore();
private:
    GLint alphaLocation = -1;
    float alpha = 1;
};


#endif //BZFFMPEG_BASEPARTICLEPROGRAM_H
