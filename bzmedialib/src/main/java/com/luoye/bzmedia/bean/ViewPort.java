package com.luoye.bzmedia.bean;

/**
 * Created by bookzhan on 2017-10-09 17:28.
 * Description:
 */
public class ViewPort {
    public int x, y;
    public int width, height;

    public ViewPort() {
    }

    public ViewPort(int _x, int _y, int _width, int _height) {
        x = _x;
        y = _y;
        width = _width;
        height = _height;
    }

    @Override
    public String toString() {
        return "ViewPort{" +
                "x=" + x +
                ", y=" + y +
                ", width=" + width +
                ", height=" + height +
                '}';
    }
}
