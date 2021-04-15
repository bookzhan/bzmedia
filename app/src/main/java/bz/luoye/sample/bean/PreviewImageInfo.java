package bz.luoye.sample.bean;

import android.graphics.Bitmap;

/**
 * Created by bookzhan on 2021-08-22 15:22.
 * description:
 */
public class PreviewImageInfo {
    private Bitmap bitmap;
    private int index = 0;
    private boolean isGapIndex = false;
    private float startRatio = 0;

    public PreviewImageInfo() {
    }

    public PreviewImageInfo(Bitmap bitmap, int index, boolean isGapIndex, float startRatio) {
        this.bitmap = bitmap;
        this.index = index;
        this.isGapIndex = isGapIndex;
        this.startRatio = startRatio;
    }

    public Bitmap getBitmap() {
        return bitmap;
    }

    public void setBitmap(Bitmap bitmap) {
        this.bitmap = bitmap;
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public boolean isGapIndex() {
        return isGapIndex;
    }

    public void setGapIndex(boolean gapIndex) {
        isGapIndex = gapIndex;
    }

    public float getStartRatio() {
        return startRatio;
    }

    public void setStartRatio(float startRatio) {
        this.startRatio = startRatio;
    }
}
