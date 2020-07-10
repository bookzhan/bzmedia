package bz.luoye.sample.utils;

import android.os.Environment;

/**
 * Created by zhandalin on 2020-07-10 22:37.
 * description:
 */
public class FilePathUtil {
    public static String getWorkDir() {
        return Environment.getExternalStorageDirectory().getAbsolutePath() + "/bzmedia";
    }

    public static String getAVideoPath() {
        return getWorkDir() + "/VID_" + System.currentTimeMillis() + ".mp4";
    }
}
