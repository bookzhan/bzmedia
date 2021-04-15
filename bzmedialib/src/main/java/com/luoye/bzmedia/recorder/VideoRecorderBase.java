package com.luoye.bzmedia.recorder;

import com.bzcommon.utils.BZLogUtil;

import com.luoye.bzmedia.bean.VideoRecordParams;

/**
 * Created by bookzhan on 2020-07-09 15:16.
 * description:
 */
public abstract class VideoRecorderBase {
    private static final String TAG = "bz_VideoRecorderBase";
    VideoRecordParams mVideoRecordParams;
    volatile boolean mRecording;
    private boolean avPacketFromMediaCodec = false;
    OnRecorderErrorListener mOnRecorderErrorListener = null;
    OnVideoRecorderStateListener mOnVideoRecorderStateListener = null;
    OnRecordPCMListener onRecordPCMListener = null;
    protected final int ALIGN_BYTE = 8;

    public synchronized int startRecord(VideoRecordParams videoRecordParams) {
        mVideoRecordParams = videoRecordParams;
        if (null == videoRecordParams) {
            BZLogUtil.e(TAG, "null == videoRecordParams");
            return -1;
        }
        if (null == mVideoRecordParams.getOutputPath() || mVideoRecordParams.getInputWidth() <= 0 || mVideoRecordParams.getInputHeight() <= 0) {
            BZLogUtil.e(TAG, "null == mVideoRecordParams.getOutputPath() || mVideoRecordParams.getVideoWidth() <= 0 || mVideoRecordParams.getVideoHeight() <= 0");
            return -1;
        }
        BZLogUtil.d(TAG, "startRecord videoRecordParams=" + videoRecordParams);
        return 0;
    }

    public String getVideoPath() {
        if (null == mVideoRecordParams) {
            return null;
        }
        return mVideoRecordParams.getOutputPath();
    }

    /**
     * @param pts ms*1000
     */
    public abstract void addVideoData4YUV420(byte[] data, long pts);

    public void addVideoData4YUV420(byte[] data) {
        addVideoData4YUV420(data, -1);
    }

    public void addVideoData4Texture(int textureId) {
        addVideoData4Texture(textureId, -1);
    }

    /**
     * @param textureId Call in GL thread
     */
    public abstract void addVideoData4Texture(int textureId, long pts);

    public abstract void stopRecord();


    public abstract long getRecordTime();

    public interface OnVideoRecorderStateListener {
        void onVideoRecorderStarted(boolean success);

        void onVideoRecording(long recordTime);

        void onVideoRecorderStopped(String videoPath, boolean success);
    }

    public void setOnRecorderErrorListener(OnRecorderErrorListener onRecorderErrorListener) {
        mOnRecorderErrorListener = onRecorderErrorListener;
    }

    public void setOnVideoRecorderStateListener(OnVideoRecorderStateListener mOnVideoRecorderStateListener) {
        this.mOnVideoRecorderStateListener = mOnVideoRecorderStateListener;
    }

    public void setOnRecordPCMListener(OnRecordPCMListener onRecordPCMListener) {
        this.onRecordPCMListener = onRecordPCMListener;
    }

    public boolean isAvPacketFromMediaCodec() {
        return avPacketFromMediaCodec;
    }

    public void setAvPacketFromMediaCodec(boolean avPacketFromMediaCodec) {
        this.avPacketFromMediaCodec = avPacketFromMediaCodec;
    }
}

