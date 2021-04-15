package bz.luoye.sample.activity;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class GetImageFromVideoActivity extends AppCompatActivity {
    private final static String TAG = "bz_GetImageFromVideo";
    private final static String inputPath = FilePathUtil.getReadWorkDir() + "/testvideo.mp4";
    //String inputPath = FilePathUtil.getWorkDir() + "/test_key.MP4";
    //String inputPath = FilePathUtil.getWorkDir() + "/different_formats/drop.avi";
    //String inputPath = FilePathUtil.getWorkDir() + "/different_formats/elephants-dream.webm";
    //String inputPath = FilePathUtil.getWorkDir() + "/different_formats/sample_960x400_ocean_with_audio.flv";
    //String inputPath = FilePathUtil.getWorkDir() + "/different_formats/sample_960x400_ocean_with_audio.mkv";
    //测试失败,原因不明
    //String inputPath = FilePathUtil.getWorkDir() + "/different_formats/sample_960x400_ocean_with_audio.mts";
//    String inputPath = FilePathUtil.getWorkDir() + "/different_formats/sample_960x400_ocean_with_audio.wmv";

    private LinearLayout ll_image_container;
    private ImageView iv_test;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_get_image_from_video);
        ll_image_container = findViewById(R.id.ll_image_container);
        iv_test = findViewById(R.id.iv_test);
    }

    public void getBitmapFromVideo(View view) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                long startTime = System.currentTimeMillis();
                BZMedia.getBitmapFromVideo(inputPath, 10, 200, 2300, 12000, new BZMedia.OnGetBitmapFromVideoListener() {
                    @Override
                    public void onGetBitmapFromVideo(int index, final Bitmap bitmap) {
                        BZLogUtil.d(TAG, "onGetBitmapFromVideo index=" + index);
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                ImageView imageView = new ImageView(GetImageFromVideoActivity.this);
                                imageView.setImageBitmap(bitmap);
                                LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(100, ViewGroup.LayoutParams.MATCH_PARENT);
                                ll_image_container.addView(imageView, layoutParams);
                            }
                        });
                    }
                });
                BZLogUtil.d(TAG, "time cost=" + (System.currentTimeMillis() - startTime));
            }
        }).start();
    }

    public void getVideoFrameAtTime(View view) {
        iv_test.setVisibility(View.VISIBLE);
        new Thread(new Runnable() {
            @Override
            public void run() {
                final Bitmap bitmap = BZMedia.getVideoFrameAtTime(inputPath, 3200, 300);
                iv_test.post(new Runnable() {
                    @Override
                    public void run() {
                        iv_test.setImageBitmap(bitmap);
                    }
                });
            }
        }).start();
    }
}
