package com.luoye.bzmedia.widget;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import androidx.core.graphics.ColorUtils;

import com.bzcommon.utils.BZDensityUtil;
import com.luoye.bzmedia.bean.PaintInfo;
import com.luoye.bzmedia.bean.PaintingData;
import com.luoye.bzmedia.bean.PaintingMode;
import com.luoye.bzmedia.utils.ParcelableUtil;

import java.util.Deque;
import java.util.concurrent.ConcurrentLinkedDeque;


public class PaintingView extends View {
    protected final Paint paint;
    protected float preX, preY;
    protected final Deque<PaintingData> paintingDataDeque = new ConcurrentLinkedDeque<>();
    protected final Deque<PaintingData> lastPaintingDataDeque = new ConcurrentLinkedDeque<>();

    protected BZPath drawPath = new BZPath();
    protected PaintInfo paintInfo = new PaintInfo(PaintingMode.LINE_MODE);
    protected boolean enableTouch = true;
    protected long currentTime = -1;
    protected OnFinishPaintingListener onFinishPaintingListener;
    private OnTouchListener onTouchListener;
    private long currentDuration;


    public PaintingView(Context context) {
        this(context, null);
    }

    public PaintingView(Context context, AttributeSet attrs) {
        super(context, attrs);
        paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setStyle(Paint.Style.STROKE);
        float strokeWidth = BZDensityUtil.dip2px(context, 4);
        paintInfo.setPaintWidth(strokeWidth);
        paintInfo.setColor(Color.RED);
        setLayerType(LAYER_TYPE_HARDWARE, null);
    }

    public void setEnableTouch(boolean enableTouch) {
        this.enableTouch = enableTouch;
    }

    public boolean isEnableTouch() {
        return enableTouch;
    }

    public void setBackground(int color, int transparent) {
        int chCl = ColorUtils.setAlphaComponent(color, transparent);
        setBackgroundColor(chCl);
        invalidate();
    }

    public void removePaintingData(PaintingData paintingData) {
        if (null == paintingData) {
            return;
        }
        paintingDataDeque.remove(paintingData);
        postInvalidate();
    }

    public void addPaintingData(PaintingData paintingData) {
        if (null == paintingData) {
            return;
        }
        paintingDataDeque.add(paintingData);
        postInvalidate();
    }

    public long getCurrentTime() {
        return currentTime >= 0 ? currentTime : System.currentTimeMillis();
    }

    public void setCurrentTime(long currentTime) {
        this.currentTime = currentTime;
    }

    public void setCurrentDuration(long currentDuration) {
        this.currentDuration = currentDuration;
    }

    public void setPaintingMode(PaintingMode paintingMode) {
        paintInfo.setPaintingMode(paintingMode);
    }

    public void setColor(int color, int transparent) {
        int chCl = ColorUtils.setAlphaComponent(color, transparent);
        paintInfo.setColor(chCl);
        invalidate();
    }

    public void setPaintWidth(float paintWidth) {
        paintInfo.setPaintWidth(paintWidth);
        invalidate();
    }

    public void eraserIt() {
        paintInfo.setColor(Color.WHITE);
        paintInfo.setPaintingMode(PaintingMode.ERASER_MODE);
        invalidate();
    }

    public void back() {
        PaintingData paintingData = paintingDataDeque.pollLast();
        if (null != paintingData) {
            lastPaintingDataDeque.addLast(paintingData);
        }
        invalidate();
    }

    public Deque<PaintingData> getPaintingDataDeque() {
        return paintingDataDeque;
    }

    public void next() {
        PaintingData paintingData = lastPaintingDataDeque.pollLast();
        if (null != paintingData) {
            paintingDataDeque.addLast(paintingData);
        }
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        canvas.drawColor(Color.TRANSPARENT);
        for (PaintingData paintingData : paintingDataDeque) {
            drawImplement(canvas, paintingData, getCurrentTime());
        }
        setMyPaint(paintInfo, paint);
        canvas.drawPath(drawPath, paint);
    }

    public void drawImplement(Canvas canvas, PaintingData paintingData, long currentTime) {
        setMyPaint(paintingData.getPaintInfo(), paint);
        BZPath bzPath = paintingData.getPath();
        canvas.drawPath(bzPath.getTargetPath(currentTime, paintingData.getDuration()), paint);
    }

    protected static void setMyPaint(PaintInfo paintInfo, Paint paint) {
        switch (paintInfo.getPaintingMode()) {
            case LINE_MODE:
                paint.setXfermode(null);
                paint.setColor(paintInfo.getColor());
                paint.setStrokeWidth(paintInfo.getPaintWidth());
                paint.setStrokeCap(Paint.Cap.BUTT);
                paint.setShader(null);
                paint.setMaskFilter(null);
                break;
            case PIC_MODE:
                paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
                paint.setColor(paintInfo.getColor());
                paint.setStrokeWidth(paintInfo.getPaintWidth());
                paint.setStrokeCap(Paint.Cap.BUTT);
                paint.setShader(null);
                paint.setMaskFilter(null);
                break;
            case ERASER_MODE:
                paint.setColor(paintInfo.getColor());
                paint.setStrokeWidth(paintInfo.getPaintWidth());
                paint.setStrokeCap(Paint.Cap.BUTT);
                paint.setShader(null);
                paint.setMaskFilter(null);
                break;
        }
    }


    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (!enableTouch) {
            return super.onTouchEvent(event);
        }
        float x = event.getX();
        float y = event.getY();
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                drawPath.reset();
                drawPath.moveTo(x, y, getCurrentTime());
                preX = x;
                preY = y;
                break;
            case MotionEvent.ACTION_MOVE:
                drawPath.quadTo(preX, preY, (x + preX) / 2, (y + preY) / 2, getCurrentTime());
                preX = x;
                preY = y;
                break;
            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_UP:
                drawPath.lineTo(x, y, getCurrentTime());
                PaintingData paintingData = new PaintingData(drawPath, paintInfo, currentDuration);
                paintingDataDeque.addLast(paintingData);
                if (null != onFinishPaintingListener) {
                    onFinishPaintingListener.onFinishPainting(paintingData);
                }
                drawPath = new BZPath();
                paintInfo = (PaintInfo) ParcelableUtil.copy(paintInfo);
                break;
        }
        invalidate();
        if (null != onTouchListener) {
            onTouchListener.onTouch(this, event);
        }
        return true;
    }

    public void setOnFinishPaintingListener(OnFinishPaintingListener onFinishPaintingListener) {
        this.onFinishPaintingListener = onFinishPaintingListener;
    }

    public void setInnerOnTouchListener(OnTouchListener onTouchListener) {
        this.onTouchListener = onTouchListener;
    }

    public interface OnFinishPaintingListener {
        void onFinishPainting(PaintingData paintingData);
    }
}
