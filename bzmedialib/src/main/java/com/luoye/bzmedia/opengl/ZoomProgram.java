package com.luoye.bzmedia.opengl;

/**
 * Created by bookzhan on 2022-04-04 08:57.
 * description:
 */
public class ZoomProgram {
    public native static long initNative(boolean flipVertical);

    public native static void setCenterPoint(long nativeHandle, float x, float y);

    public native static void setTextureSize(long nativeHandle, int width, int height);

    public native static void setRadius(long nativeHandle, float radius);

    public native static void setZoomRatio(long nativeHandle, float zoomRatio);

    public native static int onDrawFrame(long nativeHandle, int textureId);

    public native static int setMatrix(long nativeHandle, float[] matrix);

    public native static int release(long nativeHandle);
}
