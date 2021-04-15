package com.luoye.bzmedia.recorder;

/**
 * Created by bookzhan on 2018-06-25 10:29.
 * 说明:
 */
public interface OnRecordPCMListener {
    byte[] onRecordPCM(byte[] pcmData);
}
