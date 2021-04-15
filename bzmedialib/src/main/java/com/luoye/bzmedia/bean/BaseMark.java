package com.luoye.bzmedia.bean;

import android.graphics.Canvas;

/**
 * Created by bookzhan on 2022-03-07 20:34.
 * description:
 */
public class BaseMark {
    private long startTime;
    private long duration;

    public BaseMark(long startTime, long duration) {
        this.startTime = startTime;
        this.duration = duration;
    }

    public long getStartTime() {
        return startTime;
    }

    public void setStartTime(long startTime) {
        this.startTime = startTime;
    }

    public long getDuration() {
        return duration;
    }

    public void setDuration(long duration) {
        this.duration = duration;
    }

    public void onDraw(Canvas canvas, long currentTime) {

    }


}
