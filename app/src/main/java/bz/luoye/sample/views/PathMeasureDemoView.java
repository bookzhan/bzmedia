package bz.luoye.sample.views;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PathMeasure;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

/**
 * Created by bookzhan on 2022-04-07 09:59.
 * description:
 */
public class PathMeasureDemoView extends View {

    private Path mPath;
    private Paint mPaint;
    private PathMeasure mPathMeasure;
    private Path mDstPath;
    private int last = 0;


    public PathMeasureDemoView(Context context) {
        this(context, null);
    }

    public PathMeasureDemoView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public PathMeasureDemoView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        mPath = new Path();
        mPath.moveTo(300, 300);
        mPath.lineTo(300, 600);
        mPath.lineTo(600, 600);
        mPath.lineTo(600, 300);
        mPaint = new Paint();
        mPaint.setColor(Color.RED);
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setStrokeWidth(10);
        mPaint.setStrokeCap(Paint.Cap.BUTT);

        mPath.approximate(1);
        mPathMeasure = new PathMeasure(mPath, false);

        mDstPath = new Path();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int step = 20;
        int end = last + step;
        if (end > mPathMeasure.getLength()) {
            last = 0;
            end = step;
        }
        mPathMeasure.getSegment(last, end, mDstPath, true);
        last += step;
        canvas.drawPath(mDstPath, mPaint);
//        canvas.drawPath(mPath, mPaint);
    }
}
