//
/**
 * Created by zhandalin on 2018-06-12 11:31.
 * 说明:
 */
//

#ifndef BZMEDIA_CROPTEXTUREUTIL_H
#define BZMEDIA_CROPTEXTUREUTIL_H


#include "FrameBufferUtils.h"
#include "../glprogram/BaseProgram.h"

class CropTextureUtil {
public:
    int cropTexture(int srcTexture, int srcWidth, int srcHeight,
                    int startX, int startY, int targetWidth, int targetHeight);

    void cropTextureOnPause();

    int releaseCropTexture();

private:
    FrameBufferUtils *frameBufferUtils= nullptr;
    BaseProgram *baseProgram= nullptr;
};


#endif //BZMEDIA_CROPTEXTUREUTIL_H
