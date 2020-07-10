package com.luoye.bzmedia.bean;

import com.luoye.bzmedia.BZMedia;

/**
 * Created by luoye on 2017/6/18.
 * 录制参数封装
 */
public class VideoRecordParams {
    /**
     * outputPath      视频输出路径
     * videoWidth      视频原始的宽,注意是否是倒置的
     * videoHeight     视频原始的高
     * targetWidth      最终视频输出的宽
     * targetHeight     最终视频输出的高
     * videoFrameRate        视频帧率
     * bitRate          视频码率
     * nbSamples        每一个音频帧有多少采样
     * sampleRate       音频的采样率
     * videoRotate      视频旋转角度
     * extraFilterParam 额外的滤镜参数,保留参数,可以传ffmpeg支持的参数
     * pixelFormat      录制的像素格式
     * hasAudio         是否有音频
     * recordSpeed      视频录制速度
     */
    private String outputPath;
    private int videoWidth;
    private int videoHeight;
    private int targetWidth;
    private int targetHeight;
    private int videoFrameRate = 30;
    private int nbSamples;
    private int sampleRate;
    private int videoRotate;
    private int pixelFormat=BZMedia.PixelFormat.YUVI420.ordinal();
    private boolean hasAudio = true;
    private boolean needFlipVertical = false;
    //Are the videos all key frames?
    private boolean allFrameIsKey = false;
    private long bitRate;

    private float recordSpeed = 1.0f;

    //Is the video synchronously encoded?
    private boolean synEncode = false;

    private boolean avPacketFromMediaCodec = false;

    public String getOutputPath() {
        return outputPath;
    }

    public void setOutputPath(String outputPath) {
        this.outputPath = outputPath;
    }

    public int getVideoWidth() {
        return videoWidth;
    }

    public void setVideoWidth(int videoWidth) {
        this.videoWidth = videoWidth;
    }

    public int getVideoHeight() {
        return videoHeight;
    }

    public void setVideoHeight(int videoHeight) {
        this.videoHeight = videoHeight;
    }

    public int getTargetWidth() {
        return targetWidth;
    }

    public void setTargetWidth(int targetWidth) {
        this.targetWidth = targetWidth;
    }

    public int getTargetHeight() {
        return targetHeight;
    }

    public void setTargetHeight(int targetHeight) {
        this.targetHeight = targetHeight;
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

    public boolean isHasAudio() {
        return hasAudio;
    }

    public void setHasAudio(boolean hasAudio) {
        this.hasAudio = hasAudio;
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
                ", videoWidth=" + videoWidth +
                ", videoHeight=" + videoHeight +
                ", targetWidth=" + targetWidth +
                ", targetHeight=" + targetHeight +
                ", videoFrameRate=" + videoFrameRate +
                ", nbSamples=" + nbSamples +
                ", sampleRate=" + sampleRate +
                ", videoRotate=" + videoRotate +
                ", pixelFormat=" + pixelFormat +
                ", hasAudio=" + hasAudio +
                ", needFlipVertical=" + needFlipVertical +
                ", allFrameIsKey=" + allFrameIsKey +
                ", bitRate=" + bitRate +
                ", recordSpeed=" + recordSpeed +
                ", synEncode=" + synEncode +
                ", avPacketFromMediaCodec=" + avPacketFromMediaCodec +
                '}';
    }
}
