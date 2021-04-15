package bz.luoye.sample.activity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.SeekBar;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.glutils.BZOpenGlUtils;
import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.bzcommon.glutils.LookUpFilterProgram;
import com.bzcommon.glutils.MixTextureProgram;
import com.bzcommon.utils.BZAssetsFileManager;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.AdjustEffectConfig;
import com.luoye.bzmedia.bean.BZColor;
import com.luoye.bzmedia.bean.VideoItem;
import com.luoye.bzmedia.bean.ViewPort;
import com.luoye.bzmedia.opengl.AdjustProgram;
import com.luoye.bzmedia.widget.BZBaseGLSurfaceView;
import com.luoye.bzmedia.widget.BZVideoView;

import java.io.IOException;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

public class VideoFilterActivity extends AppCompatActivity implements View.OnClickListener {
    private static final String TAG = "bz_VideoFilterActivity";

    private LookUpFilterProgram lookUpFilterProgram = null;
    private BZVideoView bz_video_view;
    private String currentTagName;
    private BaseProgram baseProgram = null;
    private BaseProgram baseProgramTemp = null;
    private SeekBar search_bar;
    private SeekBar intensity_bar;
    private LinearLayout ll_adjust_container;
    private float currentProgress = 0;
    private float currentIntensity = 0;
    private long adjustProgramHandle = 0;
    private AdjustEffectConfig adjustEffectConfig = new AdjustEffectConfig();
    private FrameBufferUtil frameBufferUtil_1;
    private FrameBufferUtil frameBufferUtil_2;
    private int frameBufferIndex = 0;
    private MixTextureProgram mixTextureProgram;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_filter);
        bz_video_view = findViewById(R.id.bz_video_view);
        String path = FilePathUtil.getReadWorkDir() + "/testvideo.mp4";
        bz_video_view.addVideoItem(new VideoItem(path, 0, BZMedia.getMediaDuration(path)));
        String path2 = FilePathUtil.getReadWorkDir() + "/testvideo5.mp4";
        bz_video_view.addVideoItem(new VideoItem(path2, 0, BZMedia.getMediaDuration(path2)));
        bz_video_view.setPlayLoop(true);
        bz_video_view.setAutoStartPlay(true);
        bz_video_view.prepare();
        bz_video_view.start();

        bz_video_view.setOnDrawFrameListener(new BZBaseGLSurfaceView.OnDrawFrameListener() {
            @Override
            public void onDrawFrame(int textureId, long time) {
                onDrawVideoFrame(textureId, time);
            }
        });

        LinearLayout ll_container = findViewById(R.id.ll_container);
        Button buttonSRC = new Button(this);
        buttonSRC.setText("SRC");
        buttonSRC.setTag("SRC");
        buttonSRC.setOnClickListener(this);
        ll_container.addView(buttonSRC);
        try {
            String[] filters = getAssets().list("filters");
            for (String filter : filters) {
                Button button = new Button(this);
                button.setText(filter);
                button.setTag(filter);
                button.setOnClickListener(this);
                ll_container.addView(button);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        ll_adjust_container = findViewById(R.id.ll_adjust_container);
        intensity_bar = findViewById(R.id.intensity_bar);
        search_bar = findViewById(R.id.search_bar);
        search_bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    currentProgress = 1.0f * progress / seekBar.getMax();
                }
                View selectedView = null;
                for (int i = 0; i < ll_adjust_container.getChildCount(); i++) {
                    View childAt = ll_adjust_container.getChildAt(i);
                    if (childAt.isSelected()) {
                        selectedView = childAt;
                        break;
                    }
                }
                if (null != selectedView) {
                    selectedView.performClick();
                }

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        intensity_bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    currentIntensity = 1.0f * progress / seekBar.getMax();
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    private void onDrawVideoFrame(int textureId, long time) {
        if (bz_video_view.getVideoWidth() <= 0 || bz_video_view.getVideoHeight() <= 0) {
            BZLogUtil.e(TAG, "onDrawVideoFrame bz_video_view.getVideoWidth()<=0|| bz_video_view.getVideoHeight()<=0");
            return;
        }
        if (null == baseProgram) {
            baseProgram = new BaseProgram(false);
        }
        if (null == baseProgramTemp) {
            baseProgramTemp = new BaseProgram(false);
        }
        ViewPort viewport = bz_video_view.getFirstDrawViewport();
        if (null == viewport) {
            return;
        }
        FrameBufferUtil frameBufferUtil;
        int lastTextureId = textureId;
        //Do the filter first
        if (!"SRC".equalsIgnoreCase(currentTagName) && null != lookUpFilterProgram) {
            frameBufferUtil = getNextFrameBuffer(viewport.width, viewport.height);
            frameBufferUtil.bindFrameBuffer();
            lookUpFilterProgram.draw(lastTextureId);
            frameBufferUtil.unbindFrameBuffer();
            lastTextureId = frameBufferUtil.getFrameBufferTextureID();
        }
        //Processing filter strength
        if (null == mixTextureProgram) {
            mixTextureProgram = new MixTextureProgram(false);
        }
        mixTextureProgram.setMixTexture(lastTextureId);
        mixTextureProgram.setMixturePercent(currentIntensity);


        frameBufferUtil = getNextFrameBuffer(viewport.width, viewport.height);
        GLES20.glViewport(0, 0, viewport.width, viewport.height);
        frameBufferUtil.bindFrameBuffer();
        //Need the texture before the filter
        mixTextureProgram.draw(textureId);
        frameBufferUtil.unbindFrameBuffer();
        lastTextureId = frameBufferUtil.getFrameBufferTextureID();

        //Processing adjustment
        if (adjustProgramHandle == 0) {
            adjustProgramHandle = AdjustProgram.initNative(false);
            AdjustProgram.setAdjustConfig(adjustProgramHandle, adjustEffectConfig);
        }
        frameBufferUtil = getNextFrameBuffer(viewport.width, viewport.height);
        GLES20.glViewport(0, 0, viewport.width, viewport.height);
        frameBufferUtil.bindFrameBuffer();
        AdjustProgram.onDrawFrame(adjustProgramHandle, lastTextureId);
        frameBufferUtil.unbindFrameBuffer();
        lastTextureId = frameBufferUtil.getFrameBufferTextureID();

        //Draw to the screen
        GLES20.glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
        baseProgram.draw(lastTextureId);
    }

    @Override
    protected void onPause() {
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (null != lookUpFilterProgram) {
                    lookUpFilterProgram.release();
                    lookUpFilterProgram = null;
                }
                if (null != mixTextureProgram) {
                    mixTextureProgram.release();
                    mixTextureProgram = null;
                }
                if (null != baseProgram) {
                    baseProgram.release();
                    baseProgram = null;
                }
                if (null != baseProgramTemp) {
                    baseProgramTemp.release();
                    baseProgramTemp = null;
                }
                if (null != frameBufferUtil_1) {
                    frameBufferUtil_1.release();
                    frameBufferUtil_1 = null;
                }
                if (null != frameBufferUtil_2) {
                    frameBufferUtil_2.release();
                    frameBufferUtil_2 = null;
                }
                if (adjustProgramHandle != 0) {
                    AdjustProgram.release(adjustProgramHandle);
                    adjustProgramHandle = 0;
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
    public void onClick(final View v) {
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (null == lookUpFilterProgram) {
                    lookUpFilterProgram = new LookUpFilterProgram(false);
                }
                currentTagName = (String) v.getTag();
                if (!"SRC".equalsIgnoreCase(currentTagName)) {
                    String finalPath = BZAssetsFileManager.getFinalPath(getBaseContext(), "filters/" + currentTagName + "/lookup.png");
                    Bitmap bitmap = BitmapFactory.decodeFile(finalPath);
                    lookUpFilterProgram.setLookUpBitmap(bitmap);
                    bitmap.recycle();
                    BZOpenGlUtils.checkEglError("startFilter");
                }
            }
        });
        bz_video_view.requestRender();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bz_video_view.release();
    }

    private FrameBufferUtil getNextFrameBuffer(int width, int height) {
        //Double buffer exchange
        if (null != frameBufferUtil_1 && (width != frameBufferUtil_1.getWidth() || height != frameBufferUtil_1.getHeight())) {
            frameBufferUtil_1.release();
            frameBufferUtil_1 = null;
        }
        if (null == frameBufferUtil_1) {
            frameBufferUtil_1 = new FrameBufferUtil(width, height);
        }
        if (null != frameBufferUtil_2 && (width != frameBufferUtil_2.getWidth() || height != frameBufferUtil_2.getHeight())) {
            frameBufferUtil_2.release();
            frameBufferUtil_2 = null;
        }
        if (null == frameBufferUtil_2) {
            frameBufferUtil_2 = new FrameBufferUtil(width, height);
        }
        frameBufferIndex++;
        return frameBufferIndex % 2 == 1 ? frameBufferUtil_1 : frameBufferUtil_2;
    }

    private void updateConfig() {
        if (adjustProgramHandle != 0) {
            AdjustProgram.setAdjustConfig(adjustProgramHandle, adjustEffectConfig);
        }
    }

    private void removeAllSelectedView() {
        for (int i = 0; i < ll_adjust_container.getChildCount(); i++) {
            ll_adjust_container.getChildAt(i).setSelected(false);
        }
    }

    private void handleClick(View view) {
        updateConfig();
        removeAllSelectedView();
        view.setSelected(true);
        bz_video_view.requestRender();
    }

    public void shadows(View view) {
        adjustEffectConfig.setShadows(0.45f + currentProgress * (1 - 0.45f));
        handleClick(view);
    }

    public void highlights(View view) {
        adjustEffectConfig.setHighlights(0.25f + currentProgress * (1.75f - 0.25f));
        handleClick(view);
    }

    public void contrast(View view) {
        adjustEffectConfig.setContrast(0.7f + currentProgress * (1.3f - 0.7f));
        handleClick(view);
    }

    public void fadeAmount(View view) {
        adjustEffectConfig.setFadeAmount(0 + currentProgress * (1f - 0f));
        handleClick(view);
    }

    public void saturation(View view) {
        adjustEffectConfig.setSaturation(0 + currentProgress * (2.05f - 0f));
        handleClick(view);
    }

    public void shadowsTintIntensity(View view) {
        adjustEffectConfig.setShadowsTintIntensity(0 + currentProgress * (1f - 0f));
        handleClick(view);
    }

    public void highlightsTintIntensity(View view) {
        adjustEffectConfig.setHighlightsTintIntensity(0.5f + currentProgress * (1f - 0.5f));
        handleClick(view);
    }

    public void shadowsTintColor(View view) {
        BZColor tintColor = adjustEffectConfig.getShadowsTintColor();
        tintColor.setR(1);
        tintColor.setG(0);
        tintColor.setB(0);
        handleClick(view);
    }

    public void highlightsTintColor(View view) {
        BZColor tintColor = adjustEffectConfig.getHighlightsTintColor();
        tintColor.setR(0);
        tintColor.setG(0);
        tintColor.setB(1);
        handleClick(view);
    }

    public void exposure(View view) {
        adjustEffectConfig.setExposure(-1 + currentProgress * (1f + 1f));
        handleClick(view);
    }

    public void warmth(View view) {
        adjustEffectConfig.setWarmth(-1 + currentProgress * (1f + 1f));
        handleClick(view);
    }

    public void green(View view) {
        adjustEffectConfig.setGreen(0.75f + currentProgress * (1.25f - 0.75f));
        handleClick(view);
    }

    public void hueAdjust(View view) {
        adjustEffectConfig.setHueAdjust(-0.174f + currentProgress * (0.174f + 0.174f));
        handleClick(view);
    }

    public void vignette(View view) {
        adjustEffectConfig.setVignette(0f + currentProgress * (1 - 0f));
        handleClick(view);
    }
}
