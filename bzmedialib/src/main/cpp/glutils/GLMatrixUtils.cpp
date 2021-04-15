//
/**
 * Created by bookzhan on 2017-10-12 16:53.
 * 说明:
 */
//

#include <string.h>
#include "GLMatrixUtils.h"

float *GLMatrixUtils::sTemp = new float[32];

void GLMatrixUtils::translateM(float *m, int mOffset, float x, float y, float z) {
    for (int i = 0; i < 4; i++) {
        int mi = mOffset + i;
        m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
    }
}

void
GLMatrixUtils::rotateM(float *m, int mOffset,
                       float a, float x, float y, float z) {
    //TODO 验证正确性
    setRotateM(sTemp, 0, a, x, y, z);
    multiplyMM(sTemp + 16, m + mOffset, sTemp);
    memcpy(m + mOffset, sTemp + 16, 16);
}

void GLMatrixUtils::scaleM(float *m, int mOffset, float x, float y, float z) {
    for (int i = 0; i < 4; i++) {
        int mi = mOffset + i;
        m[mi] *= x;
        m[4 + mi] *= y;
        m[8 + mi] *= z;
    }
}

void GLMatrixUtils::setLookAtM(float *rm, int rmOffset, float eyeX, float eyeY, float eyeZ,
                               float centerX, float centerY, float centerZ, float upX, float upY,
                               float upZ) {
    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;

    // Normalize f
    float rlf = 1.0f / vectorLength(fx, fy, fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    // compute s = f x up (x means "cross product")
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;

    // and normalize s
    float rls = 1.0f / vectorLength(sx, sy, sz);
    sx *= rls;
    sy *= rls;
    sz *= rls;

    // compute u = s x f
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    rm[rmOffset + 0] = sx;
    rm[rmOffset + 1] = ux;
    rm[rmOffset + 2] = -fx;
    rm[rmOffset + 3] = 0.0f;

    rm[rmOffset + 4] = sy;
    rm[rmOffset + 5] = uy;
    rm[rmOffset + 6] = -fy;
    rm[rmOffset + 7] = 0.0f;

    rm[rmOffset + 8] = sz;
    rm[rmOffset + 9] = uz;
    rm[rmOffset + 10] = -fz;
    rm[rmOffset + 11] = 0.0f;

    rm[rmOffset + 12] = 0.0f;
    rm[rmOffset + 13] = 0.0f;
    rm[rmOffset + 14] = 0.0f;
    rm[rmOffset + 15] = 1.0f;

    translateM(rm, rmOffset, -eyeX, -eyeY, -eyeZ);
}

void GLMatrixUtils::frustumM(float *m, int offset, float left, float right, float bottom, float top,
                             float near, float far) {
    if (left == right) {
        return;
    }
    if (top == bottom) {
        return;
    }
    if (near == far) {
        return;
    }
    if (near <= 0.0f) {
        return;
    }
    if (far <= 0.0f) {
        return;
    }
    float r_width = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth = 1.0f / (near - far);
    float x = 2.0f * (near * r_width);
    float y = 2.0f * (near * r_height);
    float A = (right + left) * r_width;
    float B = (top + bottom) * r_height;
    float C = (far + near) * r_depth;
    float D = 2.0f * (far * near * r_depth);
    m[offset + 0] = x;
    m[offset + 5] = y;
    m[offset + 8] = A;
    m[offset + 9] = B;
    m[offset + 10] = C;
    m[offset + 14] = D;
    m[offset + 11] = -1.0f;
    m[offset + 1] = 0.0f;
    m[offset + 2] = 0.0f;
    m[offset + 3] = 0.0f;
    m[offset + 4] = 0.0f;
    m[offset + 6] = 0.0f;
    m[offset + 7] = 0.0f;
    m[offset + 12] = 0.0f;
    m[offset + 13] = 0.0f;
    m[offset + 15] = 0.0f;
}

void GLMatrixUtils::orthoM(float *m, int mOffset, float left, float right, float bottom, float top,
                           float near, float far) {
    if (left == right) {
        return;
    }
    if (bottom == top) {
        return;
    }
    if (near == far) {
        return;
    }

    float r_width = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth = 1.0f / (far - near);
    float x = 2.0f * (r_width);
    float y = 2.0f * (r_height);
    float z = -2.0f * (r_depth);
    float tx = -(right + left) * r_width;
    float ty = -(top + bottom) * r_height;
    float tz = -(far + near) * r_depth;
    m[mOffset + 0] = x;
    m[mOffset + 5] = y;
    m[mOffset + 10] = z;
    m[mOffset + 12] = tx;
    m[mOffset + 13] = ty;
    m[mOffset + 14] = tz;
    m[mOffset + 15] = 1.0f;
    m[mOffset + 1] = 0.0f;
    m[mOffset + 2] = 0.0f;
    m[mOffset + 3] = 0.0f;
    m[mOffset + 4] = 0.0f;
    m[mOffset + 6] = 0.0f;
    m[mOffset + 7] = 0.0f;
    m[mOffset + 8] = 0.0f;
    m[mOffset + 9] = 0.0f;
    m[mOffset + 11] = 0.0f;
}

void GLMatrixUtils::multiplyMM(float *r, const float *lhs, const float *rhs) {
    for (int i = 0; i < 4; i++) {
        register const float rhs_i0 = rhs[I(i, 0)];
        register float ri0 = lhs[I(0, 0)] * rhs_i0;
        register float ri1 = lhs[I(0, 1)] * rhs_i0;
        register float ri2 = lhs[I(0, 2)] * rhs_i0;
        register float ri3 = lhs[I(0, 3)] * rhs_i0;
        for (int j = 1; j < 4; j++) {
            register const float rhs_ij = rhs[I(i, j)];
            ri0 += lhs[I(j, 0)] * rhs_ij;
            ri1 += lhs[I(j, 1)] * rhs_ij;
            ri2 += lhs[I(j, 2)] * rhs_ij;
            ri3 += lhs[I(j, 3)] * rhs_ij;
        }
        r[I(i, 0)] = ri0;
        r[I(i, 1)] = ri1;
        r[I(i, 2)] = ri2;
        r[I(i, 3)] = ri3;
    }
}

void GLMatrixUtils::setRotateM(float *rm, int rmOffset, float a, float x, float y, float z) {
    rm[rmOffset + 3] = 0;
    rm[rmOffset + 7] = 0;
    rm[rmOffset + 11] = 0;
    rm[rmOffset + 12] = 0;
    rm[rmOffset + 13] = 0;
    rm[rmOffset + 14] = 0;
    rm[rmOffset + 15] = 1;
    a *= (float) (M_1_PI / 180.0f);
    float s = (float) sin(a);
    float c = (float) cos(a);
    if (1.0f == x && 0.0f == y && 0.0f == z) {
        rm[rmOffset + 5] = c;
        rm[rmOffset + 10] = c;
        rm[rmOffset + 6] = s;
        rm[rmOffset + 9] = -s;
        rm[rmOffset + 1] = 0;
        rm[rmOffset + 2] = 0;
        rm[rmOffset + 4] = 0;
        rm[rmOffset + 8] = 0;
        rm[rmOffset + 0] = 1;
    } else if (0.0f == x && 1.0f == y && 0.0f == z) {
        rm[rmOffset + 0] = c;
        rm[rmOffset + 10] = c;
        rm[rmOffset + 8] = s;
        rm[rmOffset + 2] = -s;
        rm[rmOffset + 1] = 0;
        rm[rmOffset + 4] = 0;
        rm[rmOffset + 6] = 0;
        rm[rmOffset + 9] = 0;
        rm[rmOffset + 5] = 1;
    } else if (0.0f == x && 0.0f == y && 1.0f == z) {
        rm[rmOffset + 0] = c;
        rm[rmOffset + 5] = c;
        rm[rmOffset + 1] = s;
        rm[rmOffset + 4] = -s;
        rm[rmOffset + 2] = 0;
        rm[rmOffset + 6] = 0;
        rm[rmOffset + 8] = 0;
        rm[rmOffset + 9] = 0;
        rm[rmOffset + 10] = 1;
    } else {
        float len = vectorLength(x, y, z);
        if (1.0f != len) {
            float recipLen = 1.0f / len;
            x *= recipLen;
            y *= recipLen;
            z *= recipLen;
        }
        float nc = 1.0f - c;
        float xy = x * y;
        float yz = y * z;
        float zx = z * x;
        float xs = x * s;
        float ys = y * s;
        float zs = z * s;
        rm[rmOffset + 0] = x * x * nc + c;
        rm[rmOffset + 4] = xy * nc - zs;
        rm[rmOffset + 8] = zx * nc + ys;
        rm[rmOffset + 1] = xy * nc + zs;
        rm[rmOffset + 5] = y * y * nc + c;
        rm[rmOffset + 9] = yz * nc - xs;
        rm[rmOffset + 2] = zx * nc - ys;
        rm[rmOffset + 6] = yz * nc + xs;
        rm[rmOffset + 10] = z * z * nc + c;
    }
}

void GLMatrixUtils::setRotateEulerM(float *rm, int rmOffset, float x, float y, float z) {
    x *= (float) (M_PI / 180.0f);
    y *= (float) (M_PI / 180.0f);
    z *= (float) (M_PI / 180.0f);
    float cx = (float) cos(x);
    float sx = (float) sin(x);
    float cy = (float) cos(y);
    float sy = (float) sin(y);
    float cz = (float) cos(z);
    float sz = (float) sin(z);
    float cxsy = cx * sy;
    float sxsy = sx * sy;

    rm[rmOffset + 0] = cy * cz;
    rm[rmOffset + 1] = -cy * sz;
    rm[rmOffset + 2] = sy;
    rm[rmOffset + 3] = 0.0f;

    rm[rmOffset + 4] = cxsy * cz + cx * sz;
    rm[rmOffset + 5] = -cxsy * sz + cx * cz;
    rm[rmOffset + 6] = -sx * cy;
    rm[rmOffset + 7] = 0.0f;

    rm[rmOffset + 8] = -sxsy * cz + sx * sz;
    rm[rmOffset + 9] = sxsy * sz + sx * cz;
    rm[rmOffset + 10] = cx * cy;
    rm[rmOffset + 11] = 0.0f;

    rm[rmOffset + 12] = 0.0f;
    rm[rmOffset + 13] = 0.0f;
    rm[rmOffset + 14] = 0.0f;
    rm[rmOffset + 15] = 1.0f;
}

void
GLMatrixUtils::multiplyMV(float *resultVec, int resultVecOffset, float *lhsMat, int lhsMatOffset,
                          float *rhsVec, int rhsVecOffset) {
    multiplyMVImplement(resultVec + resultVecOffset, lhsMat + lhsMatOffset, rhsVec + rhsVecOffset);
}

void GLMatrixUtils::setIdentityM(float *sm, int smOffset) {
    for (int i = 0; i < 16; i++) {
        sm[smOffset + i] = 0;
    }
    for (int i = 0; i < 16; i += 5) {
        sm[smOffset + i] = 1.0f;
    }
}
