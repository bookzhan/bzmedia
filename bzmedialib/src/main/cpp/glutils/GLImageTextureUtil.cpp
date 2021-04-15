//
/**
 * Created by bookzhan on 2017-10-12 14:48.
 * 说明:
 */
//

#include <GLES2/gl2.h>
#include <malloc.h>
#include <cstdio>
#include <common/BZLogUtil.h>
#include "GLImageTextureUtil.h"
#include "../common/PngReader.h"

char *
(*GLImageTextureUtil::imageTextureCallBack)(const char *, int, int, int) = NULL;

int
(*GLImageTextureUtil::circleTextureCallBack)(int width, int height) = NULL;

int
(*GLImageTextureUtil::getRhombusTextureCallBack)(int width, int height) = NULL;

ImageTextureInfo *
GLImageTextureUtil::getTextureIdFromAssets(const char *fileName, int rotate, int flipHorizontal,
                                           int flipVertical) {
    if (nullptr == fileName) {
        BZLogUtil::logE("getTextureIdFromAssets nullptr == fileName");
        return new ImageTextureInfo();
    }

    char *finalPath = imageTextureCallBack(fileName, rotate, flipHorizontal, flipVertical);
    PngReader pngReader;
    ImageTextureInfo *imageTextureInfo = pngReader.getPngInfo(finalPath);
    imageTextureInfo->textureID = genTextureFromImageData(imageTextureInfo->data,
                                                          imageTextureInfo->textureWidth,
                                                          imageTextureInfo->textureHeight);

    free(imageTextureInfo->data);
    imageTextureInfo->data = nullptr;

    remove(finalPath);
    return imageTextureInfo;
}

ImageTextureInfo *
GLImageTextureUtil::getTextureIdFromSDCard(const char *fileName, int rotate, int flipHorizontal,
                                           int flipVertical) {
    if (nullptr == fileName) {
        BZLogUtil::logE("getTextureIdFromSDCard nullptr == fileName");
        return new ImageTextureInfo();
    }

    char *finalPath = imageTextureCallBack(fileName, rotate, flipHorizontal, flipVertical);
    PngReader pngReader;
    ImageTextureInfo *imageTextureInfo = pngReader.getPngInfo(finalPath);
    imageTextureInfo->textureID = genTextureFromImageData(imageTextureInfo->data,
                                                          imageTextureInfo->textureWidth,
                                                          imageTextureInfo->textureHeight);

    free(imageTextureInfo->data);
    imageTextureInfo->data = nullptr;

    remove(finalPath);
    return imageTextureInfo;
}

ImageTextureInfo *GLImageTextureUtil::getBitmap(const char *fileName) {
    PngReader pngReader;
    return pngReader.getPngInfo(imageTextureCallBack(fileName, 0, 0, 0));
}

int GLImageTextureUtil::genTextureFromImageData(void *imageData, int width, int height) {
    if (nullptr == imageData || width <= 0 || height <= 0)
        return 0;
    int imageTextureId = 0;
    glGenTextures(1, (GLuint *) &imageTextureId);
    glBindTexture(GL_TEXTURE_2D, (GLuint) imageTextureId);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    return imageTextureId;
}

int GLImageTextureUtil::getCircleTexture(int width, int height) {
    if (nullptr != circleTextureCallBack) {
        return circleTextureCallBack(width, height);
    }
    return 0;
}
