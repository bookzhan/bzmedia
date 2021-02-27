//
// Created by luoye on 2017/6/1.
//

#ifndef BZFFMPEG_EGLCONTEXTUTIL_H
#define BZFFMPEG_EGLCONTEXTUTIL_H

#include <EGL/eglplatform.h>
#include <EGL/egl.h>


class EGLContextUtil {
public:
    int initEGLContext(int width, int height);

    int initEGLContext(int width, int height, EGLContext share_context);

    void swapBuffers();

    int releaseEGLContext();

    void makeCurrent();

    void detachCurrent();

private:
    EGLDisplay eglDisp = EGL_NO_DISPLAY;
    EGLConfig eglConf;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    EGLContext eglCtx = EGL_NO_CONTEXT;
};


#endif //BZFFMPEG_EGLCONTEXTUTIL_H
