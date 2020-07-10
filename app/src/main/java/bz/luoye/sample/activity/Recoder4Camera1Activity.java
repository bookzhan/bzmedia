package bz.luoye.sample.activity;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzcamera.BZCameraView;
import com.luoye.bzcamera.listener.CameraStateListener;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.recorder.OnRecorderErrorListener;
import com.luoye.bzmedia.recorder.VideoRecorderBase;
import com.luoye.bzmedia.recorder.VideoRecorderNative;
import com.luoye.bzyuvlib.BZYUVUtil;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class Recoder4Camera1Activity extends AppCompatActivity implements CameraStateListener {
    private final static String TAG = "bz_Recoder4Camera1";

    private int previewFormat = ImageFormat.YV12;
    private byte[] yuvBuffer;
    private int displayWidth = 0;
    private int displayHeight = 0;
    private VideoRecorderNative videoRecorderNative;
    private long logIndex = 0;
    private View bz_start_record;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recoder4_camera1);
        bz_start_record = findViewById(R.id.bz_start_record);
        BZCameraView bz_camera_view = findViewById(R.id.bz_camera_view);
        bz_camera_view.setPreviewFormat(previewFormat);
        bz_camera_view.setNeedCallBackData(true);
        bz_camera_view.setCameraStateListener(this);
    }

    public void startRecord(View view) {
        if (displayWidth <= 0 || displayHeight <= 0) {
            BZLogUtil.d(TAG, "Please wait for the preview to succeed");
            return;
        }
        bz_start_record.setEnabled(false);
        VideoRecordParams videoRecordParams = new VideoRecordParams();
        videoRecordParams.setVideoWidth(displayWidth);
        videoRecordParams.setVideoHeight(displayHeight);
        videoRecordParams.setOutputPath(FilePathUtil.getAVideoPath());
        videoRecorderNative = new VideoRecorderNative();
        videoRecorderNative.setOnVideoRecorderStateListener(new VideoRecorderBase.OnVideoRecorderStateListener() {
            @Override
            public void onVideoRecorderStarted(boolean success) {
                BZLogUtil.d(TAG, "onVideoRecorderStarted=" + success + " VideoPath=" + videoRecorderNative.getVideoPath());
            }

            @Override
            public void onVideoRecording(long recordTime) {
                if (logIndex % 30 == 0) {
                    BZLogUtil.d(TAG, "onVideoRecording recordTime=" + recordTime + " VideoPath=" + videoRecorderNative.getVideoPath());
                }
                logIndex++;
            }

            @Override
            public void onVideoRecorderStopped(String videoPath, boolean success) {
                BZLogUtil.d(TAG, "onVideoRecorderStopped success=" + success + " VideoPath=" + videoPath);
                videoRecorderNative = null;
                enableStartRecordButton();
            }
        });
        videoRecorderNative.setOnRecorderErrorListener(new OnRecorderErrorListener() {
            @Override
            public void onVideoError(int what, int extra) {
                BZLogUtil.e(TAG,"onVideoError what="+what+" extra="+extra);
                stopRecord(null);
            }

            @Override
            public void onAudioError(int what, String message) {
                BZLogUtil.e(TAG,"onAudioError what="+what+" extra="+message);
                stopRecord(null);
            }
        });
        int ret = videoRecorderNative.startRecord(videoRecordParams);
        if (ret < 0) {
            BZLogUtil.e(TAG, "startRecord fail");
            videoRecorderNative.stopRecord();
            videoRecorderNative = null;
        }
    }

    private void enableStartRecordButton(){
        bz_start_record.post(new Runnable() {
            @Override
            public void run() {
                bz_start_record.setEnabled(true);
            }
        });
    }
    public void stopRecord(View view) {
        if (null != videoRecorderNative) {
            videoRecorderNative.stopRecord();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        stopRecord(null);
    }

    @Override
    public void onPreviewSuccess(Camera camera, int width, int height) {
        BZLogUtil.d(TAG, "onPreviewSuccess");
    }

    @Override
    public void onPreviewFail(String message) {
        BZLogUtil.e(TAG, "onPreviewFail message=" + message);
    }

    @Override
    public void onPreviewDataUpdate(byte[] data, int width, int height, int displayOrientation, int cameraId) {
        if (null == yuvBuffer) {
            yuvBuffer = new byte[width * height * 3 / 2];
        }
        displayWidth = width;
        displayHeight = height;
        if (displayOrientation == 90 || displayOrientation == 270) {
            displayWidth = height;
            displayHeight = width;
        }
        if (previewFormat == ImageFormat.YV12) {
            BZYUVUtil.preHandleYV12(data, yuvBuffer, width, height, cameraId == Camera.CameraInfo.CAMERA_FACING_FRONT, displayOrientation);
        } else if (previewFormat == ImageFormat.NV21) {
            BZYUVUtil.preHandleNV21(data, yuvBuffer, width, height, cameraId == Camera.CameraInfo.CAMERA_FACING_FRONT, displayOrientation);
        }
        if (null != videoRecorderNative) {
            videoRecorderNative.updateVideoData(yuvBuffer);
        }
    }

    @Override
    public void onCameraClose() {

    }
}
