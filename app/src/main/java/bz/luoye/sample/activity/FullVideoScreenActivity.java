package bz.luoye.sample.activity;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoItem;
import com.luoye.bzmedia.widget.BZVideoView;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class FullVideoScreenActivity extends AppCompatActivity {
    private static final String videoPath = FilePathUtil.getReadWorkDir()+"/chess-board-400x1080.mp4";
//    private static final String videoPath = FilePathUtil.getWorkDir()+"/chess-board-1920x1080.mp4";

    private BZVideoView bz_video_view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_full_video_screen);
        bz_video_view = findViewById(R.id.bz_video_view);
        bz_video_view.setAutoStartPlay(true);

        bz_video_view.setFitFullView(true);
        bz_video_view.addVideoItem(new VideoItem(videoPath, 0, BZMedia.getMediaDuration(videoPath)));
        bz_video_view.setPlayLoop(true);
        bz_video_view.prepare();
        bz_video_view.start();
    }

    @Override
    protected void onPause() {
        bz_video_view.onPause();
        bz_video_view.pause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        bz_video_view.onResume();
        bz_video_view.start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bz_video_view.release();
    }
}