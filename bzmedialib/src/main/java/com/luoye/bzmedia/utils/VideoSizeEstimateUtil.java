package com.luoye.bzmedia.utils;

/**
 * Created by bookzhan on 2021-09-04 22:44.
 * description:
 */
public class VideoSizeEstimateUtil {
    public static float getVideoSizeBySize(int videoWidth, int videoHeight, long videoDuration, long audioDuration) {
        long bitRate = VideoUtil.getDefaultBitRate(videoWidth, videoHeight);
        return getVideoSizeByBitrate(bitRate, videoDuration, 128000, audioDuration);
    }

    /**
     * @param videoDuration ms
     * @param audioDuration ms
     * @return m
     */
    public static float getVideoSizeByBitrate(long videoBitrate, long videoDuration, long audioBitrate, long audioDuration) {
        float result = videoBitrate / 1024f * videoDuration / 1000f + audioBitrate / 1024f * audioDuration / 1000f;
        return result / 8f / 1024f;
    }
}
