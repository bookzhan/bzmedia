package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.player.AudioNativePlayer;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class AudioPlayerActivity extends AppCompatActivity {

    private static final String TAG = "bz_AudioPlayerActivity";
    private long handle;
    private AudioNativePlayer audioNativePlayer;
    private SeekBar seek_bar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_player);
        seek_bar = findViewById(R.id.seek_bar);
        seek_bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser && null != audioNativePlayer) {
                    audioNativePlayer.seek(handle, (long) (audioNativePlayer.getDuration(handle) * (progress*1.0f / seekBar.getMax())));
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    public void startPlay(View view) {
        audioNativePlayer = new AudioNativePlayer();
        String audioPath = FilePathUtil.getWorkDir() + "/test.m4a";
        handle = audioNativePlayer.init(audioPath, new AudioNativePlayer.OnProgressChangedListener() {
            @Override
            public void onProgressChanged(float progress) {
                BZLogUtil.d(TAG, "onProgressChanged=" + progress);
                seek_bar.setProgress((int) (seek_bar.getMax() * progress));
            }
        });
        audioNativePlayer.start(handle);
    }

    public void stopPlay(View view) {
        audioNativePlayer.pause(handle);
        audioNativePlayer.stop(handle);
        audioNativePlayer.release(handle);
        handle = 0;
    }
}