package com.luoye.bzmedia.bean;

/**
 * Created by bookzhan on 2018-01-12 10:01.
 * Description:
 */

public class VideoTransCodeParams {
    private final String inputPath;
    private final String outputPath;
    private int gopSize = 30;

    private long startTime = -1;
    private long endTime = -1;

    private int targetWidth = -2;
    private int targetHeight = -2;

    /**
     * Frame rate control
     */
    private int frameRate = 30;

    public VideoTransCodeParams(String inputPath, String outputPath) {
        this.inputPath = inputPath;
        this.outputPath = outputPath;
    }

    public String getInputPath() {
        return inputPath;
    }

    public String getOutputPath() {
        return outputPath;
    }

    public int getGopSize() {
        return gopSize;
    }

    public void setGopSize(int gopSize) {
        this.gopSize = gopSize;
    }

    public long getStartTime() {
        return startTime;
    }

    public void setStartTime(long startTime) {
        this.startTime = startTime;
    }

    public long getEndTime() {
        return endTime;
    }

    public void setEndTime(long endTime) {
        this.endTime = endTime;
    }

    public int getTargetWidth() {
        return targetWidth;
    }

    public void setTargetWidth(int targetWidth) {
        this.targetWidth = targetWidth / 2 * 2;
    }

    public int getTargetHeight() {
        return targetHeight;
    }

    public void setTargetHeight(int targetHeight) {
        this.targetHeight = targetHeight / 2 * 2;
    }

    public int getFrameRate() {
        return frameRate;
    }

    public void setFrameRate(int frameRate) {
        this.frameRate = frameRate;
    }
}
