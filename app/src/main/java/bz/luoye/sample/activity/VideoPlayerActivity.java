package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;

import androidx.appcompat.app.AppCompatActivity;

import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoItem;
import com.luoye.bzmedia.widget.BZVideoView;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class VideoPlayerActivity extends AppCompatActivity {
    public static final String ARG_VIDEO_PATH = "videoPath";
    private String videoPath;
    private BZVideoView bz_video_view;
    private SeekBar seek_bar;
    private static final String TAG = "bz_VideoPlayerActivity";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_player);
        bz_video_view = findViewById(R.id.bz_video_view);
        seek_bar = findViewById(R.id.seek_bar);
        String stringExtra = getIntent().getStringExtra(ARG_VIDEO_PATH);
        if (null != stringExtra) {
            videoPath = stringExtra;
        }
        seek_bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    if (bz_video_view.isPlaying()) {
                        bz_video_view.pause();
                    }
                    bz_video_view.seek(1.0f * progress / seekBar.getMax());
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                bz_video_view.seek(1.0f * seekBar.getProgress() / seekBar.getMax());
            }
        });
        bz_video_view.setAutoStartPlay(true);
        if (null == videoPath) {
            videoPath = FilePathUtil.getReadWorkDir() + "/testvideo.mp4";
        }
        bz_video_view.addVideoItem(new VideoItem(videoPath,0, BZMedia.getMediaDuration(videoPath)));
        bz_video_view.setPlayLoop(true);
        bz_video_view.setOnProgressChangedListener(new BZVideoView.OnProgressChangedListener() {
            @Override
            public void onProgressChanged(float currentProgress, int currentMediaItemIndex, float totalProgress) {
//                BZLogUtil.d(TAG, "currentProgress=" + currentProgress + " currentMediaItemIndex=" + currentMediaItemIndex + " totalProgress=" + totalProgress);
                seek_bar.setProgress((int) (seek_bar.getMax() * totalProgress));
            }
        });
        bz_video_view.prepare();
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

    public void pause(View view) {
        bz_video_view.pause();
    }

    public void mute(View view) {
        bz_video_view.setVolume(0);
    }

    public void Flip(View view) {
        bz_video_view.setFlip(true, false);
    }

    public void addItem(View view) {
        String path = FilePathUtil.getReadWorkDir() + "/testvideo2.mp4";
        bz_video_view.addVideoItem(new VideoItem(path,0,BZMedia.getMediaDuration(path)));
    }
}
