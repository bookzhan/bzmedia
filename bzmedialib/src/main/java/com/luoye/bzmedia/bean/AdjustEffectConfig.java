package com.luoye.bzmedia.bean;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.Objects;

/**
 * Created by bookzhan on 2021-03-11 15:16.
 * description:
 */
public class AdjustEffectConfig implements Parcelable{

    private float shadows = 1;
    private float highlights = 1;
    private float contrast = 1;
    private float fadeAmount = 0;
    private float saturation = 1;
    private float shadowsTintIntensity = 0;
    private float highlightsTintIntensity = 0.5f;
    private BZColor shadowsTintColor = new BZColor(0, 0, 0, 1);
    private BZColor highlightsTintColor = new BZColor(0, 0, 0, 1);
    private float exposure = 0;
    private float warmth = 0;
    private float green = 1;
    private float hueAdjust = 0;
    private float vignette = 0;
    private boolean needUpdateConfig = true;

    public float getShadows() {
        return shadows;
    }

    /**
     * @param shadows 0.45~1
     */
    public void setShadows(float shadows) {
        this.shadows = shadows;
    }

    public float getHighlights() {
        return highlights;
    }

    /**
     * @param highlights 0.25~1.75
     */
    public void setHighlights(float highlights) {
        this.highlights = highlights;
    }

    public float getContrast() {
        return contrast;
    }

    /**
     * @param contrast 0.7~1.3
     */
    public void setContrast(float contrast) {
        this.contrast = contrast;
    }

    public float getFadeAmount() {
        return fadeAmount;
    }

    /**
     * @param fadeAmount 0~1
     */
    public void setFadeAmount(float fadeAmount) {
        this.fadeAmount = fadeAmount;
    }


    public float getSaturation() {
        return saturation;
    }

    /**
     * @param saturation 0~2.05
     */
    public void setSaturation(float saturation) {
        this.saturation = saturation;
    }

    public float getShadowsTintIntensity() {
        return shadowsTintIntensity;
    }

    /**
     * @param shadowsTintIntensity 0~1
     */
    public void setShadowsTintIntensity(float shadowsTintIntensity) {
        this.shadowsTintIntensity = shadowsTintIntensity;
    }

    public float getHighlightsTintIntensity() {
        return highlightsTintIntensity;
    }

    /**
     * @param highlightsTintIntensity 0.5~1
     */
    public void setHighlightsTintIntensity(float highlightsTintIntensity) {
        this.highlightsTintIntensity = highlightsTintIntensity;
    }

    public BZColor getShadowsTintColor() {

        return shadowsTintColor;
    }

    /**
     * @param shadowsTintColor r,g,b 0~1
     */
    public void setShadowsTintColor(BZColor shadowsTintColor) {
        if (null == shadowsTintColor) {
            return;
        }
        this.shadowsTintColor = shadowsTintColor;
    }

    public BZColor getHighlightsTintColor() {
        return highlightsTintColor;
    }

    /**
     * @param highlightsTintColor r,g,b 0~1
     */
    public void setHighlightsTintColor(BZColor highlightsTintColor) {
        if (null == highlightsTintColor) {
            return;
        }
        this.highlightsTintColor = highlightsTintColor;
    }

    public float getExposure() {
        return exposure;
    }

    /**
     * @param exposure -1~1
     */
    public void setExposure(float exposure) {
        this.exposure = exposure;
    }

    public float getWarmth() {
        return warmth;
    }

    /**
     * @param warmth -1~1
     */
    public void setWarmth(float warmth) {
        this.warmth = warmth;
    }

    public float getGreen() {
        return green;
    }

    /**
     * @param green 0.75~1.25
     */
    public void setGreen(float green) {
        this.green = green;
    }

    public float getHueAdjust() {
        return hueAdjust;
    }

    /**
     * @param hueAdjust -0.174~0.174
     */
    public void setHueAdjust(float hueAdjust) {
        this.hueAdjust = hueAdjust;
    }

    public float getVignette() {
        return vignette;
    }

    /**
     * @param vignette 0~1
     */
    public void setVignette(float vignette) {
        this.vignette = vignette;
    }

    public boolean isNeedUpdateConfig() {
        return needUpdateConfig;
    }

    public void setNeedUpdateConfig(boolean needUpdateConfig) {
        this.needUpdateConfig = needUpdateConfig;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        AdjustEffectConfig that = (AdjustEffectConfig) o;
        return Float.compare(that.shadows, shadows) == 0 &&
                Float.compare(that.highlights, highlights) == 0 &&
                Float.compare(that.contrast, contrast) == 0 &&
                Float.compare(that.fadeAmount, fadeAmount) == 0 &&
                Float.compare(that.saturation, saturation) == 0 &&
                Float.compare(that.shadowsTintIntensity, shadowsTintIntensity) == 0 &&
                Float.compare(that.highlightsTintIntensity, highlightsTintIntensity) == 0 &&
                Float.compare(that.exposure, exposure) == 0 &&
                Float.compare(that.warmth, warmth) == 0 &&
                Float.compare(that.green, green) == 0 &&
                Float.compare(that.hueAdjust, hueAdjust) == 0 &&
                Float.compare(that.vignette, vignette) == 0 &&
                Objects.equals(shadowsTintColor, that.shadowsTintColor) &&
                Objects.equals(highlightsTintColor, that.highlightsTintColor);
    }

    @Override
    public int hashCode() {
        return Objects.hash(shadows, highlights, contrast, fadeAmount, saturation, shadowsTintIntensity, highlightsTintIntensity, shadowsTintColor, highlightsTintColor, exposure, warmth, green, hueAdjust, vignette);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeFloat(this.shadows);
        dest.writeFloat(this.highlights);
        dest.writeFloat(this.contrast);
        dest.writeFloat(this.fadeAmount);
        dest.writeFloat(this.saturation);
        dest.writeFloat(this.shadowsTintIntensity);
        dest.writeFloat(this.highlightsTintIntensity);
        dest.writeParcelable(this.shadowsTintColor, flags);
        dest.writeParcelable(this.highlightsTintColor, flags);
        dest.writeFloat(this.exposure);
        dest.writeFloat(this.warmth);
        dest.writeFloat(this.green);
        dest.writeFloat(this.hueAdjust);
        dest.writeFloat(this.vignette);
    }

    public void readFromParcel(Parcel source) {
        this.shadows = source.readFloat();
        this.highlights = source.readFloat();
        this.contrast = source.readFloat();
        this.fadeAmount = source.readFloat();
        this.saturation = source.readFloat();
        this.shadowsTintIntensity = source.readFloat();
        this.highlightsTintIntensity = source.readFloat();
        this.shadowsTintColor = source.readParcelable(BZColor.class.getClassLoader());
        this.highlightsTintColor = source.readParcelable(BZColor.class.getClassLoader());
        this.exposure = source.readFloat();
        this.warmth = source.readFloat();
        this.green = source.readFloat();
        this.hueAdjust = source.readFloat();
        this.vignette = source.readFloat();
    }

    public AdjustEffectConfig() {
    }

    protected AdjustEffectConfig(Parcel in) {
        this.shadows = in.readFloat();
        this.highlights = in.readFloat();
        this.contrast = in.readFloat();
        this.fadeAmount = in.readFloat();
        this.saturation = in.readFloat();
        this.shadowsTintIntensity = in.readFloat();
        this.highlightsTintIntensity = in.readFloat();
        this.shadowsTintColor = in.readParcelable(BZColor.class.getClassLoader());
        this.highlightsTintColor = in.readParcelable(BZColor.class.getClassLoader());
        this.exposure = in.readFloat();
        this.warmth = in.readFloat();
        this.green = in.readFloat();
        this.hueAdjust = in.readFloat();
        this.vignette = in.readFloat();
    }

    public static final Creator<AdjustEffectConfig> CREATOR = new Creator<AdjustEffectConfig>() {
        @Override
        public AdjustEffectConfig createFromParcel(Parcel source) {
            return new AdjustEffectConfig(source);
        }

        @Override
        public AdjustEffectConfig[] newArray(int size) {
            return new AdjustEffectConfig[size];
        }
    };
}
