package com.luoye.bzmedia.bean;

/**
 * Created by bookzhan on 2022-04-07 14:56.
 * description:
 */
public class BZPathInfo {
    /**
     * current x
     */
    private float x;
    /**
     * current y
     */
    private float y;
    private float lastX;
    private float lastY;
    private BZPathOperateType mBZPathOperateType;
    private long time;

    public BZPathInfo(float x, float y, BZPathOperateType BZPathOperateType) {
        this.x = x;
        this.y = y;
        mBZPathOperateType = BZPathOperateType;
    }

    public BZPathInfo(float x, float y, BZPathOperateType BZPathOperateType, long time) {
        this.x = x;
        this.y = y;
        mBZPathOperateType = BZPathOperateType;
        this.time = time;
    }

    public float getX() {
        return x;
    }

    public void setX(float x) {
        this.x = x;
    }

    public float getY() {
        return y;
    }

    public void setY(float y) {
        this.y = y;
    }

    public float getLastX() {
        return lastX;
    }

    public void setLastX(float lastX) {
        this.lastX = lastX;
    }

    public float getLastY() {
        return lastY;
    }

    public void setLastY(float lastY) {
        this.lastY = lastY;
    }

    public BZPathOperateType getBZPathOperateType() {
        return mBZPathOperateType;
    }

    public void setBZPathOperateType(BZPathOperateType BZPathOperateType) {
        mBZPathOperateType = BZPathOperateType;
    }

    public long getTime() {
        return time;
    }

    public void setTime(long time) {
        this.time = time;
    }
}
