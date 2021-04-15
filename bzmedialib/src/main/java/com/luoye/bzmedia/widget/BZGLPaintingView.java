package com.luoye.bzmedia.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.util.AttributeSet;

import com.luoye.bzmedia.bean.PaintingData;


/**
 * Created by bookzhan on 2022-03-12 20:54.
 * description:
 */
public class BZGLPaintingView extends PaintingView implements GLDraw {
    public BZGLPaintingView(Context context) {
        super(context);
    }

    public BZGLPaintingView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onDraw(Canvas canvas) {
//        canvas.drawColor(Color.TRANSPARENT);
//        setMyPaint(paintInfo, paint);
//        canvas.drawPath(drawPath, paint);
    }

    @Override
    public void onGLDraw(Canvas canvas, long currentTime) {
        for (PaintingData paintingData : paintingDataDeque) {
            drawImplement(canvas, paintingData, currentTime);
        }
        canvas.drawColor(Color.TRANSPARENT);
        setMyPaint(paintInfo, paint);
        canvas.drawPath(drawPath, paint);
    }
}
