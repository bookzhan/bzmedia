//
/**
 * Created by zhandalin on 2017-10-12 17:24.
 * 说明:
 */
//

#include <cstring>
#include <common/BZLogUtil.h>
#include "MatrixVaryTools.h"
#include "GLMatrixUtils.h"

void MatrixVaryTools::pushMatrix() {
    float *temp = new float[16];
    memcpy(temp, mMatrixCurrent, 16 * sizeof(float));
    mStack->push(temp);
}

void MatrixVaryTools::popMatrix() {
    if (!mStack->empty()) {
        if (nullptr != mMatrixCurrent)
            delete[](mMatrixCurrent);
        mMatrixCurrent = mStack->top();
        mStack->pop();
    }
}

void MatrixVaryTools::clearStack() {
    while (!mStack->empty()) {
        delete[](mStack->top());
        mStack->pop();
    }
}

void MatrixVaryTools::translate(float x, float y, float z) {
    GLMatrixUtils::translateM(mMatrixCurrent, 0, x, y, z);
}

void MatrixVaryTools::rotate(float angle, float x, float y, float z) {
    GLMatrixUtils::rotateM(mMatrixCurrent, 0, angle, x, y, z);
}

void MatrixVaryTools::scale(float x, float y, float z) {
    GLMatrixUtils::scaleM(mMatrixCurrent, 0, x, y, z);
}

void
MatrixVaryTools::setCamera(float ex, float ey, float ez, float cx, float cy, float cz, float ux,
                           float uy, float uz) {
    GLMatrixUtils::setLookAtM(mMatrixCamera, 0, ex, ey, ez, cx, cy, cz, ux, uy, uz);
}

void
MatrixVaryTools::frustum(float left, float right, float bottom, float top, float near, float far) {
    GLMatrixUtils::frustumM(mMatrixProjection, 0, left, right, bottom, top, near, far);
}

void
MatrixVaryTools::ortho(float left, float right, float bottom, float top, float near, float far) {
    GLMatrixUtils::orthoM(mMatrixProjection, 0, left, right, bottom, top, near, far);
}

float *MatrixVaryTools::getFinalMatrix() {
    GLMatrixUtils::multiplyMM(ans, mMatrixCamera, mMatrixCurrent);
    GLMatrixUtils::multiplyMM(ans, mMatrixProjection, ans);
    return ans;
}

MatrixVaryTools::~MatrixVaryTools() {
    BZLogUtil::logD("~MatrixVaryTools()");
    delete (mStack);
    delete[](ans);

    delete[](mMatrixCamera);
    delete[](mMatrixProjection);
    delete[](mMatrixCurrent);
}
