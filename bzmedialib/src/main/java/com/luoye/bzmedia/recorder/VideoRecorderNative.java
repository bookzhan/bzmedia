package com.luoye.bzmedia.recorder;

import android.graphics.Bitmap;
import android.graphics.Matrix;

import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.bzcommon.utils.BZFileUtils;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.bean.VideoSize;
import com.luoye.bzyuvlib.BZYUVUtil;

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
    private byte[] yuvBuffer = null;
    private byte[] yuvCropBuffer = null;

    @Override
    public synchronized int startRecord(VideoRecordParams videoRecordParams) {
        int ret = super.startRecord(videoRecordParams);
        if (ret < 0) {
            return ret;
        }
        BZFileUtils.createNewFile(videoRecordParams.getOutputPath());

        VideoSize fitVideoSize = new VideoSize(videoRecordParams.getTargetWidth(), videoRecordParams.getTargetHeight());
        if (fitVideoSize.getVideoWidth() <= 0 || fitVideoSize.getVideoHeight() <= 0) {
            fitVideoSize = VideoTacticsManager.getFitVideoSize(videoRecordParams.getInputWidth(), videoRecordParams.getInputHeight());
        }
        //align to 16
        fitVideoSize.setVideoWidth(fitVideoSize.getVideoWidth() / 16 * 16);
        fitVideoSize.setVideoHeight(fitVideoSize.getVideoHeight() / 16 * 16);

        videoRecordParams.setTargetWidth(fitVideoSize.getVideoWidth());
        videoRecordParams.setTargetHeight(fitVideoSize.getVideoHeight());
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
    public synchronized void updateYUV420Data(byte[] data, long pts) {
        super.updateYUV420Data(data, pts);
        if (null == mVideoRecordParams) {
            return;
        }
        byte[] buffer = data;
        if (mVideoRecordParams.getInputWidth() != mVideoRecordParams.getTargetWidth()
                || mVideoRecordParams.getInputHeight() != mVideoRecordParams.getTargetHeight()) {
            if (null == yuvCropBuffer) {
                yuvCropBuffer = new byte[mVideoRecordParams.getTargetWidth() * mVideoRecordParams.getTargetHeight() * 3 / 2];
            }
            int ret = BZYUVUtil.zoomYUV420(data, yuvCropBuffer, mVideoRecordParams.getInputWidth(), mVideoRecordParams.getInputHeight(), mVideoRecordParams.getTargetWidth(), mVideoRecordParams.getTargetHeight());
            if (ret < 0) {
                BZLogUtil.e(TAG, "updateYUV420Data zoomYUV420 fail");
                return;
            }
            buffer = yuvCropBuffer;
        }
        long ret = BZMedia.addYUV420Data(nativeHandle, buffer, pts);
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
                frameBufferUtil = new FrameBufferUtil(mVideoRecordParams.getInputWidth(), mVideoRecordParams.getInputHeight());
            }
            frameBufferUtil.bindFrameBuffer();
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glViewport(0, 0, mVideoRecordParams.getInputWidth(), mVideoRecordParams.getInputHeight());
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
        if (null == mVideoRecordParams) {
            BZLogUtil.e(TAG, "addVideoData4Bitmap null==mVideoRecordParams");
            return;
        }
        if (null == bitmap || bitmap.isRecycled() || bitmap.getWidth() <= 0 || bitmap.getHeight() <= 0) {
            BZLogUtil.e(TAG, "null==bitmap||bitmap.isRecycled()||bitmap.getWidth()<=0||bitmap.getHeight()<=0");
            return;
        }
        Bitmap scaleBitmap = null;
        if (mVideoRecordParams.getTargetWidth() != bitmap.getWidth() || mVideoRecordParams.getTargetHeight() != bitmap.getHeight()) {
            scaleBitmap = scaleBitmap(bitmap, mVideoRecordParams.getTargetWidth(), mVideoRecordParams.getTargetHeight());
        }
        if (null == yuvBuffer) {
            yuvBuffer = new byte[bitmap.getWidth() * bitmap.getHeight() * 3 / 2];
        }
        int result = BZYUVUtil.bitmapToYUV420(null != scaleBitmap ? scaleBitmap : bitmap, yuvBuffer);
        if (result < 0) {
            BZLogUtil.e(TAG, "bitmapToYUV420 fail");
            return;
        }
        updateYUV420Data(yuvBuffer);
        if (null != scaleBitmap && !scaleBitmap.isRecycled()) {
            scaleBitmap.recycle();
        }
    }


    private static Bitmap scaleBitmap(Bitmap srcBitmap, float targetWidth, float targetHeight) {
        float scaleX = targetWidth / srcBitmap.getWidth();
        float scaleY = targetHeight / srcBitmap.getHeight();
        Matrix matrix = new Matrix();
        matrix.postScale(scaleX, scaleY);
        return Bitmap.createBitmap(srcBitmap, 0, 0, srcBitmap.getWidth(), srcBitmap.getHeight(), matrix, true);
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
