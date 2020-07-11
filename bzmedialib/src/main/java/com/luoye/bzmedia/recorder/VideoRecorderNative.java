package com.luoye.bzmedia.recorder;

import android.graphics.Bitmap;

import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.bzcommon.utils.BZFileUtils;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.bean.VideoSize;

import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glViewport;
import static com.luoye.bzmedia.recorder.OnRecorderErrorListener.ERROR_UNKNOWN;

/**
 * Created by zhandalin on 2020-07-09 15:41.
 * description:
 */
public class VideoRecorderNative extends VideoRecorderBase implements AudioCapture.OnAudioFrameCapturedListener {
    private final static String TAG = "bz_VideoRecorderNative";
    private long nativeHandle = 0;
    private long recordTime = 0;
    private AudioCapture mAudioRecorder = null;
    private BaseProgram baseProgram;
    private FrameBufferUtil frameBufferUtil;
    private long lastUpdateTextureTime = 0;

    @Override
    public synchronized int startRecord(VideoRecordParams videoRecordParams) {
        int ret = super.startRecord(videoRecordParams);
        if (ret < 0) {
            return ret;
        }
        BZFileUtils.createNewFile(videoRecordParams.getOutputPath());

        VideoSize fitVideoSize;
        if (adjustVideoSize && mVideoRecordParams.getPixelFormat() != BZMedia.PixelFormat.RGBA) {
            fitVideoSize = VideoTacticsManager.getFitVideoSize(videoRecordParams.getVideoWidth(), videoRecordParams.getVideoHeight());
        } else {
            fitVideoSize = new VideoSize(videoRecordParams.getVideoWidth(), videoRecordParams.getVideoHeight());
        }
        videoRecordParams.setTargetWidth(fitVideoSize.width);
        videoRecordParams.setTargetHeight(fitVideoSize.height);
        videoRecordParams.setNbSamples(AudioCapture.getNbSamples());
        videoRecordParams.setSampleRate(44100);
        if (videoRecordParams.getBitRate() <= 0) {
            long bitRate = getBitRate(videoRecordParams.getTargetWidth(), videoRecordParams.getTargetHeight());
            if (videoRecordParams.isAllFrameIsKey()) {
                bitRate = bitRate / 2 * 3;
            }
            BZLogUtil.d(TAG, "bitRate=" + bitRate);
            videoRecordParams.setBitRate(bitRate);
        }
        nativeHandle = BZMedia.startRecord(videoRecordParams);
        if (nativeHandle < 0) {
            mRecording = false;
            if (null != mOnRecorderErrorListener)
                mOnRecorderErrorListener.onVideoError(ERROR_UNKNOWN, ERROR_UNKNOWN);

            if (null != mOnVideoRecorderStateListener)
                mOnVideoRecorderStateListener.onVideoRecorderStarted(false);
            BZLogUtil.d(TAG, "startRecord fail");
        } else {
            BZLogUtil.d(TAG, "Record start success");
            if (null != mOnVideoRecorderStateListener)
                mOnVideoRecorderStateListener.onVideoRecorderStarted(true);
        }
        if (videoRecordParams.isHasAudio()) {
            if (null != mAudioRecorder)
                mAudioRecorder.stopCapture();
            mAudioRecorder = new AudioCapture();
            mAudioRecorder.setOnAudioFrameCapturedListener(this);
            mAudioRecorder.startCapture();
        }
        mRecording = true;
        return 0;
    }

    @Override
    public synchronized void updateVideoData(byte[] data, long pts) {
        super.updateVideoData(data, pts);
        long ret = BZMedia.addVideoData(nativeHandle, data, pts);
        if (ret < 0) {
            BZLogUtil.d(TAG, "addVideoData fail");
        } else {
            callBackVideoTime(ret);
        }
    }

    @Override
    public synchronized void updateTexture(int textureId) {
        if (mVideoRecordParams.isNeedFlipVertical()) {
            if (null == baseProgram) {
                baseProgram = new BaseProgram(true);
            }
            if (null == frameBufferUtil) {
                frameBufferUtil = new FrameBufferUtil(mVideoRecordParams.getVideoWidth(), mVideoRecordParams.getVideoHeight());
            }
            frameBufferUtil.bindFrameBuffer();
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glViewport(0, 0, mVideoRecordParams.getVideoWidth(), mVideoRecordParams.getVideoHeight());
            baseProgram.draw(textureId);
            frameBufferUtil.unbindFrameBuffer();
            textureId = frameBufferUtil.getFrameBufferTextureID();
        }
        long timeMillis = System.currentTimeMillis();
        if (timeMillis - lastUpdateTextureTime >= getFrameDuration()) {
            long ret = BZMedia.updateVideoRecorderTexture(nativeHandle, textureId);
            lastUpdateTextureTime = timeMillis;
            callBackVideoTime(ret);
        }
    }

    @Override
    public synchronized void stopRecord() {
        if (null != mAudioRecorder) {
            mAudioRecorder.stopCapture();
            mAudioRecorder = null;
        }
        if (null != frameBufferUtil) {
            frameBufferUtil.release();
            frameBufferUtil = null;
        }
        if (null != baseProgram) {
            baseProgram.release();
            baseProgram = null;
        }
        //由于开始启动子线程了,设置这个标记很重要,否则会崩溃
        BZMedia.setStopRecordFlag(nativeHandle);
        if (mVideoRecordParams.isSynEncode()) {
            int ret = BZMedia.stopRecord(nativeHandle);
            nativeHandle = 0;
            mRecording = false;
            BZLogUtil.d(TAG, "stopRecord success");
            if (null != mOnVideoRecorderStateListener) {
                mOnVideoRecorderStateListener.onVideoRecorderStopped(mVideoRecordParams.getOutputPath(), ret >= 0);
                mOnVideoRecorderStateListener = null;
            }
        } else {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    int ret = BZMedia.stopRecord(nativeHandle);
                    nativeHandle = 0;
                    mRecording = false;
                    BZLogUtil.d(TAG, "stopRecord success");
                    if (null != mOnVideoRecorderStateListener) {
                        mOnVideoRecorderStateListener.onVideoRecorderStopped(mVideoRecordParams.getOutputPath(), ret >= 0);
                        mOnVideoRecorderStateListener = null;
                    }
                }
            }, "StopRecordThread").start();
        }
    }

    private void stopAll() {

    }


    @Override
    public long getRecordTime() {
        return recordTime;
    }


    public void addVideoData4Bitmap(Bitmap bitmap) {
        if (null == bitmap || bitmap.isRecycled() || bitmap.getWidth() <= 0 || bitmap.getHeight() <= 0) {
            BZLogUtil.e(TAG, "null==bitmap||bitmap.isRecycled()||bitmap.getWidth()<=0||bitmap.getHeight()<=0");
            return;
        }
        long ret = BZMedia.addVideoData4Bitmap(nativeHandle, bitmap, bitmap.getWidth(), bitmap.getHeight());
        callBackVideoTime(ret);
    }

    public void addVideoPacketData(byte[] videoPacket, long size, long pts) {
        long ret = BZMedia.addVideoPacketData(nativeHandle, videoPacket, size, pts);
        callBackVideoTime(ret);
    }

    private void callBackVideoTime(long videoTime) {
        if (!mVideoRecordParams.isHasAudio()) {
            recordTime = videoTime;
            if (null != mOnVideoRecorderStateListener) {
                mOnVideoRecorderStateListener.onVideoRecording(recordTime);
            }
        }
    }

    @Override
    public void onAudioFrameCaptured(byte[] audioData, int length) {
        if (!mRecording || null == audioData) return;
        if (null != onRecordPCMListener) {
            audioData = onRecordPCMListener.onRecordPCM(audioData);
        }
        if (null == audioData) {
            return;
        }
        recordTime = BZMedia.addAudioData(nativeHandle, audioData, length);
        if (null != mOnVideoRecorderStateListener) {
            mOnVideoRecorderStateListener.onVideoRecording(recordTime);
        }
    }

    @Override
    public void onAudioError(int what, String message) {
        if (mOnRecorderErrorListener != null)
            mOnRecorderErrorListener.onAudioError(what, message);
    }
}
