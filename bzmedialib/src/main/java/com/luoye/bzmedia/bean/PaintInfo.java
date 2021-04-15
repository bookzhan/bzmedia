package com.luoye.bzmedia.bean;

import android.os.Parcel;
import android.os.Parcelable;

public class PaintInfo implements Parcelable {

    private int color;
    private float paintWidth;
    private PaintingMode paintingMode;

    public PaintInfo(PaintingMode paintingMode) {
        this.paintingMode = paintingMode;
    }

    public int getColor() {
        return color;
    }

    public void setColor(int color) {
        this.color = color;
    }

    public Float getPaintWidth() {
        return paintWidth;
    }

    public void setPaintWidth(Float paintWidth) {
        this.paintWidth = paintWidth;
    }

    public void setPaintWidth(float paintWidth) {
        this.paintWidth = paintWidth;
    }

    public PaintingMode getPaintingMode() {
        return paintingMode;
    }

    public void setPaintingMode(PaintingMode paintingMode) {
        this.paintingMode = paintingMode;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.color);
        dest.writeFloat(this.paintWidth);
        dest.writeInt(this.paintingMode == null ? -1 : this.paintingMode.ordinal());
    }

    public void readFromParcel(Parcel source) {
        this.color = source.readInt();
        this.paintWidth = source.readFloat();
        int tmpPaintingMode = source.readInt();
        this.paintingMode = tmpPaintingMode == -1 ? null : PaintingMode.values()[tmpPaintingMode];
    }

    protected PaintInfo(Parcel in) {
        this.color = in.readInt();
        this.paintWidth = in.readFloat();
        int tmpPaintingMode = in.readInt();
        this.paintingMode = tmpPaintingMode == -1 ? null : PaintingMode.values()[tmpPaintingMode];
    }

    public static final Parcelable.Creator<PaintInfo> CREATOR = new Parcelable.Creator<PaintInfo>() {
        @Override
        public PaintInfo createFromParcel(Parcel source) {
            return new PaintInfo(source);
        }

        @Override
        public PaintInfo[] newArray(int size) {
            return new PaintInfo[size];
        }
    };
}
