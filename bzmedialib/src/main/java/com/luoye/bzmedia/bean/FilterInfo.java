package com.luoye.bzmedia.bean;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.Objects;

/**
 * Created by bookzhan on 2021-03-28 18:03.
 * description:
 */
public class FilterInfo implements Parcelable {
    private String filterName = null;
    private String filterPath = null;
    private boolean updateFilter = true;
    /**
     * 0~1
     */
    private float filterIntensity = 1;

    public FilterInfo(String filterPath) {
        this.filterPath = filterPath;
    }

    public FilterInfo(String filterPath, float filterIntensity) {
        this.filterPath = filterPath;
        this.filterIntensity = filterIntensity;
    }

    public String getFilterName() {
        return filterName;
    }

    public void setFilterName(String filterName) {
        this.filterName = filterName;
    }

    public String getFilterPath() {
        return filterPath;
    }

    public void setFilterPath(String filterPath) {
        this.filterPath = filterPath;
    }

    public float getFilterIntensity() {
        return filterIntensity;
    }

    public void setFilterIntensity(float filterIntensity) {
        this.filterIntensity = filterIntensity;
    }

    public boolean isUpdateFilter() {
        return updateFilter;
    }

    public void setUpdateFilter(boolean updateFilter) {
        this.updateFilter = updateFilter;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        FilterInfo that = (FilterInfo) o;
        return Float.compare(that.filterIntensity, filterIntensity) == 0 &&
                Objects.equals(filterName, that.filterName) &&
                Objects.equals(filterPath, that.filterPath);
    }

    @Override
    public int hashCode() {
        return Objects.hash(filterName, filterPath, filterIntensity);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.filterName);
        dest.writeString(this.filterPath);
        dest.writeByte(this.updateFilter ? (byte) 1 : (byte) 0);
        dest.writeFloat(this.filterIntensity);
    }

    public void readFromParcel(Parcel source) {
        this.filterName = source.readString();
        this.filterPath = source.readString();
        this.updateFilter = source.readByte() != 0;
        this.filterIntensity = source.readFloat();
    }

    protected FilterInfo(Parcel in) {
        this.filterName = in.readString();
        this.filterPath = in.readString();
        this.updateFilter = in.readByte() != 0;
        this.filterIntensity = in.readFloat();
    }

    public static final Creator<FilterInfo> CREATOR = new Creator<FilterInfo>() {
        @Override
        public FilterInfo createFromParcel(Parcel source) {
            return new FilterInfo(source);
        }

        @Override
        public FilterInfo[] newArray(int size) {
            return new FilterInfo[size];
        }
    };
}
