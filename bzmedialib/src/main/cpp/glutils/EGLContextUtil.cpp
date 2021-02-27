//
// Created by luoye on 2017/6/1.
//


#include <common/BZLogUtil.h>
#include "EGLContextUtil.h"

int EGLContextUtil::initEGLContext(int width, int height) {
    return initEGLContext(width, height, EGL_NO_CONTEXT);
}

int EGLContextUtil::releaseEGLContext() {
    if (eglCtx != EGL_NO_CONTEXT)
        eglDestroyContext(eglDisp, eglCtx);
    if (eglSurface != EGL_NO_SURFACE)
        eglDestroySurface(eglDisp, eglSurface);

    if (eglDisp != EGL_NO_DISPLAY)
        eglMakeCurrent(eglDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (eglDisp != EGL_NO_DISPLAY)
        eglTerminate(eglDisp);

    eglDisp = EGL_NO_DISPLAY;
    eglSurface = EGL_NO_SURFACE;
    eglCtx = EGL_NO_CONTEXT;
    BZLogUtil::logD("releaseEGLContext");
    return 0;
}

void EGLContextUtil::swapBuffers() {
//    if (eglDisp != EGL_NO_DISPLAY && eglSurface != EGL_NO_SURFACE)
//        eglSwapBuffers(eglDisp, eglSurface);
}

int EGLContextUtil::initEGLContext(int width, int height, EGLContext share_context) {
    //EGL config attributes
    const EGLint confAttr[] =
            {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,// very important!
                    EGL_SURFACE_TYPE,
                    EGL_PBUFFER_BIT,//EGL_WINDOW_BIT EGL_PBUFFER_BIT we will create a pixelbuffer surface
                    EGL_RED_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_BLUE_SIZE, 8,
                    EGL_ALPHA_SIZE, 8,// if you need the alpha channel
                    EGL_DEPTH_SIZE, 0,// if you need the depth buffer
                    EGL_STENCIL_SIZE, 0,
                    EGL_NONE
            };
    // EGL context attributes
    EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION,
            2,// very important!
            EGL_NONE
    };
#ifdef USEOPENGL3
    ctxAttr[1] = 3;
#endif
    // surface attributes
    // the surface size is set to the input frame size
    const EGLint surfaceAttr[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    EGLint eglMajVers, eglMinVers;
    EGLint numConfigs;

    eglDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisp == EGL_NO_DISPLAY) {
        //Unable to open connection to local windowing system
        BZLogUtil::logD("Unable to open connection to local windowing system");
    }
    if (!eglInitialize(eglDisp, &eglMajVers, &eglMinVers)) {
        // Unable to initialize EGL. Handle and recover
        BZLogUtil::logD("Unable to initialize EGL");
    }
    BZLogUtil::logD("EGL init with version %d.%d", eglMajVers, eglMinVers);
    // choose the first config, i.e. best config
    if (!eglChooseConfig(eglDisp, confAttr, &eglConf, 1, &numConfigs)) {
        BZLogUtil::logD("some config is wrong");
    } else {
        BZLogUtil::logD("all configs is OK");
    }
    // create a pixelbuffer surface
    eglSurface = eglCreatePbufferSurface(eglDisp, eglConf, surfaceAttr);
    if (eglSurface == EGL_NO_SURFACE) {
        switch (eglGetError()) {
            case EGL_BAD_ALLOC:
                // Not enough resources available. Handle and recover
                BZLogUtil::logD("Not enough resources available");
                break;
            case EGL_BAD_CONFIG:
                // Verify that provided EGLConfig is valid
                BZLogUtil::logD("provided EGLConfig is invalid");
                break;
            case EGL_BAD_PARAMETER:
                // Verify that the EGL_WIDTH and EGL_HEIGHT are
                // non-negative values
                BZLogUtil::logD("provided EGL_WIDTH and EGL_HEIGHT is invalid");
                break;
            case EGL_BAD_MATCH:
                // Check window and EGLConfig attributes to determine
                // compatibility and pbuffer-texture parameters
                BZLogUtil::logD("Check window and EGLConfig attributes");
                break;
            default:
                break;
        }
    }
    eglCtx = eglCreateContext(eglDisp, eglConf, share_context, ctxAttr);
    if (eglCtx == EGL_NO_CONTEXT) {
        EGLint error = eglGetError();
        if (error == EGL_BAD_CONFIG) {
            // Handle error and recover
            BZLogUtil::logD("EGL_BAD_CONFIG");
        }
    }
    if (!eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx)) {
        BZLogUtil::logD("MakeCurrent failed");
    }
    BZLogUtil::logD("initialize initEGLContext success!");
    return 0;
}

void EGLContextUtil::makeCurrent() {
    if (!eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx)) {
        BZLogUtil::logD("MakeCurrent failed");
    }
}

void EGLContextUtil::detachCurrent() {
    if (eglDisp != EGL_NO_DISPLAY)
        eglMakeCurrent(eglDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}
