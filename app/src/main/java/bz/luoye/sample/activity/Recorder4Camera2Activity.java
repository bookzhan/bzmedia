package bz.luoye.sample.activity;

import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.media.Image;
import android.os.Build;
import android.os.Bundle;
import android.view.View;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzcamera.BZCamera2View;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.recorder.OnRecorderErrorListener;
import com.luoye.bzmedia.recorder.VideoRecorderBase;
import com.luoye.bzmedia.recorder.VideoRecorderNative;
import com.luoye.bzyuvlib.BZYUVUtil;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class Recorder4Camera2Activity extends AppCompatActivity {
    private final static String TAG = "bz_Recoder4Camera2";
    private int displayWidth = 0;
    private int displayHeight = 0;
    private View bz_start_record;
    private VideoRecorderNative videoRecorderNative;
    private long logIndex = 0;
    private BZYUVUtil bzyuvUtil = new BZYUVUtil();
    private BZCamera2View bz_camera2_view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recoder4_camera2);
        bz_start_record = findViewById(R.id.bz_start_record);
        bz_camera2_view = findViewById(R.id.bz_camera2_view);
        bz_camera2_view.setOnStatusChangeListener(new BZCamera2View.OnStatusChangeListener() {
            @Override
            public void onPreviewSuccess(CameraDevice mCameraDevice, int width, int height) {

            }

            @Override
            public void onImageAvailable(Image image, int displayOrientation, float fps) {
                displayWidth = image.getWidth();
                displayHeight = image.getHeight();
                if (displayOrientation == 90 || displayOrientation == 270) {
                    displayWidth = image.getHeight();
                    displayHeight = image.getWidth();
                }
                byte[] yuvBuffer = bzyuvUtil.preHandleYUV420(image, bz_camera2_view.getCurrentCameraLensFacing() == CameraCharacteristics.LENS_FACING_FRONT, displayOrientation);
                if (null != videoRecorderNative) {
                    videoRecorderNative.addVideoData4YUV420(yuvBuffer);
                }
            }
        });
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
        bz_camera2_view.onPause();
        stopRecord(null);
    }

    @Override
    protected void onResume() {
        super.onResume();
        bz_camera2_view.onResume();
    }
}
