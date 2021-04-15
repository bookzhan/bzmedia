package com.luoye.bzmedia.bean;

import android.graphics.Canvas;
import android.view.View;

import com.luoye.bzmedia.opengl.MosaicProgram;

/**
 * Created by bookzhan on 2022-03-07 21:55.
 * description:
 */
public class MosaicMark extends NativeMark {
    private float centerX = 0;
    private float centerY = 0;
    private float mosaicRadius = 0;
    private int rectangleWidth = 1;
    private int rectangleHeight = 1;
    private MosaicProgram.MosaicType mMosaicType = MosaicProgram.MosaicType.CIRCLE;

    public MosaicMark(long startTime, long duration, View view) {
        super(startTime, duration,view);
    }

    public MosaicProgram.MosaicType getMosaicType() {
        return mMosaicType;
    }

    public void setMosaicType(MosaicProgram.MosaicType mosaicType) {
        mMosaicType = mosaicType;
    }

    public float getCenterX() {
        return centerX;
    }

    public void setCenterX(float centerX) {
        this.centerX = centerX;
    }

    public float getCenterY() {
        return centerY;
    }

    public void setCenterY(float centerY) {
        this.centerY = centerY;
    }

    public float getMosaicRadius() {
        return mosaicRadius;
    }

    public void setMosaicRadius(float mosaicRadius) {
        this.mosaicRadius = mosaicRadius;
    }

    public int getRectangleWidth() {
        return rectangleWidth;
    }

    public void setRectangleWidth(int rectangleWidth) {
        this.rectangleWidth = rectangleWidth;
    }

    public int getRectangleHeight() {
        return rectangleHeight;
    }

    public void setRectangleHeight(int rectangleHeight) {
        this.rectangleHeight = rectangleHeight;
    }

    @Override
    public void onDraw(Canvas canvas, long currentTime) {
        super.onDraw(canvas, currentTime);
    }
}
