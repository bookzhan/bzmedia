package com.luoye.bzmedia.opengl;

/**
 * Created by bookzhan on 2021-04-01 14:00.
 * description:
 */
public class CropTextureUtil {
    public static native long initCropTexture();

    public static native int cropTexture(long handle, int srcTexture, int srcWidth, int srcHeight,
                                         int startX, int startY, int targetWidth, int targetHeight);

    public static native int cropTextureRelease(long handle);

}
