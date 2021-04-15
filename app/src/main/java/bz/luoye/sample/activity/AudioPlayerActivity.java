package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;

import androidx.appcompat.app.AppCompatActivity;

import com.luoye.bzmedia.player.AudioNativePlayer;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

@Deprecated
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
                    audioNativePlayer.seek(handle, (long) (audioNativePlayer.getDuration(handle) * (progress * 1.0f / seekBar.getMax())));
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        SeekBar seek_bar_volume = findViewById(R.id.seek_bar_volume);
        seek_bar_volume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser && null != audioNativePlayer) {
                    audioNativePlayer.setAudioVolume(handle, 1.0f * progress / seekBar.getMax());
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
        String audioPath = FilePathUtil.getReadWorkDir() + "/input_2.WAV";
        handle = audioNativePlayer.init(audioPath, new AudioNativePlayer.OnProgressChangedListener() {
            @Override
            public void onProgressChanged(float progress) {
//                BZLogUtil.d(TAG, "onProgressChanged=" + progress);
                seek_bar.setProgress((int) (seek_bar.getMax() * progress));
            }
        });
        audioNativePlayer.start(handle);
    }

    public void stopPlay(View view) {
        audioNativePlayer.pause(handle);
        audioNativePlayer.release(handle);
        handle = 0;
    }

    public void start(View view) {
        if (null == audioNativePlayer) {
            return;
        }
        audioNativePlayer.start(handle);
    }

    public void pause(View view) {
        if (null == audioNativePlayer) {
            return;
        }
        audioNativePlayer.pause(handle);
    }
}