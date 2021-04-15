package com.luoye.bzmedia.bean;

import android.view.View;

/**
 * Created by bookzhan on 2022-04-07 18:29.
 * description:Record the movement, rotation and scaling process of view
 */
public class ViewStateInfo {
    private long time;
    private float translationX;
    private float translationY;
    private float rotation;
    private float scaleX;
    private float scaleY;
    private float pivotX;
    private float pivotY;

    public ViewStateInfo(View view,long time) {
        this.translationX = view.getTranslationX();
        this.translationY = view.getTranslationY();
        this.rotation = view.getRotation();
        this.scaleX = view.getScaleX();
        this.scaleY = view.getScaleY();
        this.pivotX = view.getPivotX();
        this.pivotY = view.getPivotY();
        this.time = time;
    }

    public long getTime() {
        return time;
    }

    public void setTime(long time) {
        this.time = time;
    }

    public float getTranslationX() {
        return translationX;
    }

    public void setTranslationX(float translationX) {
        this.translationX = translationX;
    }

    public float getTranslationY() {
        return translationY;
    }

    public void setTranslationY(float translationY) {
        this.translationY = translationY;
    }

    public float getRotation() {
        return rotation;
    }

    public void setRotation(float rotation) {
        this.rotation = rotation;
    }

    public float getScaleX() {
        return scaleX;
    }

    public void setScaleX(float scaleX) {
        this.scaleX = scaleX;
    }

    public float getScaleY() {
        return scaleY;
    }

    public void setScaleY(float scaleY) {
        this.scaleY = scaleY;
    }

    public float getPivotX() {
        return pivotX;
    }

    public void setPivotX(float pivotX) {
        this.pivotX = pivotX;
    }

    public float getPivotY() {
        return pivotY;
    }

    public void setPivotY(float pivotY) {
        this.pivotY = pivotY;
    }
}
