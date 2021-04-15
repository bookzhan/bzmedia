package bz.luoye.sample.activity;

import android.opengl.GLES20;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.bean.ViewPort;
import com.luoye.bzmedia.widget.BZBaseGLSurfaceView;
import com.luoye.bzmedia.widget.BZVideoView;
import com.nbb.effect.opengl.BaseEffect;
import com.nbb.effect.opengl.EffectEngine;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class EffectsActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "bz_EffectsActivity";

    private BZVideoView bz_video_view;
    private BaseProgram baseProgram;
    private BaseEffect baseEffect;
    private EffectEngine.EffectType effectType = EffectEngine.EffectType.GLITCH;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_effects);
        bz_video_view = findViewById(R.id.bz_video_view);
        String videoPath = FilePathUtil.getReadWorkDir() + "/testvideo.mp4";
        bz_video_view.setDataSource(videoPath);
        bz_video_view.setAutoStartPlay(true);
        bz_video_view.setPlayLoop(true);
        bz_video_view.prepare();
        bz_video_view.start();

        bz_video_view.setOnDrawFrameListener(new BZBaseGLSurfaceView.OnDrawFrameListener() {
            @Override
            public void onDrawFrame(int textureId, long time) {
                onDrawVideoFrame(textureId,time);
            }
        });
        LinearLayout ll_container = findViewById(R.id.ll_container);
        for (EffectEngine.EffectType effectType : EffectEngine.EffectType.values()) {
            Button button = new Button(this);
            button.setText(effectType.name());
            button.setOnClickListener(this);
            button.setTag(effectType);
            ll_container.addView(button);
        }
    }


    private void onDrawVideoFrame(int textureId, long time) {
        if (bz_video_view.getVideoWidth() <= 0 || bz_video_view.getVideoHeight() <= 0) {
            BZLogUtil.e(TAG, "onDrawVideoFrame bz_video_view.getVideoWidth()<=0|| bz_video_view.getVideoHeight()<=0");
            return;
        }
        if (null == baseProgram) {
            baseProgram = new BaseProgram(false);
        }
        if (null == baseEffect) {
            baseEffect = EffectEngine.createEffect(effectType);
            baseEffect.init(this, false, bz_video_view.getVideoWidth(), bz_video_view.getVideoHeight());
        }
        int lastTextureId =textureId;
        //Processing effects
        lastTextureId = baseEffect.onDrawFrame(lastTextureId);

        //Draw to the screen
        ViewPort viewport = bz_video_view.getCurrentDrawViewport();
        GLES20.glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
        baseProgram.draw(lastTextureId);
    }


    @Override
    protected void onPause() {
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (null != baseProgram) {
                    baseProgram.release();
                    baseProgram = null;
                }
                if (null != baseEffect) {
                    baseEffect.release();
                    baseEffect = null;
                }
            }
        });
        bz_video_view.pause();
        bz_video_view.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        bz_video_view.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bz_video_view.release();
    }

    @Override
    public void onClick(View v) {
        effectType = (EffectEngine.EffectType) v.getTag();
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (null != baseEffect) {
                    baseEffect.release();
                    baseEffect = null;
                }
            }
        });
    }
}
