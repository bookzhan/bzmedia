package com.luoye.bzmedia.utils;

import com.bzcommon.glutils.FrameBufferUtil;

/**
 * Created by bookzhan on 2022-03-06 09:42.
 * description:
 */
public class DoubleFrameBufferUtil {
    protected FrameBufferUtil frameBufferUtil_1 = null;
    protected FrameBufferUtil frameBufferUtil_2 = null;
    private int frameBufferIndex = 0;
    private int width;
    private int height;

    public FrameBufferUtil getNextFrameBuffer(int width, int height) {
        if (this.width != width || this.height != height) {
            release();
        }
        this.width = width;
        this.height = height;
        //Double buffer exchange
        if (null == frameBufferUtil_1) {
            frameBufferUtil_1 = new FrameBufferUtil(width, height);
        }
        if (null == frameBufferUtil_2) {
            frameBufferUtil_2 = new FrameBufferUtil(width, height);
        }
        frameBufferIndex++;
        return frameBufferIndex % 2 == 1 ? frameBufferUtil_1 : frameBufferUtil_2;
    }

    public void release() {
        if (null != frameBufferUtil_1) {
            frameBufferUtil_1.release();
            frameBufferUtil_1 = null;
        }
        if (null != frameBufferUtil_2) {
            frameBufferUtil_2.release();
            frameBufferUtil_2 = null;
        }
    }
}
