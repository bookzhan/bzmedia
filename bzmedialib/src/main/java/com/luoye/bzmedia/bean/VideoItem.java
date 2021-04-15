package com.luoye.bzmedia.bean;

import android.graphics.RectF;
import android.os.Parcel;
import android.os.Parcelable;

import java.util.Objects;

/**
 * Created by bookzhan on 2021-04-12 09:22.
 * description:the record and save parameters of the video
 */
public class VideoItem implements Parcelable {
    private String videoPath;
    /**
     * ms
     */
    private long startTime = -1;
    /**
     * ms
     */
    private long endTime = -1;
    /**
     * Multiple of volume
     */
    private float volume = 1;

    private FilterInfo filterInfo = null;
    private AdjustEffectConfig adjustEffectConfig = null;

    /**
     * Multiples of video playback speed
     */
    private float speed = 1;

    private int rotate = 0;
    /**
     * Whether to support horizontal flip
     */
    private boolean flipHorizontal = false;

    /**
     * The values of cropRect are all relative to the percentage of width and height, and they are all relative to the original video frame.
     */
    private RectF cropRect = null;

    private long startTimeBoundary = -1;
    private long endTimeBoundary = -1;

    public VideoItem(String videoPath, long startTime, long endTime) {
        this.videoPath = videoPath;
        this.startTime = startTime;
        this.endTime = endTime;
        this.startTimeBoundary = startTime;
        this.endTimeBoundary = endTime;
    }

    public String getVideoPath() {
        return videoPath;
    }

    public void setVideoPath(String videoPath) {
        this.videoPath = videoPath;
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

    public float getVolume() {
        return volume;
    }

    public void setVolume(float volume) {
        this.volume = volume;
    }

    public FilterInfo getFilterInfo() {
        return filterInfo;
    }

    public void setFilterInfo(FilterInfo filterInfo) {
        this.filterInfo = filterInfo;
    }

    public float getSpeed() {
        return speed;
    }

    public void setSpeed(float speed) {
        this.speed = speed;
    }

    public int getRotate() {
        return rotate;
    }

    public void setRotate(int rotate) {
        this.rotate = rotate;
    }

    public boolean isFlipHorizontal() {
        return flipHorizontal;
    }

    public void setFlipHorizontal(boolean flipHorizontal) {
        this.flipHorizontal = flipHorizontal;
    }

    public RectF getCropRect() {
        return cropRect;
    }

    public void setCropRect(RectF cropRect) {
        this.cropRect = cropRect;
    }

    public AdjustEffectConfig getAdjustEffectConfig() {
        return adjustEffectConfig;
    }

    public void setAdjustEffectConfig(AdjustEffectConfig adjustEffectConfig) {
        this.adjustEffectConfig = adjustEffectConfig;
    }

    public long getStartTimeBoundary() {
        return startTimeBoundary;
    }

    public void setStartTimeBoundary(long startTimeBoundary) {
        this.startTimeBoundary = startTimeBoundary;
    }

    public long getDuration() {
        return endTime - startTime;
    }

    public long getEndTimeBoundary() {
        return endTimeBoundary;
    }

    public void setEndTimeBoundary(long endTimeBoundary) {
        this.endTimeBoundary = endTimeBoundary;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        VideoItem videoItem = (VideoItem) o;
        return startTime == videoItem.startTime &&
                endTime == videoItem.endTime &&
                Float.compare(videoItem.volume, volume) == 0 &&
                Float.compare(videoItem.speed, speed) == 0 &&
                rotate == videoItem.rotate &&
                flipHorizontal == videoItem.flipHorizontal &&
                startTimeBoundary == videoItem.startTimeBoundary &&
                endTimeBoundary == videoItem.endTimeBoundary &&
                Objects.equals(videoPath, videoItem.videoPath) &&
                Objects.equals(filterInfo, videoItem.filterInfo) &&
                Objects.equals(adjustEffectConfig, videoItem.adjustEffectConfig) &&
                Objects.equals(cropRect, videoItem.cropRect);
    }

    @Override
    public int hashCode() {
        return Objects.hash(videoPath, startTime, endTime, volume, filterInfo, adjustEffectConfig, speed, rotate, flipHorizontal, cropRect, startTimeBoundary, endTimeBoundary);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.videoPath);
        dest.writeLong(this.startTime);
        dest.writeLong(this.endTime);
        dest.writeFloat(this.volume);
        dest.writeParcelable(this.filterInfo, flags);
        dest.writeParcelable(this.adjustEffectConfig, flags);
        dest.writeFloat(this.speed);
        dest.writeInt(this.rotate);
        dest.writeByte(this.flipHorizontal ? (byte) 1 : (byte) 0);
        dest.writeParcelable(this.cropRect, flags);
        dest.writeLong(this.startTimeBoundary);
        dest.writeLong(this.endTimeBoundary);
    }

    public void readFromParcel(Parcel source) {
        this.videoPath = source.readString();
        this.startTime = source.readLong();
        this.endTime = source.readLong();
        this.volume = source.readFloat();
        this.filterInfo = source.readParcelable(FilterInfo.class.getClassLoader());
        this.adjustEffectConfig = source.readParcelable(AdjustEffectConfig.class.getClassLoader());
        this.speed = source.readFloat();
        this.rotate = source.readInt();
        this.flipHorizontal = source.readByte() != 0;
        this.cropRect = source.readParcelable(RectF.class.getClassLoader());
        this.startTimeBoundary = source.readLong();
        this.endTimeBoundary = source.readLong();
    }

    protected VideoItem(Parcel in) {
        this.videoPath = in.readString();
        this.startTime = in.readLong();
        this.endTime = in.readLong();
        this.volume = in.readFloat();
        this.filterInfo = in.readParcelable(FilterInfo.class.getClassLoader());
        this.adjustEffectConfig = in.readParcelable(AdjustEffectConfig.class.getClassLoader());
        this.speed = in.readFloat();
        this.rotate = in.readInt();
        this.flipHorizontal = in.readByte() != 0;
        this.cropRect = in.readParcelable(RectF.class.getClassLoader());
        this.startTimeBoundary = in.readLong();
        this.endTimeBoundary = in.readLong();
    }

    public static final Creator<VideoItem> CREATOR = new Creator<VideoItem>() {
        @Override
        public VideoItem createFromParcel(Parcel source) {
            return new VideoItem(source);
        }

        @Override
        public VideoItem[] newArray(int size) {
            return new VideoItem[size];
        }
    };
}
