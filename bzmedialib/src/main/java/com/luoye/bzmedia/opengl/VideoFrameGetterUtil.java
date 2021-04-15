package com.luoye.bzmedia.opengl;

import android.graphics.Bitmap;

/**
 * Created by bookzhan on 2019-02-21 15:51.
 * 说明:
 */
public class VideoFrameGetterUtil {
    public static native long init(String videoPath, boolean userSoftDecode);

    public static native int setStartTime(long nativeHandle, long startTime);

    public static native int setEnableLoop(long nativeHandle, boolean enableLoop);

    public static native int getVideoWidth(long nativeHandle);

    public static native int getVideoHeight(long nativeHandle);

    public static native long getVideoDuration(long nativeHandle);

    public static native int getVideoFrame(long nativeHandle, long currentTime);

    public static native Bitmap getVideoFrame4Bitmap(long nativeHandle, long currentTime);

    public static native int release(long nativeHandle);

}
