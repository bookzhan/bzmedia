package com.luoye.bzmedia.utils;

/**
 * Created by zhandalin on 2017-05-25 16:20.
 * 说明:
 */
public class FFmpegCMDUtil {

    public synchronized static native int showLog(boolean showLog);

    public synchronized static native int executeFFmpegCommand(String command, OnActionListener onActionListener);

    public interface OnActionListener {
        void progress(float progress);

        void fail();

        void success();
    }
}
