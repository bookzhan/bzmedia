package com.luoye.bzmedia.utils;

import android.content.Context;
import android.hardware.SensorManager;
import android.view.OrientationEventListener;

import com.bzcommon.utils.BZLogUtil;

/**
 * Created by bookzhan on 2022-03-13 15:06.
 * description:
 */
public class ScreenOrientationHandler {
    private final static String TAG = "bz_OrientationListener";
    private final OrientationEventListener mOrientationListener;
    private OrientationListener orientationListener;

    public ScreenOrientationHandler(Context context, OrientationListener orientationListener) {
        this.orientationListener = orientationListener;
        mOrientationListener = new OrientationEventListener(context,
                SensorManager.SENSOR_DELAY_NORMAL) {
            @Override
            public void onOrientationChanged(int orientation) {
                parseOrientation(orientation);
            }
        };
    }

    private void parseOrientation(int orientation) {
        orientation = orientation % 360;
        BZLogUtil.d(TAG, "Orientation changed to " + orientation);
        int temp_0 = orientation / 90;
        int temp_1 = orientation % 90;
        if (temp_1 > 45) {
            temp_0 += 1;
        }
        int finalOrientation = temp_0 * 90 % 360;
        if (null != orientationListener) {
            orientationListener.onOrientationChange(finalOrientation);
        }
    }

    public void enable() {
        mOrientationListener.enable();
    }

    public void disable() {
        mOrientationListener.disable();
    }

    public interface OrientationListener {
        void onOrientationChange(int orientation );
    }
}
