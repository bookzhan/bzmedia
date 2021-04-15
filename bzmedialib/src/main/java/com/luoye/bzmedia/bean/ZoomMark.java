package com.luoye.bzmedia.bean;

import android.view.View;

/**
 * Created by bookzhan on 2022-04-04 09:31.
 * description:
 */
public class ZoomMark extends NativeMark {
    private float touchX = 0;
    private float touchY = 0;
    private float radius = 100;
    private float zoomRatio = 2;

    public ZoomMark(long startTime, long duration, View view) {
        super(startTime, duration,view);
    }

    public float getTouchX() {
        return touchX;
    }

    public void setTouchX(float touchX) {
        this.touchX = touchX;
    }

    public float getTouchY() {
        return touchY;
    }

    public void setTouchY(float touchY) {
        this.touchY = touchY;
    }

    public float getRadius() {
        return radius;
    }

    public void setRadius(float radius) {
        this.radius = radius;
    }

    public float getZoomRatio() {
        return zoomRatio;
    }

    public void setZoomRatio(float zoomRatio) {
        this.zoomRatio = zoomRatio;
    }
}
