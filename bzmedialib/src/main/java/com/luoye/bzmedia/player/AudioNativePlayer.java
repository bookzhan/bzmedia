package com.luoye.bzmedia.player;

/**
 * Created by bookzhan on 2020-08-10 17:25.
 * description:
 */
@Deprecated
public class AudioNativePlayer {
    public native synchronized long init(String audioPath, OnProgressChangedListener onProgressChangedListener);

    public native synchronized int seek(long handle, long time);

    public native synchronized int setPlayLoop(long handle, boolean isLoop);

    public native synchronized void setAudioVolume(long handle, float volume);

    public native synchronized long getDuration(long handle);

    public native synchronized int pause(long handle);

    public native synchronized int start(long handle);

    public native synchronized int release(long handle);

    public interface OnProgressChangedListener {
        void onProgressChanged(float progress);
    }
}
