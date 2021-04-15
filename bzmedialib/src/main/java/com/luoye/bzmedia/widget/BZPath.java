package com.luoye.bzmedia.widget;

import android.graphics.Path;
import android.graphics.PointF;

import com.luoye.bzmedia.bean.BZPathInfo;
import com.luoye.bzmedia.bean.BZPathOperateType;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;


/**
 * Created by bookzhan on 2022-04-07 14:41.
 * description:
 */
public class BZPath extends Path {
    private final List<BZPathInfo> mPathInfoList = new CopyOnWriteArrayList<>();
    private final Path mPath = new Path();
    private final PointF mPoint = new PointF();


    @Override
    public void reset() {
        super.reset();
        mPathInfoList.clear();
    }

    @Override
    public void moveTo(float x, float y) {
        super.moveTo(x, y);
        mPathInfoList.add(new BZPathInfo(x, y, BZPathOperateType.MOVE_TO));
    }

    public void moveTo(float x, float y, long time) {
        super.moveTo(x, y);
        mPathInfoList.add(new BZPathInfo(x, y, BZPathOperateType.MOVE_TO, time));
    }

    @Override
    public void quadTo(float x1, float y1, float x2, float y2) {
        super.quadTo(x1, y1, x2, y2);
        BZPathInfo bzPathInfo = new BZPathInfo(x2, y2, BZPathOperateType.QUAD_TO);
        bzPathInfo.setLastX(x1);
        bzPathInfo.setLastY(y1);
        mPathInfoList.add(bzPathInfo);
    }

    public void quadTo(float x1, float y1, float x2, float y2, long time) {
        super.quadTo(x1, y1, x2, y2);
        BZPathInfo bzPathInfo = new BZPathInfo(x2, y2, BZPathOperateType.QUAD_TO, time);
        bzPathInfo.setLastX(x1);
        bzPathInfo.setLastY(y1);
        mPathInfoList.add(bzPathInfo);
    }

    @Override
    public void lineTo(float x, float y) {
        super.lineTo(x, y);
        mPathInfoList.add(new BZPathInfo(x, y, BZPathOperateType.LINE_TO));
    }

    public void lineTo(float x, float y, long time) {
        super.lineTo(x, y);
        mPathInfoList.add(new BZPathInfo(x, y, BZPathOperateType.LINE_TO, time));
    }

    public PointF getPoint(long time) {
        mPoint.x = 0;
        mPoint.y = 0;
        for (BZPathInfo bzPathInfo : mPathInfoList) {
            if (bzPathInfo.getTime() > time) {
                break;
            }
            mPoint.x = bzPathInfo.getX();
            mPoint.y = bzPathInfo.getY();
        }
        return mPoint;
    }

    public Path getTargetPath(long time, long duration) {
        mPath.reset();
        if (!mPathInfoList.isEmpty()) {
            BZPathInfo bzPathInfo = mPathInfoList.get(mPathInfoList.size() - 1);
            if (time > bzPathInfo.getTime() + duration) {
                return mPath;
            }
        }
        for (BZPathInfo bzPathInfo : mPathInfoList) {
            if (bzPathInfo.getTime() > time) {
                break;
            }
            switch (bzPathInfo.getBZPathOperateType()) {
                case MOVE_TO:
                    mPath.moveTo(bzPathInfo.getX(), bzPathInfo.getY());
                    break;
                case QUAD_TO:
                    mPath.quadTo(bzPathInfo.getLastX(), bzPathInfo.getLastY(), bzPathInfo.getX(), bzPathInfo.getY());
                    break;
                case LINE_TO:
                    mPath.lineTo(bzPathInfo.getX(), bzPathInfo.getY());
                    break;
            }
        }
        return mPath;
    }

    public void setPathInfoList(List<BZPathInfo> pathInfoList) {
        if (null == pathInfoList) {
            return;
        }
        mPathInfoList.clear();
        mPathInfoList.addAll(pathInfoList);
    }

    public List<BZPathInfo> getPathInfoList() {
        return mPathInfoList;
    }
}
