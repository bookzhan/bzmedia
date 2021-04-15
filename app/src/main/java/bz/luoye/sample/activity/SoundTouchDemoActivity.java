package bz.luoye.sample.activity;

import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZAssetsFileManager;
import com.bzcommon.utils.BZLogUtil;
import com.google.android.exoplayer2.MediaItem;
import com.google.android.exoplayer2.PlaybackParameters;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.audio.DefaultAudioSink;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.utils.CommonUtils;
import com.luoye.bzmedia.utils.SoundTouchUtil;

import java.util.Locale;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class SoundTouchDemoActivity extends AppCompatActivity {
    private static final String TAG = "bz_SoundTouchDemoActivity";
    private SimpleExoPlayer audioPlayer;
    private float mFinalPitch = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sound_touch_demo);
        TextView tv_pitch = findViewById(R.id.tv_pitch);
        SeekBar seek_bar = findViewById(R.id.seek_bar);
        seek_bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    if (progress < 500) {
                        mFinalPitch = progress / 500f;
                    } else {
                        mFinalPitch = 1 + (progress - 500) / 500f * (DefaultAudioSink.MAX_PITCH - 1);
                    }
                    if (mFinalPitch <= 0) {
                        return;
                    }
                    tv_pitch.setText(String.format(Locale.ENGLISH, "pitch=%.3f", mFinalPitch));
                    BZLogUtil.d(TAG, "finalPitch=" + mFinalPitch);
                    if (null != audioPlayer) {
                        audioPlayer.setPlaybackParameters(new PlaybackParameters(1, mFinalPitch));
                    }
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

    public void getVersionString(View view) {
        String versionString = SoundTouchUtil.getVersionString();
        BZLogUtil.d(TAG, "versionString=" + versionString);
    }

    public void getErrorString(View view) {
        String errorString = SoundTouchUtil.getErrorString();
        BZLogUtil.d(TAG, "errorString=" + errorString);
    }

    public void processFile(View view) {
        long startTime = System.currentTimeMillis();
        SoundTouchUtil soundTouchUtil = new SoundTouchUtil();
        soundTouchUtil.setPitchSemiTones(mFinalPitch);
//        soundTouchUtil.setTempo(-50);
//        soundTouchUtil.setSpeed(2);

        String input = BZAssetsFileManager.getFinalPath(this, "test.wav");
        String output = FilePathUtil.getWritePath(this) + "/out_process_file.wav";
        BZLogUtil.d(TAG, "output=" + output);
        soundTouchUtil.processFile(input, output);
        soundTouchUtil.close();
        BZLogUtil.d(TAG, "time cost=" + (System.currentTimeMillis() - startTime));
    }

    public void startPlay(View view) {
        if (null == audioPlayer) {
            audioPlayer = new SimpleExoPlayer.Builder(this).build();
            MediaItem mediaItem = new MediaItem.Builder().setUri(BZAssetsFileManager.getFinalPath(this, "test.wav")).build();
            audioPlayer.setMediaItem(mediaItem);
            audioPlayer.setPlaybackParameters(new PlaybackParameters(1, 1));
            audioPlayer.setRepeatMode(Player.REPEAT_MODE_ALL);
            audioPlayer.prepare();
            audioPlayer.play();
        } else if (!audioPlayer.isPlaying()) {
            audioPlayer.play();
        }
    }

    public void pause(View view) {
        if (null != audioPlayer) {
            audioPlayer.pause();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (null != audioPlayer) {
            audioPlayer.release();
            audioPlayer = null;
        }
    }

    public void processVideo(View view) {
        long startTime = System.currentTimeMillis();
        String videoPath = FilePathUtil.getReadWorkDir() + "/pre-ui-res/raw-clip1-37s-shanghai-center-101-120-floors.MP4";
        String aAudioPath = FilePathUtil.getAAudioPath(this);
        BZLogUtil.d(TAG, "aAudioPath=" + aAudioPath);
        SoundTouchUtil mSoundTouchUtil = new SoundTouchUtil();
        mSoundTouchUtil.setPitchSemiTones(1);
        mSoundTouchUtil.setChannels(1);
        mSoundTouchUtil.setSampleRate(44100);
        BZMedia.startAudioTransCode(videoPath, aAudioPath, new BZMedia.OnAudioTransCodeListener() {
            @Override
            public byte[] onPcmCallBack(byte[] pcmData) {
                short[] aShort = CommonUtils.getShort(pcmData);
                if (null == aShort) {
                    return pcmData;
                }
                mSoundTouchUtil.putSamples(aShort, aShort.length);
                short[] buffer = mSoundTouchUtil.receiveSamples(aShort.length);
                if (null == buffer || buffer.length <= 0 || aShort.length != buffer.length) {
                    return pcmData;
                }
                return CommonUtils.shortToByteSmall(buffer);
            }

            @Override
            public void transCodeProgress(float progress) {
//                BZLogUtil.d(TAG, "transCodeProgress=" + progress);
                if (progress > 0.2) {
                    mSoundTouchUtil.setPitchSemiTones(mFinalPitch);
                }
            }
        });
        mSoundTouchUtil.close();
        BZLogUtil.d(TAG, "time cost=" + (System.currentTimeMillis() - startTime));
        String videoOutPath = FilePathUtil.getAVideoPath(getBaseContext());
        BZLogUtil.d(TAG, "videoOutPath=" + videoOutPath);
        int ret = BZMedia.replaceBackgroundMusicOnly(videoPath, aAudioPath, videoOutPath, null);
    }
}