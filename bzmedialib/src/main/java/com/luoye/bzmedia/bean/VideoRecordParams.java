package com.luoye.bzmedia.bean;

import com.luoye.bzmedia.BZMedia;

/**
 * Created by luoye on 2017/6/18.
 * 录制参数封装
 */
public class VideoRecordParams {
    /**
     * outputPath      Video output path
     * inputWidth      The width of the video input, pay attention to whether it is inverted
     * inputHeight     Video input high
     * targetWidth      The width of the final video output
     * targetHeight     The high of the final video output
     * videoFrameRate        Video frame rate
     * bitRate          Video bitrate
     * nbSamples        How many samples are in each audio frame
     * sampleRate       Audio sampling rate
     * videoRotate      Video rotation angle
     * pixelFormat      Recorded pixel format
     * hasAudio         Whether there is audio
     * recordSpeed      Video recording speed
     */
    private String outputPath;
    private int inputWidth;
    private int inputHeight;
    private int targetWidth;
    private int targetHeight;
    private int videoFrameRate = 30;
    private int nbSamples;
    private int sampleRate;
    private int videoRotate;
    private int pixelFormat = BZMedia.PixelFormat.YUVI420.ordinal();
    private boolean needAudio = true;

    /**
     * Only useful for texture
     */
    private boolean needFlipVertical = false;
    //Are the videos all key frames?
    private boolean allFrameIsKey = false;
    private long bitRate;

    private float recordSpeed = 1.0f;

    //Is the video synchronously encoded?
    private boolean synEncode = true;

    private boolean avPacketFromMediaCodec = false;

    public String getOutputPath() {
        return outputPath;
    }

    public void setOutputPath(String outputPath) {
        this.outputPath = outputPath;
    }

    public int getInputWidth() {
        return inputWidth;
    }

    public void setInputWidth(int inputWidth) {
        this.inputWidth = inputWidth;
    }

    public int getInputHeight() {
        return inputHeight;
    }

    public void setInputHeight(int inputHeight) {
        this.inputHeight = inputHeight;
    }

    public int getTargetWidth() {
        return targetWidth;
    }

    public void setTargetWidth(int targetWidth) {
        //align to 8
        this.targetWidth = targetWidth / 8 * 8;
    }

    public int getTargetHeight() {
        return targetHeight;
    }

    public void setTargetHeight(int targetHeight) {
        //align to 16
        this.targetHeight = targetHeight / 8 * 8;
    }

    public int getVideoFrameRate() {
        return videoFrameRate;
    }

    public void setVideoFrameRate(int videoFrameRate) {
        this.videoFrameRate = videoFrameRate;
    }

    public int getNbSamples() {
        return nbSamples;
    }

    public void setNbSamples(int nbSamples) {
        this.nbSamples = nbSamples;
    }

    public int getSampleRate() {
        return sampleRate;
    }

    public void setSampleRate(int sampleRate) {
        this.sampleRate = sampleRate;
    }

    public int getVideoRotate() {
        return videoRotate;
    }

    public void setVideoRotate(int videoRotate) {
        this.videoRotate = videoRotate;
    }

    public BZMedia.PixelFormat getPixelFormat() {
        return BZMedia.PixelFormat.values()[pixelFormat];
    }

    public void setPixelFormat(BZMedia.PixelFormat pixelFormat) {
        this.pixelFormat = pixelFormat.ordinal();
    }

    public boolean isNeedAudio() {
        return needAudio;
    }

    public void setNeedAudio(boolean needAudio) {
        this.needAudio = needAudio;
    }

    public boolean isNeedFlipVertical() {
        return needFlipVertical;
    }

    public void setNeedFlipVertical(boolean needFlipVertical) {
        this.needFlipVertical = needFlipVertical;
    }

    public boolean isAllFrameIsKey() {
        return allFrameIsKey;
    }

    public void setAllFrameIsKey(boolean allFrameIsKey) {
        this.allFrameIsKey = allFrameIsKey;
    }

    public long getBitRate() {
        return bitRate;
    }

    public void setBitRate(long bitRate) {
        this.bitRate = bitRate;
    }

    public boolean isSynEncode() {
        return synEncode;
    }

    public void setSynEncode(boolean synEncode) {
        this.synEncode = synEncode;
    }

    public boolean isAvPacketFromMediaCodec() {
        return avPacketFromMediaCodec;
    }

    public void setAvPacketFromMediaCodec(boolean avPacketFromMediaCodec) {
        this.avPacketFromMediaCodec = avPacketFromMediaCodec;
    }

    public float getRecordSpeed() {
        return recordSpeed;
    }

    public void setRecordSpeed(float recordSpeed) {
        this.recordSpeed = recordSpeed;
    }

    @Override
    public String toString() {
        return "VideoRecordParams{" +
                "outputPath='" + outputPath + '\'' +
                ", inputWidth=" + inputWidth +
                ", inputHeight=" + inputHeight +
                ", targetWidth=" + targetWidth +
                ", targetHeight=" + targetHeight +
                ", videoFrameRate=" + videoFrameRate +
                ", nbSamples=" + nbSamples +
                ", sampleRate=" + sampleRate +
                ", videoRotate=" + videoRotate +
                ", pixelFormat=" + pixelFormat +
                ", hasAudio=" + needAudio +
                ", needFlipVertical=" + needFlipVertical +
                ", allFrameIsKey=" + allFrameIsKey +
                ", bitRate=" + bitRate +
                ", recordSpeed=" + recordSpeed +
                ", synEncode=" + synEncode +
                ", avPacketFromMediaCodec=" + avPacketFromMediaCodec +
                '}';
    }
}
