package bz.luoye.sample.activity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.recorder.VideoRecorderBase;
import com.luoye.bzmedia.recorder.VideoRecorderNative;

import java.util.Locale;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class Recorder4BitmapActivity extends AppCompatActivity {
    private static final String TAG = "bz_Recoder4Bitmap";

    private long logIndex = 0;
    private String rootPath = FilePathUtil.getWorkDir() + "/outpic";
    private VideoRecorderNative videoRecorderNative;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recoder4_bitmap);
    }

    public void start(View view) {
        Bitmap bitmap = BitmapFactory.decodeFile(rootPath + "/image1.jpeg");
        VideoRecordParams videoRecordParams = new VideoRecordParams();
        videoRecordParams.setOutputPath(FilePathUtil.getAVideoPath());
        videoRecordParams.setInputWidth(bitmap.getWidth());
        videoRecordParams.setInputHeight(bitmap.getHeight());
        videoRecordParams.setHasAudio(false);
        videoRecordParams.setVideoFrameRate(15);

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
            }
        });

        int ret = videoRecorderNative.startRecord(videoRecordParams);
        if (ret < 0) {
            BZLogUtil.e(TAG, "startRecord fail");
            videoRecorderNative.stopRecord();
            videoRecorderNative = null;
        }

        for (int i = 0; i < 60; i++) {
            String path = String.format(Locale.CHINA, rootPath + "/image%d.jpeg", i + 1);
            Bitmap decodeFile = BitmapFactory.decodeFile(path);
            videoRecorderNative.addVideoData4Bitmap(decodeFile);
            if (!decodeFile.isRecycled()) {
                decodeFile.recycle();
            }
        }
        videoRecorderNative.stopRecord();
    }
}
