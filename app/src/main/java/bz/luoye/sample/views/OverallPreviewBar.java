package bz.luoye.sample.views;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.bzcommon.utils.BZDensityUtil;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoItem;

import java.util.ArrayList;
import java.util.List;

import bz.luoye.sample.bean.PreviewImageInfo;

/**
 * Created by bookzhan on 2021-08-22 14:54.
 * description:
 */
public class OverallPreviewBar extends View {
    private static final String TAG = "bz_OverallPreviewBar";
    private static final int MSG_UPDATE_OVERALL_PREVIEW = 0x13;
    private final List<VideoItem> videoItemList = new ArrayList<>();
    private final List<PreviewImageInfo> previewImageInfoList = new ArrayList<>();
    private HandlerThread handlerThread = null;
    private Handler handler = null;
    private int totalImageCount = 6;
    private final Rect rectFSrc = new Rect();
    private final RectF rectFDst = new RectF();
    private Paint paint;
    private float gapWidth = 4;
    private int screenWidth;
    private float progress = 0;

    public OverallPreviewBar(Context context) {
        this(context, null);
    }

    public OverallPreviewBar(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public OverallPreviewBar(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        handlerThread = new HandlerThread("OverallPreviewBar");
        handlerThread.start();
        handler = new Handler(handlerThread.getLooper(), new Handler.Callback() {
            @Override
            public boolean handleMessage(@NonNull Message msg) {
                if (msg.what == MSG_UPDATE_OVERALL_PREVIEW) {
                    updateOverallPreview();
                }
                return true;
            }
        });
        paint = new Paint();
        paint.setAntiAlias(true);
        gapWidth = BZDensityUtil.dip2px(context, gapWidth);
        screenWidth = BZDensityUtil.getScreenWidth(context);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int viewWidth = getWidth();
        int viewHeight = getHeight();
        if (viewWidth <= 0 || viewHeight <= 0) {
            return;
        }
        if (previewImageInfoList.isEmpty()) {
            return;
        }
        float imageWidth = viewWidth * 1.0f / totalImageCount;
        float imageHeight = viewHeight;
        for (PreviewImageInfo previewImageInfo : previewImageInfoList) {
            Bitmap bitmap = previewImageInfo.getBitmap();
            if (null == bitmap) {
                BZLogUtil.e(TAG, "null==previewImageInfo.getBitmap()");
                continue;
            }
            float startPosition = viewWidth * previewImageInfo.getStartRatio();
            rectFDst.set(startPosition + previewImageInfo.getIndex() * imageWidth, 0, startPosition + (previewImageInfo.getIndex() + 1) * imageWidth, viewHeight);
            float scaleX = bitmap.getWidth() / imageWidth;
            float scaleY = bitmap.getHeight() / imageHeight;
            int scaleWidth;
            int scaleHeight;
            if (scaleX < scaleY) {
                scaleWidth = bitmap.getWidth();
                scaleHeight = (int) (bitmap.getWidth() * imageHeight / imageWidth);
            } else {
                scaleWidth = (int) (bitmap.getHeight() * imageWidth / imageHeight);
                scaleHeight = bitmap.getHeight();
            }
            rectFSrc.set((bitmap.getWidth() - scaleWidth) / 2, (bitmap.getHeight() - scaleHeight) / 2, scaleWidth, scaleHeight);
            canvas.drawBitmap(bitmap, rectFSrc, rectFDst, paint);
        }
        paint.setStyle(Paint.Style.FILL);
        for (PreviewImageInfo previewImageInfo : previewImageInfoList) {
            if (!previewImageInfo.isGapIndex()) {
                continue;
            }
            float startPosition = viewWidth * previewImageInfo.getStartRatio();
            paint.setColor(Color.WHITE);
            float rectSize = imageHeight / 2;
            rectFDst.set(startPosition - rectSize / 2, (imageHeight - rectSize) / 2, startPosition + rectSize / 2, viewHeight - (imageHeight - rectSize) / 2);
            canvas.drawRect(rectFDst, paint);

            paint.setColor(Color.RED);
            rectFDst.set(startPosition - gapWidth / 2, 0, startPosition + gapWidth / 2, viewHeight);
            canvas.drawRect(rectFDst, paint);
        }
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(gapWidth);
        paint.setColor(Color.RED);
        rectFDst.set(gapWidth / 2, gapWidth / 2, viewWidth - gapWidth / 2, viewHeight - gapWidth / 2);
        canvas.drawRect(rectFDst, paint);

        paint.setColor(Color.BLUE);
        paint.setStyle(Paint.Style.FILL);
        rectFDst.set(progress * viewWidth, gapWidth, progress * viewWidth + gapWidth, viewHeight - gapWidth);
        canvas.drawRect(rectFDst, paint);
    }

    public void setVideoItems(List<VideoItem> videoItemList) {
        if (null == videoItemList || videoItemList.isEmpty()) {
            return;
        }
        Bundle bundle = new Bundle();
        bundle.putParcelableArrayList("videoItemList", (ArrayList<VideoItem>) videoItemList);
        if (null != handler) {
            handler.post(() -> {
                OverallPreviewBar.this.videoItemList.clear();
                ArrayList<VideoItem> itemList = bundle.getParcelableArrayList("videoItemList");
                OverallPreviewBar.this.videoItemList.addAll(itemList);
                handler.removeMessages(MSG_UPDATE_OVERALL_PREVIEW);
                handler.sendEmptyMessage(MSG_UPDATE_OVERALL_PREVIEW);
            });
        }
    }

    public void addVideoItem(VideoItem videoItem) {
        if (null == videoItem) {
            return;
        }
        if (null != handler) {
            handler.post(() -> videoItemList.add(videoItem));
            handler.removeMessages(MSG_UPDATE_OVERALL_PREVIEW);
            handler.sendEmptyMessage(MSG_UPDATE_OVERALL_PREVIEW);
        }
    }

    public void setProgress(float progress) {
        this.progress = progress;
        postInvalidate();
    }

    private void updateOverallPreview() {
        if (videoItemList.isEmpty()) {
            return;
        }
        long totalDuration = 0;
        for (VideoItem videoItem : videoItemList) {
            if (null == videoItem.getVideoPath()) {
                continue;
            }
            long duration = videoItem.getDuration();
            if ((videoItem.getEndTime() - videoItem.getStartTime()) > 0) {
                duration = videoItem.getEndTime() - videoItem.getStartTime();
            }
            totalDuration += duration;
        }
        if (totalDuration <= 0) {
            BZLogUtil.e(TAG, "updateOverallPreview totalDuration<=0");
            return;
        }
        post(this::releaseImageInfoList);
        float lastRatio = 0;
        for (VideoItem videoItem : videoItemList) {
            if (null == videoItem.getVideoPath()) {
                continue;
            }
            if (videoItem.getDuration() <= 0) {
                continue;
            }
            long duration = videoItem.getDuration();
            if ((videoItem.getEndTime() - videoItem.getStartTime()) > 0) {
                duration = videoItem.getEndTime() - videoItem.getStartTime();
            }
            float ratio = duration * 1.0f / totalDuration;
            int count = (int) Math.ceil(ratio * totalImageCount);
            float finalLastRatio = lastRatio;
            //You can make a cache here to improve the experience
            BZMedia.getBitmapFromVideo(videoItem.getVideoPath(), count, screenWidth / totalImageCount, videoItem.getStartTime(), videoItem.getEndTime(), new BZMedia.OnGetBitmapFromVideoListener() {
                @Override
                public void onGetBitmapFromVideo(int index, Bitmap bitmap) {
                    post(() -> {
                        previewImageInfoList.add(new PreviewImageInfo(bitmap, index, index == 0 && finalLastRatio > 0, finalLastRatio));
                        postInvalidate();
                    });
                }
            });
            lastRatio += ratio;
        }
    }

    private void releaseImageInfoList() {
        if (null == previewImageInfoList || previewImageInfoList.isEmpty()) {
            return;
        }
        for (PreviewImageInfo previewImageInfo : previewImageInfoList) {
            Bitmap bitmap = previewImageInfo.getBitmap();
            if (null != bitmap && !bitmap.isRecycled()) {
                bitmap.recycle();
            }
            previewImageInfo.setBitmap(null);
        }
        previewImageInfoList.clear();
    }

    public void release() {
        if (null != handler) {
            handler.removeMessages(MSG_UPDATE_OVERALL_PREVIEW);
            handler = null;
        }
        if (null != handlerThread) {
            try {
                handlerThread.quitSafely();
                handlerThread.join();
            } catch (Exception e) {
                BZLogUtil.e(TAG, e);
            }
            handlerThread = null;
        }
        releaseImageInfoList();
        this.videoItemList.clear();
    }

    public int getTotalImageCount() {
        return totalImageCount;
    }

    public void setTotalImageCount(int totalImageCount) {
        this.totalImageCount = totalImageCount;
    }
}
