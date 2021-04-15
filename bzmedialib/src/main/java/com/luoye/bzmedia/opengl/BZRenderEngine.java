package com.luoye.bzmedia.opengl;

import com.luoye.bzmedia.bean.TextureHandleInfo;

/**
 * Created by bookzhan on 2019-02-20 09:58.
 * 说明:
 */
public class BZRenderEngine {
    public static native long init();

    public static native void setFlip(long nativeHandle, boolean flipHorizontal, boolean flipVertical);

    public static native void setFinalSize(long nativeHandle, int finalWidth, int finalHeight);

    public static native void setTextureHandleInfo(long nativeHandle, TextureHandleInfo textureHandleInfo);

    public static native void setRotation(long nativeHandle, int rotation);

    public static native int draw(long nativeHandle, int textureId, int textureWidth, int textureHeight);

    public static native int release(long nativeHandle);
}
