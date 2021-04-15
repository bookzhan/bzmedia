package com.luoye.bzmedia.widget;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

import com.luoye.bzmedia.R;


/**
 * Created by bookzhan on 2021-07-31 09:43.
 * description:
 */
public class BZRectView extends View {

    private final Paint paint;
    private final float dp_1;

    public BZRectView(Context context) {
        this(context, null);
    }

    public BZRectView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public BZRectView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        dp_1 = context.getResources().getDisplayMetrics().density;
        TypedArray array = context.obtainStyledAttributes(attrs, R.styleable.BZRectView);
        int strokeColor = array.getColor(R.styleable.BZRectView_strokeColor, Color.RED);
        float strokeWidth = array.getDimension(R.styleable.BZRectView_strokeWidth, 2 * dp_1);
        array.recycle();
        paint = new Paint();
        paint.setAntiAlias(true);
        paint.setColor(strokeColor);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(strokeWidth);
    }

    public void setStrokeColor(int strokeColor) {
        paint.setColor(strokeColor);
    }

    public void setStrokeWidth(int strokeWidth) {
        paint.setStrokeWidth(strokeWidth);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int width = getWidth();
        int height = getHeight();
        if (width < 0 || height < 0) {
            return;
        }
        canvas.drawRect(dp_1, dp_1, width - dp_1, height - dp_1, paint);
    }
}
