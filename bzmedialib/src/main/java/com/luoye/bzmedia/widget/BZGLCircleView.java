package com.luoye.bzmedia.widget;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

/**
 * Created by bookzhan on 2021-07-21 11:04.
 * description:
 */
public class BZGLCircleView extends View implements GLDraw {

    private final Paint paint;
    private final float dp_1;

    public BZGLCircleView(Context context) {
        this(context, null);
    }

    public BZGLCircleView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public BZGLCircleView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        paint = new Paint();
        paint.setAntiAlias(true);
        paint.setColor(Color.RED);
        paint.setStyle(Paint.Style.STROKE);
        dp_1 = context.getResources().getDisplayMetrics().density;
        float strokeWidth = 2 * dp_1;
        paint.setStrokeWidth(strokeWidth);
    }

    public void setCircleColor(int color) {
        paint.setColor(color);
    }

    @Override
    protected void onDraw(Canvas canvas) {

    }

    @SuppressLint("WrongCall")
    @Override
    public void onGLDraw(Canvas canvas, long currentTime) {
        super.onDraw(canvas);
        int width = getWidth();
        int height = getHeight();
        if (width < 0 || height < 0) {
            return;
        }
        int radius = getRadius();
        canvas.drawCircle(width / 2f, height / 2f, radius, paint);
    }

    public int getRadius() {
        return (int) (Math.min(getWidth(), getHeight()) / 2 - 2 * dp_1);
    }

    public Paint gePaint() {
        return paint;
    }

    @Override
    public boolean performClick() {
        return super.performClick();
    }
}
