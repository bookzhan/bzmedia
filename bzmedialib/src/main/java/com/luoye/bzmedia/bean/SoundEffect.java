package com.luoye.bzmedia.bean;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Created by bookzhan on 2022-04-03 13:34.
 * description:
 */
public class SoundEffect implements Parcelable {
    private String name;
    private float tempo = 1;
    private float pitch = 1;
    private float speed = 1;
    private long startTime;
    private long endTime;

    public SoundEffect() {
    }

    public SoundEffect(float pitch) {
        this.pitch = pitch;
    }

    public SoundEffect(String name, float tempo, float pitch, float speed) {
        this.name = name;
        this.tempo = tempo;
        this.pitch = pitch;
        this.speed = speed;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public float getTempo() {
        return tempo;
    }

    public void setTempo(float tempo) {
        this.tempo = tempo;
    }

    public float getPitch() {
        return pitch;
    }

    /**
     * @param pitch (0,8)
     */
    public void setPitch(float pitch) {
        this.pitch = pitch;
    }

    public float getSpeed() {
        return speed;
    }

    /**
     * @param speed (0,100)
     */
    public void setSpeed(float speed) {
        this.speed = speed;
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

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.name);
        dest.writeFloat(this.tempo);
        dest.writeFloat(this.pitch);
        dest.writeFloat(this.speed);
        dest.writeLong(this.startTime);
        dest.writeLong(this.endTime);
    }

    public void readFromParcel(Parcel source) {
        this.name = source.readString();
        this.tempo = source.readFloat();
        this.pitch = source.readFloat();
        this.speed = source.readFloat();
        this.startTime = source.readLong();
        this.endTime = source.readLong();
    }

    protected SoundEffect(Parcel in) {
        this.name = in.readString();
        this.tempo = in.readFloat();
        this.pitch = in.readFloat();
        this.speed = in.readFloat();
        this.startTime = in.readLong();
        this.endTime = in.readLong();
    }

    public static final Creator<SoundEffect> CREATOR = new Creator<SoundEffect>() {
        @Override
        public SoundEffect createFromParcel(Parcel source) {
            return new SoundEffect(source);
        }

        @Override
        public SoundEffect[] newArray(int size) {
            return new SoundEffect[size];
        }
    };
}
