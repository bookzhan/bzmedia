
/**
 * Created by bookzhan on 2017-08-21 19:05.
 * 说明:
 */


#include <GLES/gl.h>
#include "GLUtil.h"
#include "BZLogUtil.h"

void GLUtil::checkGlError(const char *tag) {
    int error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        BZLogUtil::logE("GL ERROR %s glError %d", tag, error);
    }
}