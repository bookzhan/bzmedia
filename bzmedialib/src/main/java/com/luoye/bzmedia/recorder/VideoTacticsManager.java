package com.luoye.bzmedia.recorder;

import android.os.Build;

import com.bzcommon.utils.BZCPUTool;
import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZSpUtils;

import com.luoye.bzmedia.bean.VideoSize;

/**
 * Created by zhandalin on 2017-08-22 13:33.
 * 说明:根据手机CPU与手机版本,以及原始视频宽高给出合适的视频分辨率
 */
public class VideoTacticsManager {
    private static final int CPU_DIVIDE_VALUE_FLOOR = 1500;//1.5GHz
    private static final int CPU_DIVIDE_VALUE_CEIL = 2100;//2.1GHz
    private static final String TAG = "bz_VideoSize";

    /**
     * @return false使用原生视频录制
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
     * @return 最大分辨率为720p
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
            BZLogUtil.d(TAG, "android 4.3以下 版本 最大为480");
            targetWith = 480;//最大为480
            if (maxCpuFreq < CPU_DIVIDE_VALUE_FLOOR) {
                targetWith = 320;
                BZLogUtil.d(TAG, "cpu跟不上只支持320");
            }
        } else {
            if (maxCpuFreq < CPU_DIVIDE_VALUE_FLOOR) {
                targetWith = 480;
                BZLogUtil.d(TAG, "cpu跟不上只支持480");
            } else {
                BZLogUtil.d(TAG, "maxCpuFreq=" + maxCpuFreq + "--720p");
                targetWith = 720;
            }
        }
        if (targetWith > width) {
            targetWith = width;
            BZLogUtil.d(TAG, "targetWidth >width 取width值");
        }
        VideoSize videoSize = new VideoSize();
        videoSize.width = targetWith;
        videoSize.height = targetWith * height / width;

        if (videoSize.height > height) {
            BZLogUtil.d(TAG, "targetHeight >height 以height为准取值 即保持原值不变");
            videoSize.height = height;
            videoSize.width = width * height / height;
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

        VideoSize videoSize = new VideoSize();
        videoSize.width = targetWith;
        videoSize.height = targetWith * height / width;

        if (videoSize.height > height) {
            BZLogUtil.d(TAG, "targetHeight >height 以height为准取值 即保持原值不变");
            videoSize.height = height;
            videoSize.width = width * height / height;
        }
        return videoSize;
    }

    /**
     * android 4.3是一个大的区分点
     * android 4.3以下最大分辨率为480
     * android 4.3以上最大分辨率为1080
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

        BZLogUtil.d(TAG, "final size width=" + videoSize.width + "--height=" + videoSize.height);
        return videoSize;
    }
}
