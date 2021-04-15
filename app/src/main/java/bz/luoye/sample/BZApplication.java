package bz.luoye.sample;

import android.os.StrictMode;

import androidx.multidex.MultiDexApplication;

import com.luoye.bzmedia.BZMedia;
import com.nbb.effect.opengl.EffectEngine;

/**
 * Created by bookzhan on 2020-05-25 16:34.
 * description:
 */
public class BZApplication extends MultiDexApplication {
    @Override
    public void onCreate() {
        super.onCreate();
        BZMedia.init(getApplicationContext(), BuildConfig.DEBUG);
        EffectEngine.init(getApplicationContext());

        if(com.luoye.bzmedia.BuildConfig.DEBUG) {
            // https://medium.com/android-news/android-dev-tool-best-practice-with-strictmode-a023e09030a5
            StrictMode.enableDefaults();
        }
    }
}
