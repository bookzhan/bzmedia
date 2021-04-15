package com.luoye.bzmedia.opengl;

/**
 * Created by bookzhan on 2022-03-06 09:34.
 * description:
 */
public class MosaicProgram {
    public native static long initNative(boolean flipVertical);

    public native static int setTextureSize(long nativeHandle, int width, int height);

    public native static int setMosaicSize(long nativeHandle, float width, float height);

    public native static int setCenterPoint(long nativeHandle, float x, float y);

    public native static int setRadius(long nativeHandle, float radius);

    public static int setMosaicType(long nativeHandle, MosaicType mosaicType) {
        return setMosaicTypeImp(nativeHandle, mosaicType.ordinal());
    }

    public native static int setRectangleSize(long nativeHandle, int width, int height);

    private native static int setMosaicTypeImp(long nativeHandle, int mosaicType);

    public native static int onDrawFrame(long nativeHandle, int textureId);

    public native static int release(long nativeHandle);

    public enum MosaicType {
        CIRCLE, ELLIPSE_HORIZON, ELLIPSE_VERTICAL, RECTANGLE
    }
}
