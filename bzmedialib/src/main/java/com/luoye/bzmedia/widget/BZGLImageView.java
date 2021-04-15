package com.luoye.bzmedia.widget;


import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.AppCompatImageView;

/**
 * Created by bookzhan on 2022-03-12 19:51.
 * description:
 */
public class BZGLImageView extends AppCompatImageView implements GLDraw {
    public BZGLImageView(@NonNull Context context) {
        super(context);
    }

    public BZGLImageView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public BZGLImageView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void onDraw(Canvas canvas) {

    }

    @SuppressLint("WrongCall")
    @Override
    public void onGLDraw(Canvas canvas,long currentTime) {
        super.onDraw(canvas);
    }
}
