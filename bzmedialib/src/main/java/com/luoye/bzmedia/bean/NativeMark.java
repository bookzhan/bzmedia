package com.luoye.bzmedia.bean;

import android.view.View;

/**
 * Created by bookzhan on 2022-04-04 09:28.
 * description:
 */
public class NativeMark extends ViewMark{
    private long nativeHandle = 0;
    private long tempNativeHandle = 0;

    public NativeMark(long startTime, long duration, View view) {
        super(startTime, duration, view);
    }

    public long getNativeHandle() {
        return nativeHandle;
    }

    public void setNativeHandle(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    public long getTempNativeHandle() {
        return tempNativeHandle;
    }

    public void setTempNativeHandle(long tempNativeHandle) {
        this.tempNativeHandle = tempNativeHandle;
    }
}
