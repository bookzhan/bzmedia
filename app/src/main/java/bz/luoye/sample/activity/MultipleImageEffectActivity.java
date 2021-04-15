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
import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.luoye.bzmedia.BZMedia;
import com.nbb.effect.opengl.BaseEffect;
import com.nbb.effect.opengl.EffectEngine;

import bz.luoye.sample.R;

import static android.opengl.GLES20.glClearColor;


public class MultipleImageEffectActivity extends AppCompatActivity implements View.OnClickListener {

    private LinearLayout ll_image_container;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_multiple_image_effect);
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
                    final int width = 200;
                    int height = bitmap.getHeight() * width / bitmap.getWidth();
                    height = height / 2 * 2;//Align

                    long glContext = BZMedia.initGLContext(width, height);
                    int texture = BZOpenGlUtils.loadTexture(bitmap);
                    BaseProgram baseProgram = new BaseProgram(false);
                    bitmap.recycle();
                    FrameBufferUtil frameBufferUtil = new FrameBufferUtil(width, height);
                    for (int i = 0; i < EffectEngine.EffectType.values().length; i++) {
                        EffectEngine.EffectType effectType = EffectEngine.EffectType.values()[i];
                        BaseEffect baseEffect = EffectEngine.createEffect(effectType);
                        baseEffect.init(getBaseContext(), false, width, height);

                        GLES20.glViewport(0, 0, width, height);
                        glClearColor(1, 0, 0, 1);
                        int lastTextureId = baseEffect.onDrawFrame(texture, 0.5f);

                        frameBufferUtil.bindFrameBuffer();
                        baseProgram.draw(lastTextureId);
                        final Bitmap filterBitmap = BZMedia.bzReadPixelsNative(0, 0, width, height);
                        frameBufferUtil.unbindFrameBuffer();

                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                ImageView imageView = new ImageView(getBaseContext());
                                imageView.setImageBitmap(filterBitmap);
                                ll_image_container.addView(imageView, new ViewGroup.LayoutParams(width, ViewGroup.LayoutParams.MATCH_PARENT));
                            }
                        });
                        baseEffect.release();
                    }
                    frameBufferUtil.release();
                    BZOpenGlUtils.deleteTexture(texture);
                    BZMedia.releaseEGLContext(glContext);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }
}
