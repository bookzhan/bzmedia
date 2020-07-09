package bz.luoye.bzmedia;

import android.content.Context;
import android.os.Build;

/**
 * Created by zhandalin on 2020-05-25 16:27.
 * description:
 */
public class BZMedia {
    static {
        System.loadLibrary("bzffmpeg");
        System.loadLibrary("bzmedia");
    }

    public static int init(Context context, boolean isDebug) {
        return initNative(context, isDebug, Build.VERSION.SDK_INT);
    }

    private native static int initNative(Context context, boolean isDebug, int sdkInt);

    public native static int test();
}
