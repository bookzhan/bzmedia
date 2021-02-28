package bz.luoye.sample.activity;

import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.SeekBar;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.widget.BZVideoView;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class VideoPlayerActivity extends AppCompatActivity {
    private String videoPath = FilePathUtil.getWorkDir() + "/testvideo.mp4";
    private BZVideoView bz_video_view;
    private SeekBar seek_bar;
    private static final String TAG = "bz_VideoPlayerActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_player);
        bz_video_view = findViewById(R.id.bz_video_view);
        seek_bar = findViewById(R.id.seek_bar);
        String stringExtra = getIntent().getStringExtra("videoPath");
        if (null != stringExtra) {
            videoPath = stringExtra;
        }
        seek_bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    bz_video_view.seek(1.0f * progress / seekBar.getMax());
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                bz_video_view.startSeek();
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                bz_video_view.stopSeek();
            }
        });
//        bz_video_view.setFitFullView(true);
        bz_video_view.setAutoStartPlay(true);
        bz_video_view.setDataSource(this.videoPath);
        bz_video_view.setPrepareSyn(true);
//        bz_video_view.setDataSource(BZAssetsFileManager.getFinalPath(this,"sexy.mp4"));
        bz_video_view.setPlayLoop(true);
        bz_video_view.setOnProgressChangedListener(new BZVideoView.OnProgressChangedListener() {
            @Override
            public void onProgressChanged(float progress) {
                seek_bar.setProgress((int) (seek_bar.getMax() * progress));
            }
        });
        bz_video_view.setOnStartRenderListener(new BZVideoView.OnStartRenderListener() {
            @Override
            public void onStartRender() {
                BZLogUtil.d(TAG, "onStartRender");
            }
        });
        bz_video_view.start();
    }

    @Override
    protected void onPause() {
        bz_video_view.pause();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bz_video_view.release();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }


    public void start(View view) {
        bz_video_view.start();
    }
}
