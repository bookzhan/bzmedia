package com.luoye.bzmedia.recorder;

import android.os.Build;

import com.bzcommon.utils.BZCPUTool;
import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZSpUtils;

import com.luoye.bzmedia.bean.VideoSize;

/**
 * Created by bookzhan on 2017-08-22 13:33.
 * Description:According to the mobile phone CPU and mobile phone version, and the original video width and height, give a suitable video resolution
 */
public class VideoTacticsManager {
    private static final int CPU_DIVIDE_VALUE_FLOOR = 1500;//1.5GHz
    private static final int CPU_DIVIDE_VALUE_CEIL = 2100;//2.1GHz
    private static final String TAG = "bz_VideoSize";

    /**
     * @return false Use media codec video recording
     */
    public static boolean isUseFFmpegRecorder() {
        if (BZSpUtils.getBoolean("use_ffmpeg_new", false)) {
            return true;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            return false;
        }
        return BZCPUTool.getNumberOfCPUCores() >= 8 && BZCPUTool.getMaxCpuFreq() >= CPU_DIVIDE_VALUE_CEIL && BZCPUTool.supported64BitAbi();
    }

    /**
     * @return The maximum resolution is 720p
     */
    private static VideoSize getFFmpegVideoSize(int width, int height) {
        BZLogUtil.d(TAG, "use ffmpeg recorder");
        if (width <= 0 || height <= 0) {
            BZLogUtil.e(TAG, "width <= 0 || height <= 0");
            return new VideoSize(480, 480);
        }
        long maxCpuFreq = BZCPUTool.getMaxCpuFreq();
        int targetWith = 720;
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR2) {
            BZLogUtil.d(TAG, "Versions below android 4.3 The maximum is 480");
            targetWith = 480;//最大为480
            if (maxCpuFreq < CPU_DIVIDE_VALUE_FLOOR) {
                targetWith = 320;
                BZLogUtil.d(TAG, "The cpu is too low and only supports 320");
            }
        } else {
            if (maxCpuFreq < CPU_DIVIDE_VALUE_FLOOR) {
                targetWith = 480;
                BZLogUtil.d(TAG, "The cpu is too low and only supports 480");
            } else {
                BZLogUtil.d(TAG, "maxCpuFreq=" + maxCpuFreq + "--720p");
                targetWith = 720;
            }
        }
        if (targetWith > width) {
            targetWith = width;
            BZLogUtil.d(TAG, "targetWidth >width Take the width value");
        }
        VideoSize videoSize = new VideoSize(targetWith, targetWith * height / width);
        if (videoSize.getVideoHeight() > height) {
            BZLogUtil.d(TAG, "targetHeight >height Take height as the standard, that is, keep the original value unchanged");
            videoSize.setVideoHeight(height);
            videoSize.setVideoWidth(width * height / height);
        }
        return videoSize;
    }


    private static VideoSize getMediaCodeVideoSize(int width, int height) {
        BZLogUtil.d(TAG, "use MediaCode recorder");
        if (width <= 0 || height <= 0) {
            BZLogUtil.e(TAG, "width <= 0 || height <= 0");
            return new VideoSize(480, 480);
        }
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR2) {
            BZLogUtil.e(TAG, "Build.VERSION is " + Build.VERSION.SDK_INT + " can't use MediaCode");
            return new VideoSize(480, 480);
        }
        long maxCpuFreq = BZCPUTool.getMaxCpuFreq();
        int targetWith;
        if (maxCpuFreq < CPU_DIVIDE_VALUE_FLOOR) {
            targetWith = 480;
        } else if (maxCpuFreq >= CPU_DIVIDE_VALUE_CEIL && BZCPUTool.supported64BitAbi() && BZCPUTool.getNumberOfCPUCores() >= 8) {
            targetWith = 1080;
        } else {
            targetWith = 720;
        }

        VideoSize videoSize = new VideoSize(targetWith, targetWith * height / width);
        if (videoSize.getVideoHeight() > height) {
            BZLogUtil.d(TAG, "targetHeight >height Take height as the standard, that is, keep the original value unchanged");
            videoSize.setVideoHeight(height);
            videoSize.setVideoWidth(width * height / height);
        }
        return videoSize;
    }

    /**
     * Android 4.3 is a big distinction
     * The maximum resolution below android 4.3 is 480
     * The maximum resolution above android 4.3 is 1080
     */
    public static VideoSize getFitVideoSize(int width, int height) {
        if (width <= 0 || height <= 0) {
            BZLogUtil.e(TAG, "width <= 0 || height <= 0");
            return new VideoSize(720, 720);
        }
        BZLogUtil.d(TAG, "input width=" + width + "--height=" + height);
        VideoSize videoSize;
        if (isUseFFmpegRecorder()) {
            videoSize = getFFmpegVideoSize(width, height);
        } else {
            videoSize = getMediaCodeVideoSize(width, height);
        }

        BZLogUtil.d(TAG, "final size width=" + videoSize.getVideoWidth() + "--height=" + videoSize.getVideoHeight());
        return videoSize;
    }
}
