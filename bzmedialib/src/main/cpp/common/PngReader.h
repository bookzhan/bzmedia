//
/**
 * Created by bookzhan on 2018-04-12 15:02.
 * 说明:
 */
//

#ifndef BZFFMPEG_PNGREADER_H
#define BZFFMPEG_PNGREADER_H

#include "../glutils/ImageTextureInfo.h"

class PngReader {
public:
    ImageTextureInfo *getPngInfo(const char *filePath);
};


#endif //BZFFMPEG_PNGREADER_H
