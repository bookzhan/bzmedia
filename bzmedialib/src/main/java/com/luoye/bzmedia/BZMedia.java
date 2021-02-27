package com.luoye.bzmedia;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Build;

import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZSpUtils;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.utils.FFmpegCMDUtil;


/**
 * Created by zhandalin on 2020-05-25 16:27.
 * description:
 */
public class BZMedia {
    static {
        System.loadLibrary("bzffmpeg");
        System.loadLibrary("bzffmpegcmd");
        System.loadLibrary("bzmedia");
    }

    public static int init(Context context, boolean isDebug) {
        BZSpUtils.init(context);
        BZLogUtil.setShowLog(isDebug);
        FFmpegCMDUtil.showLog(isDebug);
        return initNative(context, isDebug, Build.VERSION.SDK_INT);
    }

    private native static int initNative(Context context, boolean isDebug, int sdkInt);

    /**
     * @param videoRecordParams 录制参数
     * @return 录制的时间, 单位毫秒, 小于0启动录制失败
     */
    public native static long startRecord(VideoRecordParams videoRecordParams);

    public native static long addAudioData(long nativeHandle, byte[] audioData, int length);

    public native static int setStopRecordFlag(long nativeHandle);

    public static native int stopRecord(long nativeHandle);

    public static native long updateVideoRecorderTexture(long nativeHandle, int textureId);

    public static native long addYUV420Data(long nativeHandle, byte[] data, long pts);

    public static native long addVideoPacketData(long nativeHandle, byte[] videoPacket, long size, long pts);

    public static native int adjustVideoSpeed(String srcVideoPath, String outputPath, float speed);


    /**
     * @param srcMusicVolume 0~1
     * @param bgMusicVolume  0~1
     * @return >=0 successful, <0 fail
     */
    public static native int addBackgroundMusic(String inputPath, String outputPath,
                                                String musicPath, float srcMusicVolume, float bgMusicVolume, OnActionListener onActionListener);

    /**
     * If only replacing background music, this function is more efficient than addBackgroundMusic
     *
     * @param musicPath The format must be m4a, and the encoding method must be aac
     * @return >=0 successful, <0 fail
     */
    public static native int replaceBackgroundMusic(String videoPath, String musicPath, String outputPath, OnActionListener onActionListener);

    /**
     * @param outImageParentPath 存放图片的父目录
     * @param imageCount         需要截图的数量,会自动平均截取的
     * @param scale2Width        把宽缩放到对应的值
     * @return >=0 success
     */
    public static native int getImageFromVideo(String videoPath, String outImageParentPath, int imageCount, int scale2Width, OnGetImageFromVideoListener onGetImageFromVideoListener);

    public static native int getBitmapFromVideo(String videoPath, int imageCount, int scale2Width, OnGetBitmapFromVideoListener onGetBitmapFromVideoListener);


    /**
     * @param startTime ms
     * @param endTime   ms
     */
    public static native int clipAudio(String audioPath, String outPath, long startTime, long endTime);

    /**
     * @param startTime ms
     * @param endTime   ms
     */
    public static native int clipVideo(String videoPath, String outPath, long startTime, long endTime);

    /**
     * @param mediaPath video or audio path
     * @return MediaDuration ms
     */
    public native static long getMediaDuration(String mediaPath);

    public interface OnActionListener {
        void progress(float progress);

        void fail();

        void success();
    }

    public interface OnGetImageFromVideoListener {
        void onGetImageFromVideo(int index, String imagePath);
    }

    public interface OnGetBitmapFromVideoListener {
        void onGetBitmapFromVideo(int index, Bitmap bitmap);
    }

    public native static int test();

    public static enum PixelFormat {
        YUVI420, YV12, TEXTURE
    }
}
