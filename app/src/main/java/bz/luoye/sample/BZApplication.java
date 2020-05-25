package bz.luoye.sample;

import android.app.Application;
import android.os.Build;

import bz.luoye.bzmedia.BZMedia;

/**
 * Created by zhandalin on 2020-05-25 16:34.
 * description:
 */
public class BZApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        BZMedia.init(getApplicationContext(), BuildConfig.DEBUG);
    }
}
