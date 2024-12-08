package com.luoye.bzmedia.recorder;

import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glViewport;
import static com.luoye.bzmedia.recorder.OnRecorderErrorListener.ERROR_UNKNOWN;

import android.graphics.Bitmap;

import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.bzcommon.utils.BZBitmapUtil;
import com.bzcommon.utils.BZFileUtils;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.bean.VideoSize;
import com.luoye.bzmedia.utils.VideoUtil;
import com.luoye.bzyuvlib.BZYUVUtil;

import java.io.File;

/**
 * Created by bookzhan on 2020-07-09 15:41.
 * description:
 */
public class VideoRecorderNative extends VideoRecorderBase implements AudioCapture.OnAudioFrameCapturedListener {
    private final static String TAG = "bz_VideoRecorderNative";
    private long nativeHandle = 0;
    private long recordTime = 0;
    private AudioCapture mAudioRecorder = null;
    private BaseProgram baseProgram;
    private FrameBufferUtil frameBufferUtil;
    private byte[] yuvBuffer = null;
    private byte[] yuvCropBuffer = null;
    private long startRecordTime = -1;
    private long frameIndex = 0;


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
        //align
        fitVideoSize.setVideoWidth(fitVideoSize.getVideoWidth() / ALIGN_BYTE * ALIGN_BYTE);
        fitVideoSize.setVideoHeight(fitVideoSize.getVideoHeight() / ALIGN_BYTE * ALIGN_BYTE);

        videoRecordParams.setTargetWidth(fitVideoSize.getVideoWidth());
        videoRecordParams.setTargetHeight(fitVideoSize.getVideoHeight());
        videoRecordParams.setNbSamples(AudioCapture.getNbSamples());
        videoRecordParams.setSampleRate(44100);
        if (videoRecordParams.getBitRate() <= 0) {
            long bitRate = VideoUtil.getDefaultBitRate(videoRecordParams.getTargetWidth(), videoRecordParams.getTargetHeight());
            if (videoRecordParams.isAllFrameIsKey()) {
                bitRate = bitRate / 2 * 3;
            }
            BZLogUtil.d(TAG, "bitRate=" + bitRate);
            videoRecordParams.setBitRate(bitRate);
        }
        nativeHandle = BZMedia.startRecord(videoRecordParams);
        if (nativeHandle == 0) {
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
        if (videoRecordParams.isNeedAudio()) {
            if (null != mAudioRecorder)
                mAudioRecorder.stopCapture();
            mAudioRecorder = new AudioCapture();
            mAudioRecorder.setOnAudioFrameCapturedListener(this);
            mAudioRecorder.startCapture();
        }
        mRecording = true;
        frameIndex = 0;
        return 0;
    }

    /**
     * @param pts ms*1000
     */
    @Override
    public synchronized void addVideoData4YUV420(byte[] data, long pts) {
        if (null == mVideoRecordParams) {
            return;
        }
        frameIndex++;
        if (pts < 0 && startRecordTime < 0) {
            startRecordTime = System.currentTimeMillis();
        }
        if (pts < 0 && !mVideoRecordParams.isNeedAudio()) {
            pts = (System.currentTimeMillis() - startRecordTime) * 1000;
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
            if (frameIndex % 30 == 0) {
                BZLogUtil.d(TAG, "addVideoData4YUV420--zoomYUV420--");
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
    public synchronized void addVideoData4Texture(int textureId, long pts) {
        if (pts < 0 && startRecordTime < 0) {
            startRecordTime = System.currentTimeMillis();
        }
        if (pts < 0 && !mVideoRecordParams.isNeedAudio()) {
            pts = (System.currentTimeMillis() - startRecordTime) * 1000;
        }
        if (mVideoRecordParams.isNeedFlipVertical()) {
            if (null == baseProgram) {
                baseProgram = new BaseProgram(true);
            }
            if (null == frameBufferUtil) {
                frameBufferUtil = new FrameBufferUtil(mVideoRecordParams.getTargetWidth(), mVideoRecordParams.getTargetHeight());
            }
            frameBufferUtil.bindFrameBuffer();
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glViewport(0, 0, mVideoRecordParams.getTargetWidth(), mVideoRecordParams.getTargetHeight());
            baseProgram.draw(textureId);
            frameBufferUtil.unbindFrameBuffer();
            textureId = frameBufferUtil.getFrameBufferTextureID();
        }
        long ret = BZMedia.updateVideoRecorderTexture(nativeHandle, textureId, pts);
        if (ret < 0) {
            BZLogUtil.d(TAG, "addVideoData fail");
        } else {
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
            startRecordTime = -1;
            mRecording = false;
            BZLogUtil.d(TAG, "stopRecord success");
            adjustVideoSpeed();
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
                    startRecordTime = -1;
                    BZLogUtil.d(TAG, "stopRecord success");
                    adjustVideoSpeed();
                    if (null != mOnVideoRecorderStateListener) {
                        mOnVideoRecorderStateListener.onVideoRecorderStopped(mVideoRecordParams.getOutputPath(), ret >= 0);
                        mOnVideoRecorderStateListener = null;
                    }
                }
            }, "StopRecordThread").start();
        }
    }

    private void adjustVideoSpeed() {
        if (null == mVideoRecordParams) {
            return;
        }
        if (Math.abs(mVideoRecordParams.getRecordSpeed() - 1) > 0.01) {
            BZLogUtil.d(TAG, "start adjustVideoSpeed");
            String absolutePath = new File(mVideoRecordParams.getOutputPath()).getParentFile().getAbsolutePath() + "/temp_" + System.currentTimeMillis() + ".mp4";
            BZFileUtils.createNewFile(absolutePath);
            int ret = BZMedia.adjustVideoSpeed(mVideoRecordParams.getOutputPath(), absolutePath, mVideoRecordParams.getRecordSpeed());
            if (ret >= 0) {
                BZFileUtils.deleteFile(mVideoRecordParams.getOutputPath());
                boolean rename = new File(absolutePath).renameTo(new File(mVideoRecordParams.getOutputPath()));
                BZLogUtil.d(TAG, "rename File successful=" + rename);
            } else {
                BZLogUtil.e(TAG, "adjustVideoSpeed fail path=" + absolutePath);
            }
        }
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
            scaleBitmap = BZBitmapUtil.scaleBitmap4Fix(bitmap, mVideoRecordParams.getTargetWidth(), mVideoRecordParams.getTargetHeight());
        }
        if (null == yuvBuffer) {
            yuvBuffer = new byte[bitmap.getWidth() * bitmap.getHeight() * 3 / 2];
        }
        int result = BZYUVUtil.bitmapToYUV420(null != scaleBitmap ? scaleBitmap : bitmap, yuvBuffer);
        if (result < 0) {
            BZLogUtil.e(TAG, "bitmapToYUV420 fail");
            return;
        }
        addVideoData4YUV420(yuvBuffer);
        if (null != scaleBitmap && !scaleBitmap.isRecycled()) {
            scaleBitmap.recycle();
        }
    }

    /**
     * @param pts ms*1000
     */
    public void addVideoPacketData(byte[] videoPacket, long size, long pts) {
        long ret = BZMedia.addVideoPacketData(nativeHandle, videoPacket, size, pts);
        callBackVideoTime(ret);
    }

    private void callBackVideoTime(long videoTime) {
        if (!mVideoRecordParams.isNeedAudio()) {
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
