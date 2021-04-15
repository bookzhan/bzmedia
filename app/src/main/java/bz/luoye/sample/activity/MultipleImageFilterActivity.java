package bz.luoye.sample.activity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.glutils.BZOpenGlUtils;
import com.bzcommon.glutils.FrameBufferUtil;
import com.bzcommon.glutils.LookUpFilterProgram;
import com.bzcommon.utils.BZAssetsFileManager;
import com.luoye.bzmedia.BZMedia;

import java.io.IOException;

import bz.luoye.sample.R;


public class MultipleImageFilterActivity extends AppCompatActivity implements View.OnClickListener {

    private LinearLayout ll_image_container;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_multiple_image_filter);
        ll_image_container = findViewById(R.id.ll_image_container);
    }

    @Override
    public void onClick(View v) {

    }

    /**
     * Used to display the cover image of the filter, the cover image does not need to be so large, which affects the speed
     */
    public void start(View view) {
        view.setEnabled(false);
        ll_image_container.removeAllViews();
        new Thread(new Runnable() {

            @Override
            public void run() {
                try {
//                    String path = FilePathUtil.getWorkDir() + "/testvideo.mp4";
//                    final Bitmap bitmap = BZMedia.getVideoFrameAtTime(path, 0);//This picture can be taken from the video
                    Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test_mxy_min);

                    //The opengl environment cannot be too small, and some phones have problems when they are small
                    int width = 200;
                    int height = bitmap.getHeight() * width / bitmap.getWidth();
                    height = height / 2 * 2;//Align
                    long glContext = BZMedia.initGLContext(width, height);
                    int texture = BZOpenGlUtils.loadTexture(bitmap);
                    bitmap.recycle();

                    LookUpFilterProgram lookUpFilterProgram = new LookUpFilterProgram(false);
                    String[] filters = getAssets().list("filters");
                    FrameBufferUtil frameBufferUtil = new FrameBufferUtil(width, height);
                    for (String filter : filters) {
                        String finalPath = BZAssetsFileManager.getFinalPath(getBaseContext(), "filters/" + filter + "/lookup.png");
                        Bitmap lookUpBitmap = BitmapFactory.decodeFile(finalPath);
                        lookUpFilterProgram.setLookUpBitmap(lookUpBitmap);
                        lookUpBitmap.recycle();
                        frameBufferUtil.bindFrameBuffer();
                        GLES20.glViewport(0, 0, width, height);
                        lookUpFilterProgram.draw(texture);
                        final Bitmap filterBitmap = BZMedia.bzReadPixelsNative(0, 0, width, height);
                        frameBufferUtil.unbindFrameBuffer();
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                ImageView imageView = new ImageView(getBaseContext());
                                imageView.setImageBitmap(filterBitmap);
                                ll_image_container.addView(imageView, new ViewGroup.LayoutParams(100, ViewGroup.LayoutParams.MATCH_PARENT));
                            }
                        });
                    }
                    frameBufferUtil.release();
                    BZOpenGlUtils.deleteTexture(texture);
                    BZMedia.releaseEGLContext(glContext);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }
}
