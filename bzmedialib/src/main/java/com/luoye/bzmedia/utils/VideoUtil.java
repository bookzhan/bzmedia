package com.luoye.bzmedia.utils;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoRecordParams;
import com.luoye.bzmedia.recorder.VideoRecorder;
import com.luoye.bzyuvlib.BZYUVUtil;

/**
 * Created by bookzhan on 2021-09-05 11:28.
 * description:
 */
public class VideoUtil {
    private static final String TAG = "bz_VideoUtil";

    /**
     * @return H264 bit rate of 30 frames without B frames
     */
    public static long getDefaultBitRate(int width, int height) {
        float bitratePre = width * height / 1000f;
        long result = (long) (2f * Math.pow(10, -9) * Math.pow(bitratePre, 3) - 0.0001f * Math.pow(bitratePre, 2) + 4.0037f * bitratePre + 1081.8f);
        return result * 1000;
    }

    /**
     * It is only suitable for saving a single picture to the video. Multiple pictures are not suitable for calling this method
     */
    public static int saveABitmap2Video(String videoOutPath, Bitmap bitmap) {
        if (null == videoOutPath || null == bitmap || bitmap.getWidth() <= 0 || bitmap.getHeight() <= 0) {
            BZLogUtil.e(TAG, "null == videoOutPath || null == bitmap || bitmap.getWidth() <= 0 || bitmap.getHeight() <= 0");
            return -1;
        }
        BZLogUtil.d(TAG, "videoOutPath=" + videoOutPath);
        Bitmap bitmapTemp = null;
        final int align = 8;
        if (bitmap.getWidth() % align != 0 || bitmap.getHeight() % align != 0) {
            bitmapTemp = Bitmap.createBitmap(bitmap.getWidth() / align * align, bitmap.getHeight() / align * align, Bitmap.Config.ARGB_8888);
            Canvas canvas = new Canvas(bitmapTemp);
            canvas.drawBitmap(bitmap, -(bitmap.getWidth() - bitmapTemp.getWidth()) / 2f, -(bitmap.getHeight() - bitmapTemp.getHeight()) / 2f, new Paint());
            BZLogUtil.d(TAG, "src width=" + bitmap.getWidth() + " src height=" + bitmap.getHeight() + " rescale width=" + bitmapTemp.getWidth() + " rescale height=" + bitmapTemp.getHeight());
            bitmap = bitmapTemp;
        }
        byte[] yuvBuffer = new byte[bitmap.getWidth() * bitmap.getHeight() * 3 / 2];
        int result = BZYUVUtil.bitmapToYUV420(bitmap, yuvBuffer);
        if (result < 0) {
            BZLogUtil.e(TAG, "bitmapToYUV420 fail");
            return -1;
        }
        VideoRecordParams videoRecordParams = new VideoRecordParams();
        videoRecordParams.setTargetWidth(bitmap.getWidth());
        videoRecordParams.setTargetHeight(bitmap.getHeight());
        videoRecordParams.setVideoRotate(0);
        final int VIDEO_RATE = 30;
        videoRecordParams.setVideoFrameRate(VIDEO_RATE);
        videoRecordParams.setNeedFlipVertical(false);
        videoRecordParams.setPixelFormat(BZMedia.PixelFormat.YUVI420);
        videoRecordParams.setNeedAudio(false);
        videoRecordParams.setAllFrameIsKey(true);
        videoRecordParams.setSynEncode(true);

        videoRecordParams.setOutputPath(videoOutPath);
        VideoRecorder videoRecorder = new VideoRecorder();
        long videoRecorderHandle = videoRecorder.initVideoRecorder();
        videoRecorder.startRecord(videoRecorderHandle, videoRecordParams);
        videoRecorder.addYUV420Data(videoRecorderHandle, yuvBuffer, 0);
        int ret = videoRecorder.stopRecord(videoRecorderHandle);
        videoRecorder.releaseRecorder(videoRecorderHandle);
        if (null != bitmapTemp && !bitmapTemp.isRecycled()) {
            bitmapTemp.recycle();
        }
        return ret;
    }
}
