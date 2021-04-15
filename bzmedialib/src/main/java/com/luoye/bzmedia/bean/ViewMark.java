package com.luoye.bzmedia.bean;

import android.graphics.Canvas;
import android.view.View;

import com.luoye.bzmedia.widget.GLDraw;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Created by bookzhan on 2022-03-07 20:42.
 * description:
 */
public class ViewMark extends BaseMark {
    private transient final View view;
    private final List<ViewStateInfo> viewStateInfoList = new CopyOnWriteArrayList<>();
    private boolean isTouching = false;

    public ViewMark(long startTime, long duration, View view) {
        super(startTime, duration);
        this.view = view;
    }

    public View getView() {
        return view;
    }

    public void recordViewState(long time) {
        viewStateInfoList.add(new ViewStateInfo(view, time));
    }

    public void recoverViewState(long time) {
        if (null == view) {
            return;
        }
        ViewStateInfo lastInfo = null;
        for (ViewStateInfo viewStateInfo : viewStateInfoList) {
            if (viewStateInfo.getTime() >= time) {
                setViewStateInfo(viewStateInfo);
                lastInfo = null;
                break;
            }
            lastInfo = viewStateInfo;
        }
        ViewStateInfo finalLastInfo = lastInfo;
        view.post(() -> setViewStateInfo(finalLastInfo));
    }

    @Override
    public long getDuration() {
        return isTouching ? Integer.MAX_VALUE : super.getDuration();
    }

    public boolean isTouching() {
        return isTouching;
    }

    public void setTouching(boolean touching) {
        isTouching = touching;
    }

    private void setViewStateInfo(ViewStateInfo viewStateInfo) {
        if (null == viewStateInfo || null == view) {
            return;
        }
        view.setPivotX(viewStateInfo.getPivotX());
        view.setPivotY(viewStateInfo.getPivotY());
        view.setTranslationX(viewStateInfo.getTranslationX());
        view.setTranslationY(viewStateInfo.getTranslationY());
        view.setScaleX(viewStateInfo.getScaleX());
        view.setScaleY(viewStateInfo.getScaleY());
        view.setRotation(viewStateInfo.getRotation());
    }

    public List<ViewStateInfo> getViewStateInfoList() {
        return viewStateInfoList;
    }

    @Override
    public void onDraw(Canvas canvas, long currentTime) {
        super.onDraw(canvas, currentTime);
        if (null == view || null == canvas) {
            return;
        }
        boolean visible = getStartTime() + getDuration() > currentTime && getStartTime() <= currentTime;
        if (getDuration() < 0) {
            visible = true;
        }
        if (!visible) {
            return;
        }
        canvas.save();
        canvas.translate(view.getX(), view.getY());
        canvas.rotate(view.getRotation(), view.getPivotX(), view.getPivotY());
        canvas.scale(view.getScaleX(), view.getScaleY(), view.getPivotX(), view.getPivotY());
        if (view instanceof GLDraw) {
            GLDraw glDraw = (GLDraw) view;
            glDraw.onGLDraw(canvas, currentTime);
        } else {
            view.draw(canvas);
        }
        canvas.restore();
    }

}
