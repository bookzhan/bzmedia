//
/**
*Created by bookzhan on {YEAR}-YEAR−{MONTH}-04 {HOUR}:HOUR:{MINUTE}.
*description:
*/
//

#ifndef BZMEDIA_ZOOMPROGRAM_H
#define BZMEDIA_ZOOMPROGRAM_H


#include "BaseMatrixProgram.h"

class ZoomProgram : public BaseProgram {
public:
    ZoomProgram();

    void setCenterPoint(float x, float y);

    void setTextureSize(int width, int height);

    void setRadius(float radius);

    void setZoomRatio(float zoomRatio);
protected:
    int initProgram(const char *vertexShader, const char *fragmentShader);

    int drawArraysBefore();
    GLint textureSizeLoc = 0;
    GLint circleCenterLoc = 0;
    GLint radiusLoc = 0;
    GLint zoomRatioLoc = 0;
    float centerPointX = 0;
    float centerPointY = 0;
    float radius = 100;
    int textureWidth = 480;
    int textureHeight = 480;
    float zoomRatio = 2;
};


#endif //BZMEDIA_ZOOMPROGRAM_H
