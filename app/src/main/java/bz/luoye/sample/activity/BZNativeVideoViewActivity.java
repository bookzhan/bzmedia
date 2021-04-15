package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;

import androidx.appcompat.app.AppCompatActivity;

import com.luoye.bzmedia.widget.BZNativeVideoView;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

@Deprecated
public class BZNativeVideoViewActivity extends AppCompatActivity {

    private BZNativeVideoView bz_native_video_view;
    private SeekBar seek_bar;
    private long lastSeekTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_b_z_native_video_view);
        bz_native_video_view = findViewById(R.id.bz_native_video_view);
        seek_bar = findViewById(R.id.seek_bar);
        seek_bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {


            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser && System.currentTimeMillis() - lastSeekTime > 100) {
                    if (bz_native_video_view.isPlaying()) {
                        bz_native_video_view.pause();
                    }
                    lastSeekTime = System.currentTimeMillis();
                    bz_native_video_view.seek(progress * 1.0f / seekBar.getMax());
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                bz_native_video_view.seek(seekBar.getProgress() * 1.0f / seekBar.getMax());
            }
        });
    }

    public void setDataSource(View view) {
        String path = FilePathUtil.getReadWorkDir() + "/testvideo.mp4";
        bz_native_video_view.setDataSource(path);
        bz_native_video_view.start();
    }

    @Override
    protected void onPause() {
        super.onPause();
        bz_native_video_view.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bz_native_video_view.release();
    }

    public void start(View view) {
        bz_native_video_view.start();
    }

    public void pause(View view) {
        bz_native_video_view.pause();
    }
}
