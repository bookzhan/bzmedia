package com.luoye.bzmedia.bean;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.Objects;

/**
 * Created by bookzhan on 2018-11-16 15:40.
 * Description: Decide how the texture should be processed, fill type, fill background, zoom information
 */
public class TextureHandleInfo implements Parcelable {
    private int bgFillType = 0;
    private BZColor bgColor = null;
    private int scaleType = 0;
    private float gaussBlurRadius = 2;

    private String bgTexturePath=null;
    private int bgTextureId = 0;
    private int bgTextureWidth = 0;
    private int bgTextureHeight = 0;
    private float bgTextureIntensity = 0;

    public BgFillType getBgFillType() {
        return BgFillType.values()[bgFillType];
    }

    public void setBgFillType(BgFillType bgFillType) {
        this.bgFillType = bgFillType.ordinal();
    }

    public BZColor getBgColor() {
        return bgColor;
    }

    public void setBgColor(BZColor bgColor) {
        this.bgColor = bgColor;
    }

    public BZScaleType getScaleType() {
        return BZScaleType.values()[scaleType];
    }

    public void setScaleType(BZScaleType scaleType) {
        this.scaleType = scaleType.ordinal();
    }

    public int getBgTextureId() {
        return bgTextureId;
    }

    public void setBgTextureId(int bgTextureId) {
        this.bgTextureId = bgTextureId;
    }

    public float getGaussBlurRadius() {
        return gaussBlurRadius;
    }

    /**
     * @param gaussBlurRadius 0~10
     */
    public void setGaussBlurRadius(float gaussBlurRadius) {
        this.gaussBlurRadius = gaussBlurRadius;
    }

    public float getBgTextureIntensity() {
        return bgTextureIntensity;
    }

    /**
     * @param bgTextureIntensity 0~10
     */
    public void setBgTextureIntensity(float bgTextureIntensity) {
        this.bgTextureIntensity = bgTextureIntensity;
    }

    public int getBgTextureWidth() {
        return bgTextureWidth;
    }

    public void setBgTextureWidth(int bgTextureWidth) {
        this.bgTextureWidth = bgTextureWidth;
    }

    public int getBgTextureHeight() {
        return bgTextureHeight;
    }

    public void setBgTextureHeight(int bgTextureHeight) {
        this.bgTextureHeight = bgTextureHeight;
    }

    public String getBgTexturePath() {
        return bgTexturePath;
    }

    public void setBgTexturePath(String bgTexturePath) {
        this.bgTexturePath = bgTexturePath;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        TextureHandleInfo that = (TextureHandleInfo) o;
        return bgFillType == that.bgFillType &&
                scaleType == that.scaleType &&
                Float.compare(that.gaussBlurRadius, gaussBlurRadius) == 0 &&
                bgTextureId == that.bgTextureId &&
                bgTextureWidth == that.bgTextureWidth &&
                bgTextureHeight == that.bgTextureHeight &&
                Float.compare(that.bgTextureIntensity, bgTextureIntensity) == 0 &&
                Objects.equals(bgColor, that.bgColor) &&
                Objects.equals(bgTexturePath, that.bgTexturePath);
    }

    @Override
    public int hashCode() {
        return Objects.hash(bgFillType, bgColor, scaleType, gaussBlurRadius, bgTexturePath, bgTextureId, bgTextureWidth, bgTextureHeight, bgTextureIntensity);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.bgFillType);
        dest.writeParcelable(this.bgColor, flags);
        dest.writeInt(this.scaleType);
        dest.writeFloat(this.gaussBlurRadius);
        dest.writeString(this.bgTexturePath);
        dest.writeInt(this.bgTextureId);
        dest.writeInt(this.bgTextureWidth);
        dest.writeInt(this.bgTextureHeight);
        dest.writeFloat(this.bgTextureIntensity);
    }

    public void readFromParcel(Parcel source) {
        this.bgFillType = source.readInt();
        this.bgColor = source.readParcelable(BZColor.class.getClassLoader());
        this.scaleType = source.readInt();
        this.gaussBlurRadius = source.readFloat();
        this.bgTexturePath = source.readString();
        this.bgTextureId = source.readInt();
        this.bgTextureWidth = source.readInt();
        this.bgTextureHeight = source.readInt();
        this.bgTextureIntensity = source.readFloat();
    }

    public TextureHandleInfo() {
    }

    protected TextureHandleInfo(Parcel in) {
        this.bgFillType = in.readInt();
        this.bgColor = in.readParcelable(BZColor.class.getClassLoader());
        this.scaleType = in.readInt();
        this.gaussBlurRadius = in.readFloat();
        this.bgTexturePath = in.readString();
        this.bgTextureId = in.readInt();
        this.bgTextureWidth = in.readInt();
        this.bgTextureHeight = in.readInt();
        this.bgTextureIntensity = in.readFloat();
    }

    public static final Creator<TextureHandleInfo> CREATOR = new Creator<TextureHandleInfo>() {
        @Override
        public TextureHandleInfo createFromParcel(Parcel source) {
            return new TextureHandleInfo(source);
        }

        @Override
        public TextureHandleInfo[] newArray(int size) {
            return new TextureHandleInfo[size];
        }
    };
}
