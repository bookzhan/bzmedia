//
/**
 * Created by bookzhan on 2017-10-12 16:53.
 * 说明:
 */
//

#ifndef BZFFMPEG_GLMATRIXUTILS_H
#define BZFFMPEG_GLMATRIXUTILS_H

#include <math.h>

#define I(_i, _j) ((_j)+ 4*(_i))

class GLMatrixUtils {
public:
    static void translateM(
            float *m, int mOffset,
            float x, float y, float z);

    static void setIdentityM(float* sm, int smOffset);

    static void rotateM(float *m, int mOffset,
                        float a, float x, float y, float z);

    static void setRotateEulerM(float *rm, int rmOffset,
                                float x, float y, float z);

    static void multiplyMV(float *resultVec,
                           int resultVecOffset, float *lhsMat, int lhsMatOffset,
                           float *rhsVec, int rhsVecOffset);

    static void scaleM(float *m, int mOffset,
                       float x, float y, float z);

    static void setLookAtM(float *rm, int rmOffset,
                           float eyeX, float eyeY, float eyeZ,
                           float centerX, float centerY, float centerZ, float upX, float upY,
                           float upZ);

    static void frustumM(float *m, int offset,
                         float left, float right, float bottom, float top,
                         float near, float far);

    static void orthoM(float *m, int mOffset,
                       float left, float right, float bottom, float top,
                       float near, float far);

    /**
     * Computes the length of a vector.
     *
     * @param x x coordinate of a vector
     * @param y y coordinate of a vector
     * @param z z coordinate of a vector
     * @return the length of a vector
     */
    static float vectorLength(float x, float y, float z) {
        return (float) sqrt(x * x + y * y + z * z);
    }

    static void multiplyMM(float *r, const float *lhs, const float *rhs);

    static float *sTemp;
private:
    static void setRotateM(float *rm, int rmOffset,
                           float a, float x, float y, float z);

    static void mx4transform(float x, float y, float z, float w, const float *pM, float *pDest) {
        pDest[0] = pM[0 + 4 * 0] * x + pM[0 + 4 * 1] * y + pM[0 + 4 * 2] * z + pM[0 + 4 * 3] * w;
        pDest[1] = pM[1 + 4 * 0] * x + pM[1 + 4 * 1] * y + pM[1 + 4 * 2] * z + pM[1 + 4 * 3] * w;
        pDest[2] = pM[2 + 4 * 0] * x + pM[2 + 4 * 1] * y + pM[2 + 4 * 2] * z + pM[2 + 4 * 3] * w;
        pDest[3] = pM[3 + 4 * 0] * x + pM[3 + 4 * 1] * y + pM[3 + 4 * 2] * z + pM[3 + 4 * 3] * w;
    }

    static void multiplyMVImplement(float *r, const float *lhs, const float *rhs) {
        mx4transform(rhs[0], rhs[1], rhs[2], rhs[3], lhs, r);
    }
};


#endif //BZFFMPEG_GLMATRIXUTILS_H
