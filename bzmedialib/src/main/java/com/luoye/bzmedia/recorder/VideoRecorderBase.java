package com.luoye.bzmedia.recorder;

import com.bzcommon.utils.BZLogUtil;

import com.luoye.bzmedia.bean.VideoRecordParams;

/**
 * Created by zhandalin on 2020-07-09 15:16.
 * description:
 */
public abstract class VideoRecorderBase {
    private static final String TAG = "bz_VideoRecorderBase";
    VideoRecordParams mVideoRecordParams;
    volatile boolean mRecording;
    private boolean avPacketFromMediaCodec = false;
    boolean adjustVideoSize = true;
    OnRecorderErrorListener mOnRecorderErrorListener = null;
    OnVideoRecorderStateListener mOnVideoRecorderStateListener = null;
    OnRecordPCMListener onRecordPCMListener = null;
    private int mFrameRate = 30;

    public synchronized int startRecord(VideoRecordParams videoRecordParams) {
        mVideoRecordParams = videoRecordParams;
        if (null == videoRecordParams) {
            BZLogUtil.e(TAG, "null == videoRecordParams");
            return -1;
        }
        if (videoRecordParams.getVideoFrameRate() > 0) {
            mFrameRate = videoRecordParams.getVideoFrameRate();
        }
        BZLogUtil.d(TAG, "startRecord videoRecordParams=" + videoRecordParams);
        return 0;
    }

    public synchronized void updateVideoData(byte[] data, long pts) {

    }

    public void updateVideoData(byte[] data) {
        updateVideoData(data, -1);
    }

    public abstract void updateTexture(int textureId);

    public abstract void stopRecord();

    public void setAdjustVideoSize(boolean adjustVideoSize) {
        this.adjustVideoSize = adjustVideoSize;
    }

    public int getFrameDuration() {
        return (int) (1000.0f / mFrameRate * mVideoRecordParams.getRecordSpeed());
    }

    public long getBitRate(int width, int height) {
        int bitrateP = Math.max(width, height);
        long bit_rate;
        if (bitrateP >= 1920) {
            bit_rate = 7552;
        } else if (bitrateP >= 1080) {
            bit_rate = 4992;
        } else if (bitrateP >= 720) {
            bit_rate = 2496;
        } else if (bitrateP >= 576) {
            bit_rate = 1856;
        } else if (bitrateP >= 480) {
            bit_rate = 1216;
        } else if (bitrateP >= 432) {
            bit_rate = 1088;
        } else if (bitrateP >= 360) {
            bit_rate = 896;
        } else if (bitrateP >= 240) {
            bit_rate = 576;
        } else {
            bit_rate = 2496;
        }
        bit_rate *= 1000;
        return (long) (bit_rate * 3.5f);
    }

    public abstract long getRecordTime();

    public interface OnVideoRecorderStateListener {
        void onVideoRecorderStarted(boolean success);

        void onVideoRecording(long recordTime);

        void onVideoRecorderStopped(String videoPath, boolean success);
    }

    public void setOnRecorderErrorListener(OnRecorderErrorListener onRecorderErrorListener) {
        mOnRecorderErrorListener = onRecorderErrorListener;
    }

    public void setmOnVideoRecorderStateListener(OnVideoRecorderStateListener mOnVideoRecorderStateListener) {
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

