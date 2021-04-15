//
/**
 * Created by bookzhan on 2022-03-06 09:14.
 *description:
 */
//

#ifndef BZMEDIA_MOSAICPROGRAM_H
#define BZMEDIA_MOSAICPROGRAM_H

#include "BaseProgram.h"

class MosaicProgram : public BaseProgram {
public:
    MosaicProgram();

    void setCenterPoint(float x, float y);

    void setRadius(float radius);

    void setMosaicType(int mosaicType);

    void setRectangleSize(int width, int height);

    void setTextureSize(int width, int height);

    void setMosaicSize(float width, float height);

protected:
    int initProgram(const char *vertexShader, const char *fragmentShader);

    int drawArraysBefore();

    GLint textureSizeLoc = 0;
    GLint mosaicSizeLoc = 0;
    GLint circleCenterLoc = 0;
    GLint radiusLoc = 0;
    GLint mosaicTypeLoc = 0;
    GLint rectangleWidthLoc = 0;
    GLint rectangleHeightLoc = 0;
    int textureWidth = 480;
    int textureHeight = 480;
    float mosaicWidth = 32;
    float mosaicHeight = 32;
    float circleCenterX = 0;
    float circleCenterY = 0;
    float radius = 100;
    int mosaicType = 0;
    int rectangleWidth = 1;
    int rectangleHeight = 1;

};


#endif //BZMEDIA_MOSAICPROGRAM_H
