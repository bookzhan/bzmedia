//
/**
 * Created by zhandalin on 2017-10-12 14:37.
 * 说明:
 */
//

#ifndef BZFFMPEG_BASEMATRIXPROGRAM_H
#define BZFFMPEG_BASEMATRIXPROGRAM_H


#include "BaseProgram.h"

class BaseMatrixProgram : public BaseProgram {
public:
    BaseMatrixProgram();

    void setMatrix(float *matrix);

protected:
    virtual int initProgram(const char *vertexShader, const char *fragmentShader);

    virtual int drawArraysBefore();

private:
    GLint vMatrixLocation = -1;
    float *matrix = nullptr;

};


#endif //BZFFMPEG_BASEMATRIXPROGRAM_H
