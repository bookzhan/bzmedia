package com.luoye.bzmedia.player;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import com.bzcommon.utils.BZLogUtil;

/**
 * Created by bookzhan on 2018-11-28 15:54.
 * Description: call from jni
 */
public class PCMPlayer {
    private AudioTrack audioTrack = null;
    private String TAG = "bz_PCMPlayer";
    private float volume = 1;

    public void init(int sampleRateInHz, int channelCount) {
        BZLogUtil.d(TAG, "init sampleRateInHz=" + sampleRateInHz + " channelCount=" + channelCount);
        if (null != audioTrack) {
            stopAudioTrack();
            audioTrack = null;
        }
        int channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        if (channelCount == 2) {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        }
        try {
            int audioBufSize = AudioTrack.getMinBufferSize(sampleRateInHz,
                    channelConfig,
                    AudioFormat.ENCODING_PCM_16BIT);

            audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRateInHz,
                    channelConfig,
                    AudioFormat.ENCODING_PCM_16BIT,
                    audioBufSize,
                    AudioTrack.MODE_STREAM);
            audioTrack.play();
            audioTrack.setStereoVolume(volume, volume);
        } catch (Exception e) {
            BZLogUtil.e(TAG, e);
        }
    }

    public void setVideoPlayerVolume(float volume) {
        BZLogUtil.d(TAG, "setVideoPlayerVolume volume=" + volume + " --" + this);
        this.volume = volume;
        try {
            if (null != audioTrack)
                audioTrack.setStereoVolume(volume, volume);
        } catch (Throwable e) {
            BZLogUtil.e(TAG, e);
        }
    }

    /**
     * call from jni
     */
    public void onPCMDataAvailable(byte[] pcmData, int length) {
//        BZLogUtil.d(TAG, "onPCMDataAvailable length=" + length);

        try {
            if (null != audioTrack) {
                audioTrack.write(pcmData, 0, length);
            }
        } catch (Throwable e) {
            BZLogUtil.e(TAG, e);
        }
    }

    public void pause() {
        try {
            if (null != audioTrack)
                audioTrack.pause();
        } catch (Throwable e) {
            BZLogUtil.e(TAG, e);
        }
    }

    public void start() {
        try {
            if (null != audioTrack)
                audioTrack.play();
        } catch (Throwable e) {
            BZLogUtil.e(TAG, e);
        }
    }

    public void stopAudioTrack() {
        BZLogUtil.d(TAG, "stopAudioTrack=" + this);
        if (null != audioTrack) {
            try {
                audioTrack.flush();
                audioTrack.pause();
                audioTrack.stop();
                audioTrack.release();
            } catch (Throwable e) {
                BZLogUtil.e(TAG, e);
            }
        }
        audioTrack = null;
    }
}
