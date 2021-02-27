package bz.luoye.sample.activity;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.bzcommon.utils.BZFileUtils;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class GetImageFromVideoActivity extends AppCompatActivity {

    private LinearLayout ll_image_container;
    private String TAG = "bz_GetImageFromVideo";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_get_image_from_video);
        ll_image_container = findViewById(R.id.ll_image_container);
    }

    public void getImageFromVideo(View view) {
        String path = FilePathUtil.getWorkDir() + "/testvideo.mp4";
        BZMedia.getImageFromVideo(path, FilePathUtil.getWorkDir() + "/", 10, 200, new BZMedia.OnGetImageFromVideoListener() {
            @Override
            public void onGetImageFromVideo(int index, String imagePath) {
                BZLogUtil.d(TAG, "onGetImageFromVideo index=" + index + " imagePath=" + imagePath);
            }
        });
    }

    public void getBitmapFromVideo(View view) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                String path = FilePathUtil.getWorkDir() + "/testvideo.mp4";
                BZMedia.getBitmapFromVideo(path, 18, 200, new BZMedia.OnGetBitmapFromVideoListener() {
                    @Override
                    public void onGetBitmapFromVideo(int index, final Bitmap bitmap) {
                        BZLogUtil.d(TAG, "onGetBitmapFromVideo index=" + index);
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                ImageView imageView = new ImageView(GetImageFromVideoActivity.this);
                                imageView.setImageBitmap(bitmap);
                                LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(50, ViewGroup.LayoutParams.MATCH_PARENT);
                                ll_image_container.addView(imageView, layoutParams);
                            }
                        });
                    }
                });
            }
        }).start();
    }
}
