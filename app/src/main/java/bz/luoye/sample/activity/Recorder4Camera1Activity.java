package bz.luoye.sample.activity;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzcamera.BZCameraView;
import com.luoye.bzcamera.listener.OnCameraStateListener;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.recorder.OnRecorderErrorListener;
import com.luoye.bzmedia.recorder.VideoRecorderBase;
import com.luoye.bzmedia.recorder.VideoRecorderNative;
import com.luoye.bzyuvlib.BZYUVUtil;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class Recorder4Camera1Activity extends AppCompatActivity implements OnCameraStateListener {
    private final static String TAG = "bz_Recoder4Camera1";

    private int previewFormat = ImageFormat.NV21;
    private byte[] yuvBuffer;
    private int displayWidth = 0;
    private int displayHeight = 0;
    private VideoRecorderNative videoRecorderNative;
    private long logIndex = 0;
    private View bz_start_record;
    private BZCameraView bz_camera_view;
    private long startTime = 0;
    private long frameIndex = 0;
    private TextView tv_info;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recoder4_camera1);
        bz_start_record = findViewById(R.id.bz_start_record);
        bz_camera_view = findViewById(R.id.bz_camera_view);
        bz_camera_view.setPreviewTargetSize(480, 640);
        bz_camera_view.setPreviewFormat(previewFormat);
        bz_camera_view.setOnCameraStateListener(this);

        tv_info = findViewById(R.id.tv_info);
    }

    public void startRecord(View view) {
        if (displayWidth <= 0 || displayHeight <= 0) {
            BZLogUtil.d(TAG, "Please wait for the preview to succeed");
            return;
        }
        bz_start_record.setEnabled(false);
        VideoRecordParams videoRecordParams = new VideoRecordParams();
        videoRecordParams.setInputWidth(displayWidth);
        videoRecordParams.setInputHeight(displayHeight);
        videoRecordParams.setOutputPath(FilePathUtil.getAVideoPath(this));

        videoRecorderNative = new VideoRecorderNative();
        videoRecorderNative.setOnVideoRecorderStateListener(new VideoRecorderBase.OnVideoRecorderStateListener() {
            @Override
            public void onVideoRecorderStarted(boolean success) {
                BZLogUtil.d(TAG, "onVideoRecorderStarted=" + success + " VideoPath=" + videoRecorderNative.getVideoPath());
            }

            @Override
            public void onVideoRecording(long recordTime) {
                if (logIndex % 60 == 0) {
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
                BZLogUtil.e(TAG, "onVideoError what=" + what + " extra=" + extra);
                stopRecord(null);
            }

            @Override
            public void onAudioError(int what, String message) {
                BZLogUtil.e(TAG, "onAudioError what=" + what + " extra=" + message);
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

    private void enableStartRecordButton() {
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
        bz_camera_view.onPause();
        stopRecord(null);
    }

    @Override
    protected void onResume() {
        super.onResume();
        bz_camera_view.onResume();
    }

    @Override
    public void onPreviewSuccess(Camera camera, int width, int height) {
        BZLogUtil.d(TAG, "onPreviewSuccess");
        startTime = 0;
        frameIndex = 0;
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
            videoRecorderNative.addVideoData4YUV420(yuvBuffer);
        }

        //show fps
        if (startTime <= 0) {
            startTime = System.currentTimeMillis();
        }
        frameIndex++;
        long time = System.currentTimeMillis() - startTime;
        if (time < 30) {
            time = 30;
        }
        float fps = frameIndex / (time / 1000.f);
        final String info = "width=" + displayWidth + " height=" + displayHeight + " fps=" + fps;
        tv_info.post(new Runnable() {
            @Override
            public void run() {
                tv_info.setText(info);
            }
        });
    }

    @Override
    public void onCameraClose() {

    }

}
