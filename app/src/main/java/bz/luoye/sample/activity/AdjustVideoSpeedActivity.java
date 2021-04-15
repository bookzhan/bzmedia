package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoItem;
import com.luoye.bzmedia.widget.BZVideoView;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class AdjustVideoSpeedActivity extends AppCompatActivity {

    private BZVideoView bz_video_view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_adjust_video_speed);
        bz_video_view = findViewById(R.id.bz_video_view);
    }


    public void start(View view) {
        String path = FilePathUtil.getReadWorkDir() + "/number_360.mp4";
        bz_video_view.addVideoItem(new VideoItem(path, 0, BZMedia.getMediaDuration(path)));
        bz_video_view.prepare();
        bz_video_view.start();
    }

    public void setSpeed(View view) {
        bz_video_view.setSpeed(20);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bz_video_view.pause();
        bz_video_view.release();
    }

}