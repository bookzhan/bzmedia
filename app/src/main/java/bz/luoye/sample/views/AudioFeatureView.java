package bz.luoye.sample.views;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Created by bookzhan on 2018-09-03 11:05.
 * Description:
 */
public class AudioFeatureView extends View {
    private Paint paint;
    private String TAG = "bz_AudioFeatureView";
    private List<Float> audioFeatureList = new ArrayList<>();
    private boolean audioFeatureInitFinish = false;
    private Random random;

    public AudioFeatureView(Context context) {
        this(context, null);
    }

    public AudioFeatureView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public AudioFeatureView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        paint = new Paint();
        paint.setColor(Color.RED);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(0.5f);
        random = new Random(System.currentTimeMillis());
    }

    public void setAudioPath(String audioPath) {
        if (null == audioPath) {
            return;
        }
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                long startTime = System.currentTimeMillis();
                //samples controls the number of samples. The smaller the number, the more data points there will be. The native implementation is to collect data once every interval.
                BZMedia.getAudioFeatureInfo(audioPath, 16, new BZMedia.AudioFeatureInfoListener() {
                    @Override
                    public void onAudioFeatureInfo(long audioTime, float featureValue) {
//                BZLogUtil.d(TAG, "audioTime=" + audioTime + " featureValue=" + featureValue);
                        //Filter out some extreme values
                        if (featureValue > 15) featureValue = 10 + random.nextInt(5);
                        if (featureValue < -15) featureValue = -10 - random.nextInt(5);
                        audioFeatureList.add(featureValue);
                    }
                });
                BZLogUtil.d(TAG, "getAudioFeatureInfo time consuming=" + (System.currentTimeMillis() - startTime));
                audioFeatureInitFinish = true;
                postInvalidate();
            }
        });
        thread.setPriority(Thread.MAX_PRIORITY);
        thread.start();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        float startX = 0;
        int width = getWidth();
        if (width <= 0 || audioFeatureList.isEmpty() || !audioFeatureInitFinish) {
            return;
        }
        long startTime = System.currentTimeMillis();
        float space = 1.0f * width / audioFeatureList.size();

//        for (Float audioFeature : audioFeatureList) {
//            //Data magnification 3x
//            float featureValue = audioFeature * 3;
//            startX += space;
//            canvas.drawLine(startX, getHeight() / 2f, startX + 1, getHeight() / 2f + featureValue, paint);
//        }

        // nbb
        float lastStartX = 0;
        float lastStartY = getHeight() / 2f;
        for (Float audioFeature : audioFeatureList) {
            //Data magnification 3x
            float featureValue = audioFeature * 3;
            startX += space;
            canvas.drawLine(lastStartX, lastStartY, startX, getHeight() / 2f + featureValue, paint);
            lastStartX = startX;
            lastStartY = getHeight() / 2f + featureValue;
        }
        BZLogUtil.d(TAG, "Drawing time-consuming=" + (System.currentTimeMillis() - startTime));
    }
}
