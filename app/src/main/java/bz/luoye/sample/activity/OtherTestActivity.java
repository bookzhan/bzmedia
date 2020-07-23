package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class OtherTestActivity extends AppCompatActivity {
    private static final String TAG = "bz_OtherTestActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_other_test);
    }

    public void addBackgroundMusic(View view) {
        String inputPath = FilePathUtil.getWorkDir() + "/VID_29.mp4";
        final String outputPath = FilePathUtil.getAVideoPath();
        String musicPath = FilePathUtil.getWorkDir() + "/mix_4.m4a";
        float srcMusicVolume = 1;
        float bgMusicVolume = 0.5f;
        int ret = BZMedia.addBackgroundMusic(inputPath, outputPath, musicPath, srcMusicVolume, bgMusicVolume, new BZMedia.OnActionListener() {
            @Override
            public void progress(float progress) {
                BZLogUtil.d(TAG, "progress=" + progress);
            }

            @Override
            public void fail() {
                BZLogUtil.e(TAG, "addBackgroundMusic fail");
            }

            @Override
            public void success() {
                BZLogUtil.d(TAG, "addBackgroundMusic success outputPath="+outputPath);
            }
        });
        if (ret < 0) {
            BZLogUtil.e(TAG, "addBackgroundMusic fail ret=" + ret);
        }
    }

    public void replaceBackgroundMusic(View view) {
        String inputPath = FilePathUtil.getWorkDir() + "/VID_29.mp4";
        final String outputPath = FilePathUtil.getAVideoPath();
        String musicPath = FilePathUtil.getWorkDir() + "/mix_4.m4a";
        int ret = BZMedia.replaceBackgroundMusic(inputPath, musicPath, outputPath, new BZMedia.OnActionListener() {
            @Override
            public void progress(float progress) {
                BZLogUtil.d(TAG, "progress=" + progress);
            }

            @Override
            public void fail() {
                BZLogUtil.e(TAG, "replaceBackgroundMusic fail");
            }

            @Override
            public void success() {
                BZLogUtil.d(TAG, "replaceBackgroundMusic success outputPath="+outputPath);
            }
        });
        if (ret < 0) {
            BZLogUtil.e(TAG, "replaceBackgroundMusic fail ret=" + ret);
        }
    }

}
