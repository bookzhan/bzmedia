package com.luoye.bzmedia.opengl;

/**
 * Created by bookzhan on 2022-02-08 23:10.
 * description:
 */
public class YUV420Program {
    public static native long init();

    public static native void setRotation(long nativeHandle, int rotation);

    public static native void setFlip(long nativeHandle, boolean flipHorizontal, boolean flipVertical);

    public static native int draw(long nativeHandle, byte[] pixels, int width, int height);

    public static native void release(long nativeHandle);
}
