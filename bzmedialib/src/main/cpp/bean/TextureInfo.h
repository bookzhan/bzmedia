//
/**
 * Created by bookzhan on 2018-04-12 15:03.
 * 说明:
 */
//

#ifndef BZFFMPEG_IMAGETEXTUREINFO_H
#define BZFFMPEG_IMAGETEXTUREINFO_H

#include <stdint.h>

class TextureInfo {
public:
    int textureID = 0;
    int textureWidth = 0;
    int textureHeight = 0;
    int stride = 0;
    char *data = NULL;
    //单位毫秒,纹理的呈现时间
    int64_t texturePts = 0;
};

#endif //BZFFMPEG_IMAGETEXTUREINFO_H
