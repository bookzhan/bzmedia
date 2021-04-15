package com.luoye.bzmedia;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Build;

import com.bzcommon.glutils.BZOpenGlUtils;
import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZSpUtils;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.bean.VideoTransCodeParams;
import com.luoye.bzmedia.utils.FFmpegCMDUtil;


/**
 * Created by bookzhan on 2020-05-25 16:27.
 * description:
 */
public class BZMedia {
    public static final int MEDIA_INFO_WHAT_VIDEO_DURATION = 1;
    public static final int MEDIA_INFO_WHAT_VIDEO_ROTATE = 2;
    public static final int MEDIA_INFO_WHAT_VIDEO_WIDTH = 3;
    public static final int MEDIA_INFO_WHAT_VIDEO_HEIGHT = 4;

    static {
        System.loadLibrary("bzffmpeg");
        System.loadLibrary("bzffmpegcmd");
        System.loadLibrary("bzmedia");
    }

    public static int init(Context context, boolean isDebug) {
        BZSpUtils.init(context);
        BZLogUtil.setShowLog(isDebug);
        FFmpegCMDUtil.showLog(isDebug);
        BZOpenGlUtils.detectOpenGLES30(context);
        return initNative(context, isDebug, Build.VERSION.SDK_INT);
    }

    private native static int initNative(Context context, boolean isDebug, int sdkInt);

    /**
     * @param videoRecordParams Recording parameters
     * @return Recording time, in milliseconds, less than 0 failed to start recording
     */
    public native static long startRecord(VideoRecordParams videoRecordParams);

    public native static long addAudioData(long nativeHandle, byte[] audioData, int length);

    public native static int setStopRecordFlag(long nativeHandle);

    public static native int stopRecord(long nativeHandle);

    public static native long updateVideoRecorderTexture(long nativeHandle, int textureId,long pts);

    public static native long addYUV420Data(long nativeHandle, byte[] data, long pts);

    public static native long addVideoPacketData(long nativeHandle, byte[] videoPacket, long size, long pts);

    public static native int adjustVideoSpeed(String srcVideoPath, String outputPath, float speed);

    /**
     * @param speed 0.1~100
     * @return >=0 successful, <0 fail
     */
    public static native int adjustAudioSpeed(String audioPath, String outputPath, float speed, OnActionListener onActionListener);

    /**
     * @param speed          0.1~100,speed>=0
     * @param startDelayTime ms
     * @param fadeInTime     ms
     * @param fadeOutTime    ms
     * @return >=0 successful, <0 fail
     */
    public static native int preHandleAudio(String audioPath, String outputPath, float speed, float volume, long startDelayTime, float fadeInTime, float fadeOutTime, long durationTime, OnActionListener onActionListener);

    public static native int mixAudios(String outputPath, String[] audios, OnActionListener onActionListener);

    /**
     * Mute for a while at the beginning
     *
     * @return >=0 successful, <0 fail
     */
    public static native int delayMusic(String audioPath, String outputPath, long time, OnActionListener onActionListener);

    /**
     * @param srcMusicVolume 0~1
     * @param bgMusicVolume  0~1
     * @return >=0 successful, <0 fail
     */
    public static native int addBackgroundMusic(String inputPath, String outputPath,
                                                String musicPath, float srcMusicVolume, float bgMusicVolume, OnActionListener onActionListener);

    /**
     * If only replacing background music, this function is more efficient than addBackgroundMusic
     * It automatically processes the same length of time
     *
     * @param musicPath The format must be m4a, and the encoding method must be aac
     * @return >=0 successful, <0 fail
     */
    public static native int replaceBackgroundMusic(String videoPath, String musicPath, String outputPath, OnActionListener onActionListener);

    /**
     * If only replacing background music, this function is more efficient than addBackgroundMusic
     * It automatically processes the same length of time
     *
     * @param musicPath The format must be m4a, and the encoding method must be aac
     * @return >=0 successful, <0 fail
     */
    public static native int replaceBackgroundMusicOnly(String videoPath, String musicPath, String outputPath, OnActionListener onActionListener);

    public static int getBitmapFromVideo(String videoPath, int imageCount, int scale2Width, OnGetBitmapFromVideoListener onGetBitmapFromVideoListener) {
        return getBitmapFromVideo(videoPath, imageCount, scale2Width, -1, -1, onGetBitmapFromVideoListener);
    }

    public static native int getBitmapFromVideo(String videoPath, int imageCount, int scale2Width, long startTime, long endTime, OnGetBitmapFromVideoListener onGetBitmapFromVideoListener);


    /**
     * Return audio waveform data
     */
    public static native int getAudioFeatureInfo(String audioPath, int samples, AudioFeatureInfoListener audioFeatureInfoListener);


    public static native Bitmap getVideoFrameAtTime(String videoPath, long time, int scale2Width);

    /**
     * @param startTime ms
     * @param endTime   ms
     */
    public static native int clipAudio(String audioPath, String outPath, long startTime, long endTime);

    /**
     * @param startTime ms
     * @param endTime   ms
     * @return If the number of keyframes is too small, it will be inaccurate
     */
    @Deprecated
    public static native int clipVideo(String videoPath, String outPath, long startTime, long endTime);


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


    /**
     * Combine audio of different channels with different sampling rates
     */
    public static native int mergeAudio(String[] inputPaths, String outPutPath, OnActionListener onActionListener);

    public static native boolean hasAudio(String path);

    /**
     * @param mediaPath video or audio path
     * @return MediaDuration ms
     */
    public native static long getMediaDuration(String mediaPath);

    public static native int startVideoTransCode(VideoTransCodeParams videoTransCodeParams, OnActionListener onActionListener);

    public static native int stopVideoTransCode();

    public native static int getVideoInfo(String videoPath, OnSendMediaInfoListener sendMediaInfoListener);

    /**
     * @param fadeInDuration  ms
     * @param fadeOutDuration ms
     * @return >=0 success
     */
    public static native int audioFade(String mediaPath, String outPath, boolean needFadeIn, long fadeInDuration, boolean needFadeOut, long fadeOutDuration);

    public native static int printVideoTimeStamp(String videoPath);

    /**
     * @return <0 fail
     */
    public native static int separateAudioStream(String videoPath, String outPath);

    public native static int repeatVideoToDuration(String inputPath, String outPath, long duration);

    /**
     * Initialize the GL environment at the Native
     *
     * @return Native Handle
     */
    public native static long initGLContext(int width, int height);

    public native static int releaseEGLContext(long nativeHandle);

    public static native Bitmap bzReadPixelsNative(int startX, int startY, int width, int height);

    public static native int setVideoDuration(String inPath, String outVideoPath, int duration);

    public static native boolean videoIsSupportForBZMedia(String inPath);

    public static boolean videoIsSupportForExoPlayer(String inPath) {
        if (null == inPath) {
            return false;
        }
        int indexOf = inPath.lastIndexOf(".");
        if (indexOf < 0) {
            return false;
        }
        String substring = inPath.substring(indexOf + 1);
        if (!substring.equalsIgnoreCase("mp4")
                && !substring.equalsIgnoreCase("WebM")
                && !substring.equalsIgnoreCase("MP3")
                && !substring.equalsIgnoreCase("M4A")
                && !substring.equalsIgnoreCase("AAC")
                && !substring.equalsIgnoreCase("WAV")
                && !substring.equalsIgnoreCase("AMR")
                && !substring.equalsIgnoreCase("Ogg")
                && !substring.equalsIgnoreCase("FLAC")
                && !substring.equalsIgnoreCase("3gp")
                && !substring.equalsIgnoreCase("mkv")
                && !substring.equalsIgnoreCase("FLV")) {
            return false;
        }
        return videoCodecIsSupportForExoPlayer(inPath);
    }

    private static native boolean videoCodecIsSupportForExoPlayer(String inPath);

    public static native int startAudioTransCode(String inputPath, String outputPath, OnAudioTransCodeListener onAudioTransCodeListener);

    public interface OnAudioTransCodeListener {
        byte[] onPcmCallBack(byte[] pcmData);

        void transCodeProgress(float progress);
    }

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

    public interface OnSendMediaInfoListener {
        void sendMediaInfo(int what, int extra);
    }

    public interface AudioFeatureInfoListener {
        void onAudioFeatureInfo(long audioTime, float featureValue);
    }

    public native static int test();

    public static enum PixelFormat {
        YUVI420, YV12, TEXTURE
    }
}
