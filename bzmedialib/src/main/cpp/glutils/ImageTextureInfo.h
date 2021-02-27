//
/**
 * Created by zhandalin on 2018-04-12 15:03.
 * 说明:
 */
//

#ifndef BZFFMPEG_IMAGETEXTUREINFO_H
#define BZFFMPEG_IMAGETEXTUREINFO_H

#include <clocale>

typedef struct ImageTextureInfo {
    int textureID = 0;
    int textureWidth = 0;
    int textureHeight = 0;
    int stride = 0;
    char *data = NULL;
} ImageTextureInfo;
#endif //BZFFMPEG_IMAGETEXTUREINFO_H
