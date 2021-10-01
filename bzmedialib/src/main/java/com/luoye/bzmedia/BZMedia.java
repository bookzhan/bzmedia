package com.luoye.bzmedia;

import android.content.Context;
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
     * Generally used for the same video, audio is split and then merged
     *
     * @param inputPaths If it is a video, it must be a video produced by the same encoder, and the width and height of the video must be the same.
     *                   If it is audio, the sampling rate should be the same, and the format should be the same
     */
    public static int mergeVideoOrAudio(String[] inputPaths, String outPutPath, OnActionListener onActionListener) {
        return mergeVideoOrAudio(inputPaths, outPutPath, true, true, onActionListener);
    }

    /**
     * The following two parameters are generally true, and the file extension must be mp4
     *
     * @param needVideo false The video stream will not be retained. At this time, the file extension needs to be m4a
     * @param needAudio false Audio is not retained, at this time the file extension must be mp4
     */
    public static native int mergeVideoOrAudio(String[] inputPaths, String outPutPath, boolean needVideo, boolean needAudio, OnActionListener onActionListener);



    public interface OnActionListener {
        void progress(float progress);

        void fail();

        void success();
    }

    public native static int test();

    public static enum PixelFormat {
        YUVI420, YV12, TEXTURE
    }
}
