//
/**
 * Created by zhandalin on 2017-10-12 17:24.
 * 说明:
 */
//

#ifndef BZFFMPEG_MATRIXVARYTOOLS_H
#define BZFFMPEG_MATRIXVARYTOOLS_H

#include <stack>

using namespace std;

class MatrixVaryTools {
public:

    //保护现场
    void pushMatrix();

    //恢复现场
    void popMatrix();

    void clearStack();

    //平移变换
    void translate(float x, float y, float z);

    //旋转变换
    void rotate(float angle, float x, float y, float z);

    //缩放变换
    void scale(float x, float y, float z);

    //设置相机
    void setCamera(float ex, float ey, float ez, float cx, float cy, float cz, float ux, float uy,
                   float uz);

    void frustum(float left, float right, float bottom, float top, float near, float far);

    void ortho(float left, float right, float bottom, float top, float near, float far);

    float *getFinalMatrix();

    ~MatrixVaryTools();

private:
    float *mMatrixCamera = new float[16];    //相机矩阵
    float *mMatrixProjection = new float[16];    //投影矩阵
    float *mMatrixCurrent = new float[16]{1, 0, 0, 0,
                                          0, 1, 0, 0,
                                          0, 0, 1, 0,
                                          0, 0, 0, 1};  //原始矩阵

    stack<float *> *mStack = new stack<float *>();      //变换矩阵堆栈
    float *ans = new float[16];
};


#endif //BZFFMPEG_MATRIXVARYTOOLS_H
