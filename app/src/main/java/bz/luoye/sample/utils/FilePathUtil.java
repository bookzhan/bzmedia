package bz.luoye.sample.utils;

import android.content.Context;
import android.os.Environment;

/**
 * Created by bookzhan on 2020-07-10 22:37.
 * description:
 */
public class FilePathUtil {
    public static String getReadWorkDir() {
        return Environment.getExternalStorageDirectory().getAbsolutePath() + "/bzmedia";
    }

    public static String getWritePath(Context context) {
        return context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS).getAbsolutePath();
    }

    public static String getAVideoPath(Context context) {
        return context.getExternalFilesDir(Environment.DIRECTORY_MOVIES).getAbsolutePath() + "/VID_" + System.currentTimeMillis() + ".mp4";
    }

    public static String getAAudioPath(Context context) {
        return context.getExternalFilesDir(Environment.DIRECTORY_MUSIC).getAbsolutePath() + "/audio_" + System.currentTimeMillis() + ".m4a";
    }
}
