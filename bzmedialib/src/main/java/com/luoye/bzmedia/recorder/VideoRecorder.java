package com.luoye.bzmedia.recorder;

import com.luoye.bzmedia.bean.VideoRecordParams;

/**
 * Created by bookzhan on 2018-12-10 14:05.
 * Note: The upper Java class of the underlying class VideoRecorder is convenient for other libraries to call, without worrying about whether it is called GL2 or GL3
 */
public class VideoRecorder {
    public native long initVideoRecorder();

    public native int startRecord(long nativeHandle, VideoRecordParams videoRecordParams);

    public native long addAudioData(long nativeHandle, byte[] data, int dataLength, long audioPts);

    public native long addYUV420Data(long nativeHandle, byte[] data, long pts);

    public native int updateTexture(long nativeHandle, int textureId, long videoPts);

    public native int setStopRecordFlag(long nativeHandle);

    public native int stopRecord(long nativeHandle);

    public native void releaseRecorder(long nativeHandle);
}