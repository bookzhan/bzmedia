package com.luoye.bzmedia.player;

/**
 * Created by bookzhan on 2019-03-18 17:29.
 * 说明:
 */
public class VideoPlayer {
    /**
     * 底层初始化,做了打开视频,音频,并打开对应的解码器
     */
    public static native long init(String videoPath, VideoPlayerStateListener listener, boolean userSoftDecode);

    public static native int release(long nativeHandle);

    public static native void seek(long nativeHandle, long videoTime);

    public static native long onDrawFrame(long nativeHandle, long time);

    public static native void pause(long nativeHandle, boolean isPause);

    public static native void onPause(long nativeHandle);

    public static native void setVolume(long nativeHandle, float volume);

    public static native long getCurrentAudioPts(long nativeHandle);

    public static native void setPlayLoop(long nativeHandle, boolean isLoop);

    public static native boolean isPlaying(long nativeHandle);

    public interface VideoPlayerStateListener {
        void onProgressChanged(float progress);

        void onVideoPlayCompletion(int flag);

        void onVideoInfoAvailable(int videoWidth, int videoHeight, int videoRotate, long videoDuration, float videoFps);
    }
}
