package com.luoye.bzmedia.bean;

import com.luoye.bzmedia.widget.BZPath;

/**
 * Created by bookzhan on 2022-02-10 10:49.
 * description:
 */
public class PaintingData {
    private BZPath path;
    private PaintInfo paintInfo;
    private long duration;

    public PaintingData(BZPath path, PaintInfo paintInfo,long duration) {
        this.path = path;
        this.paintInfo = paintInfo;
        this.duration = duration;
    }

    public long getDuration() {
        return duration;
    }

    public void setDuration(long duration) {
        this.duration = duration;
    }

    public BZPath getPath() {
        return path;
    }

    public void setPath(BZPath path) {
        this.path = path;
    }

    public PaintInfo getPaintInfo() {
        return paintInfo;
    }

    public void setPaintInfo(PaintInfo paintInfo) {
        this.paintInfo = paintInfo;
    }
}
