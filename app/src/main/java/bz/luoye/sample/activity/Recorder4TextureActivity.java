package bz.luoye.sample.activity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.glutils.BZOpenGlUtils;
import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzcamera.BZCameraView;
import com.luoye.bzcamera.listener.OnCameraStateListener;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.recorder.OnRecorderErrorListener;
import com.luoye.bzmedia.recorder.VideoRecorderBase;
import com.luoye.bzmedia.recorder.VideoRecorderNative;
import com.luoye.bzmedia.widget.BZBaseGLSurfaceView;
import com.luoye.bzyuvlib.BZYUVUtil;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

import static android.opengl.GLES20.GL_BLEND;
import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.GL_ONE_MINUS_SRC_ALPHA;
import static android.opengl.GLES20.GL_SRC_ALPHA;
import static android.opengl.GLES20.glBlendFunc;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glEnable;
import static android.opengl.GLES20.glViewport;

public class Recorder4TextureActivity extends AppCompatActivity implements OnCameraStateListener {
    private final static String TAG = "bz_Recoder4Texture";

    private int previewFormat = ImageFormat.YV12;
    private BZBaseGLSurfaceView bzBaseGlSurfaceView;
    private Button bzStartRecord;
    private long logIndex = 0;
    private int displayWidth = 0;
    private int displayHeight = 0;
    private byte[] rgbaBuffer;
    private VideoRecorderNative videoRecorderNative = null;
    private FrameBufferUtil frameBufferUtil = null;
    private int watermarkTextureId = -1;
    private BaseProgram baseProgramSrc;
    private BaseProgram baseProgramWatermark;
    private BZCameraView bzCameraView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recoder4_texture);
        bzBaseGlSurfaceView = findViewById(R.id.bz_base_gl_surface_view);
        bzStartRecord = findViewById(R.id.bz_start_record);

        bzCameraView = findViewById(R.id.bz_camera_view);
        bzCameraView.setPreviewFormat(previewFormat);
        bzCameraView.setNeedCallBackData(true);
        bzCameraView.setOnCameraStateListener(this);

        bzBaseGlSurfaceView.setOnDrawFrameListener(new BZBaseGLSurfaceView.OnDrawFrameListener() {
            @Override
            public void onDrawFrame(int textureId) {
                if (null != videoRecorderNative) {
                    //Add a watermark
                    if (watermarkTextureId < 0) {
                        //Watermark picture
                        Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.ic_launcher);
                        watermarkTextureId = BZOpenGlUtils.loadTexture(bitmap);
                        bitmap.recycle();
                    }
                    if (null == frameBufferUtil) {
                        frameBufferUtil = new FrameBufferUtil(displayWidth, displayHeight);
                    }
                    if (null == baseProgramSrc) {
                        baseProgramSrc = new BaseProgram(false);
                    }
                    if (null == baseProgramWatermark) {
                        baseProgramWatermark = new BaseProgram(false);
                    }
                    frameBufferUtil.bindFrameBuffer();
                    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);
                    glViewport(0, 0, displayWidth, displayHeight);
                    baseProgramSrc.draw(textureId);
                    //Watermark area
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glViewport(displayWidth - 100, displayHeight - 100, 100, 100);
                    baseProgramWatermark.draw(watermarkTextureId);
                    frameBufferUtil.unbindFrameBuffer();

                    videoRecorderNative.addVideoData4Texture(frameBufferUtil.getFrameBufferTextureID());
                }
            }
        });
    }

    public void startRecord(View view) {
        if (displayWidth <= 0 || displayHeight <= 0) {
            BZLogUtil.d(TAG, "Please wait for the preview to succeed");
            return;
        }
        bzStartRecord.setEnabled(false);
        VideoRecordParams videoRecordParams = new VideoRecordParams();
        videoRecordParams.setInputWidth(displayWidth);
        videoRecordParams.setInputHeight(displayHeight);
        videoRecordParams.setTargetWidth(displayWidth / 2);
        videoRecordParams.setTargetHeight(displayHeight / 2);
        videoRecordParams.setPixelFormat(BZMedia.PixelFormat.TEXTURE);
        videoRecordParams.setNeedFlipVertical(true);
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

    public void stopRecord(View view) {
        if (null != videoRecorderNative) {
            videoRecorderNative.stopRecord();
        }
    }

    @Override
    protected void onPause() {
        bzBaseGlSurfaceView.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (null != baseProgramSrc) {
                    baseProgramSrc.release();
                    baseProgramSrc = null;
                }
                if (null != baseProgramWatermark) {
                    baseProgramWatermark.release();
                    baseProgramWatermark = null;
                }
                if (null != frameBufferUtil) {
                    frameBufferUtil.release();
                    frameBufferUtil = null;
                }
            }
        });
        super.onPause();
        bzCameraView.onPause();
        bzBaseGlSurfaceView.onPause();
        stopRecord(null);
    }

    @Override
    protected void onResume() {
        super.onResume();
        bzCameraView.onResume();
        bzBaseGlSurfaceView.onResume();
    }

    private void enableStartRecordButton() {
        bzStartRecord.post(new Runnable() {
            @Override
            public void run() {
                bzStartRecord.setEnabled(true);
            }
        });
    }

    @Override
    public void onPreviewSuccess(Camera camera, int width, int height) {

    }

    @Override
    public void onPreviewFail(String message) {

    }

    @Override
    public void onPreviewDataUpdate(byte[] data, int width, int height, int displayOrientation, int cameraId) {
        if (null == rgbaBuffer) {
            rgbaBuffer = new byte[width * height * 4];
        }
        displayWidth = width;
        displayHeight = height;
        if (displayOrientation == 90 || displayOrientation == 270) {
            displayWidth = height;
            displayHeight = width;
        }
        if (previewFormat == ImageFormat.YV12) {
            BZYUVUtil.yv12ToRGBA(data, rgbaBuffer, width, height, cameraId == Camera.CameraInfo.CAMERA_FACING_FRONT, displayOrientation);
        } else if (previewFormat == ImageFormat.NV21) {
            BZYUVUtil.nv21ToRGBA(data, rgbaBuffer, width, height, cameraId == Camera.CameraInfo.CAMERA_FACING_FRONT, displayOrientation);
        }
        bzBaseGlSurfaceView.updateData(rgbaBuffer, displayWidth, displayHeight);
    }

    @Override
    public void onCameraClose() {

    }
}
