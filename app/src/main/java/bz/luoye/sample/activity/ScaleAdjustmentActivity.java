package bz.luoye.sample.activity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.opengl.GLES20;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.SeekBar;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.glutils.BZOpenGlUtils;
import com.bzcommon.glutils.BaseMatrixProgram;
import com.bzcommon.utils.BZAssetsFileManager;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.BZColor;
import com.luoye.bzmedia.bean.BgFillType;
import com.luoye.bzmedia.bean.TextureHandleInfo;
import com.luoye.bzmedia.bean.VideoItem;
import com.luoye.bzmedia.bean.ViewPort;
import com.luoye.bzmedia.opengl.BZRenderEngine;
import com.luoye.bzmedia.opengl.CropTextureUtil;
import com.luoye.bzmedia.widget.BZBaseGLSurfaceView;
import com.luoye.bzmedia.widget.BZVideoView;

import java.util.Arrays;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;
import com.luoye.bzmedia.widget.gesturedetectors.MultiTouchListener;

import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;

public class ScaleAdjustmentActivity extends AppCompatActivity {

    private static final String TAG = "bz_ScaleAdjustment";
    private BZVideoView bz_video_view;
    private SeekBar seek_bar;
    private BaseMatrixProgram baseMatrixProgram;
    private long nativeHandle;
    private TextureHandleInfo textureHandleInfo = new TextureHandleInfo();
    private int finalWidth;
    private int finalHeight;
    //The values of cropRect are all relative to the percentage of width and height, and they are all relative to the original video frame.
    private RectF cropRect = new RectF(0, 0, 1, 1);
    private long cropTextureNativeHandle = 0;
    private float[] viewMatrixValues = new float[9];
    private float[] glMatrixValues = new float[]{1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
    private View test_view;
    private ViewPort firstViewport;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_scale_adjustment);
        bz_video_view = findViewById(R.id.bz_video_view);
        test_view = findViewById(R.id.test_view);
        String path = FilePathUtil.getReadWorkDir() + "/testvideo.mp4";
        VideoItem videoItem1 = new VideoItem(path, 5000, 10000);

        String path2 = FilePathUtil.getReadWorkDir() + "/testvideo5.mp4";
        VideoItem videoItem2 = new VideoItem(path2, 0, BZMedia.getMediaDuration(path2));
        bz_video_view.addVideoItem(videoItem1);
        bz_video_view.addVideoItem(videoItem2);
        bz_video_view.setPlayLoop(true);
        bz_video_view.setOnDrawFrameListener(new BZBaseGLSurfaceView.OnDrawFrameListener() {
            @Override
            public void onDrawFrame(int textureId, long time) {
                onDrawVideoFrame(textureId, time);
            }
        });
        bz_video_view.setOnViewportCalcCompleteListener(new BZBaseGLSurfaceView.OnViewportCalcCompleteListener() {
            @Override
            public void onViewportCalcCompleteListener(final ViewPort viewport) {
                ViewGroup.LayoutParams layoutParams = test_view.getLayoutParams();
                if (layoutParams instanceof RelativeLayout.LayoutParams) {
                    final RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) layoutParams;
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            lp.setMargins(viewport.x, viewport.y, viewport.x, viewport.y);
                            test_view.setLayoutParams(lp);
                        }
                    });
                }
            }
        });
        bz_video_view.prepare();
        bz_video_view.start();

        seek_bar = findViewById(R.id.seek_bar);
        seek_bar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    float currentProgress = 1.0f * progress / seekBar.getMax() * 10;
                    textureHandleInfo.setBgTextureIntensity(currentProgress);
                    bz_video_view.queueEvent(new Runnable() {
                        @Override
                        public void run() {
                            BZRenderEngine.setTextureHandleInfo(nativeHandle, textureHandleInfo);
                        }
                    });
                    bz_video_view.requestRender();
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        MultiTouchListener multiTouchListener = new MultiTouchListener();
        multiTouchListener.isRotateEnabled = false;
        test_view.setOnTouchListener(multiTouchListener);
        multiTouchListener.setOnActionListener(new MultiTouchListener.OnActionListener() {
            @Override
            public void onMove(Matrix matrix) {
                BZLogUtil.d(TAG, "matrixValues=" + Arrays.toString(viewMatrixValues));
                matrixConversion(matrix, glMatrixValues, test_view.getWidth(), test_view.getHeight());
                bz_video_view.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        if (null != baseMatrixProgram)
                            baseMatrixProgram.setMatrix(glMatrixValues);
                    }
                });
                bz_video_view.requestRender();
            }
        });
    }

    private void onDrawVideoFrame(int textureId, long time) {
        if (bz_video_view.getVideoWidth() <= 0 || bz_video_view.getVideoHeight() <= 0) {
            BZLogUtil.e(TAG, "onDrawVideoFrame bz_video_view.getVideoWidth()<=0|| bz_video_view.getVideoHeight()<=0");
            return;
        }
        if (null == baseMatrixProgram) {
            baseMatrixProgram = new BaseMatrixProgram(false);
        }
        ViewPort viewport = bz_video_view.getFirstDrawViewport();
        if (null == viewport) {
            return;
        }
        if (nativeHandle == 0) {
            firstViewport = new ViewPort(viewport.x, viewport.y, viewport.width, viewport.height);
            nativeHandle = BZRenderEngine.init();
            //Initially, the first video area is the final working area
            finalWidth = viewport.width;
            finalHeight = viewport.height;
            BZRenderEngine.setFinalSize(nativeHandle, viewport.width, viewport.height);
            BZRenderEngine.setFlip(nativeHandle, false, false);
            BZRenderEngine.setTextureHandleInfo(nativeHandle, textureHandleInfo);
        }
        int lastTextureId = textureId;
        int textureWidth = viewport.width;
        int textureHeight = viewport.height;

        //Processing cropping, each video should record cropping information separately, here is simple and unified processing
        if (null != cropRect) {
            if (cropTextureNativeHandle == 0) {
                cropTextureNativeHandle = CropTextureUtil.initCropTexture();
            }
            int startX = (int) (cropRect.left * textureWidth);
            int startY = (int) ((1 - cropRect.bottom) * textureHeight);//GL环境中Y轴是反的
            int width = (int) (textureWidth * cropRect.width());
            int height = (int) (textureHeight * cropRect.height());
            lastTextureId = CropTextureUtil.cropTexture(cropTextureNativeHandle, lastTextureId, textureWidth, textureHeight, startX, startY, width, height);
            textureWidth = width;
            textureHeight = height;
        }
        //Uniform size, fill type, fill background, zoom information, etc.
        lastTextureId = BZRenderEngine.draw(nativeHandle, lastTextureId, textureWidth, textureHeight);


        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (finalWidth > 0 && finalHeight > 0) {
            int viewWidth = bz_video_view.getWidth();
            int viewHeight = bz_video_view.getHeight();
            int x = (viewWidth - finalWidth) / 2;
            int y = (viewHeight - finalHeight) / 2;
            GLES20.glViewport(x, y, finalWidth, finalHeight);
            baseMatrixProgram.draw(lastTextureId);
        } else {
            //After BZRenderEngine processing, the size becomes the same as the first one, so it is always drawn at the first size
            GLES20.glViewport(firstViewport.x, firstViewport.y, firstViewport.width, firstViewport.height);
            baseMatrixProgram.draw(lastTextureId);
        }
    }

    @Override
    protected void onPause() {
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (null != baseMatrixProgram) {
                    baseMatrixProgram.release();
                    baseMatrixProgram = null;
                }
                if (nativeHandle != 0) {
                    BZRenderEngine.release(nativeHandle);
                    nativeHandle = 0;
                }
                if (cropTextureNativeHandle != 0) {
                    CropTextureUtil.cropTextureRelease(cropTextureNativeHandle);
                    cropTextureNativeHandle = 0;
                }
                if (textureHandleInfo.getBgTextureId() > 0) {
                    BZOpenGlUtils.deleteTexture(textureHandleInfo.getBgTextureId());
                    textureHandleInfo.setBgTextureId(0);
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

    private boolean hasInit() {
        return nativeHandle != 0 && bz_video_view.getVideoWidth() > 0 && bz_video_view.getVideoHeight() > 0;
    }

    private void setScale(float scale) {
        finalWidth = bz_video_view.getWidth();
        finalHeight = (int) (finalWidth / scale);
        if (finalHeight > bz_video_view.getHeight()) {
            finalHeight = bz_video_view.getHeight();
            finalWidth = (int) (finalHeight * scale);
        }
//        bz_video_view.pause();
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                BZRenderEngine.setFinalSize(nativeHandle, finalWidth, finalHeight);
            }
        });
        bz_video_view.requestRender();
    }

    public void Scale1_1(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(1.0f / 1);
    }

    public void Scale4_5(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(4.0f / 5);
    }

    public void Scale16_9(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(16.0f / 9);
    }

    public void Scale9_16(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(9.0f / 16);
    }

    public void Scale3_4(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(3.0f / 4);
    }

    public void Scale4_3(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(4.0f / 3);
    }

    public void Scale2_3(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(2.0f / 3);
    }

    public void Scale3_2(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(3.0f / 2);
    }

    public void Scale2_1(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(2.0f / 1);
    }

    public void Scale1_2(View view) {
        if (!hasInit()) {
            return;
        }
        setScale(1.0f / 2);
    }

    public void setBgImage(View view) {
//        bz_video_view.pause();
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                textureHandleInfo.setBgFillType(BgFillType.IMAGE);
                if (textureHandleInfo.getBgTextureId() > 0) {
                    BZOpenGlUtils.deleteTexture(textureHandleInfo.getBgTextureId());
                    textureHandleInfo.setBgTextureId(0);
                }
                String finalPath = BZAssetsFileManager.getFinalPath(getBaseContext(), "test_mxy.png");
                textureHandleInfo.setBgTexturePath(finalPath);
                Bitmap bitmap = BitmapFactory.decodeFile(finalPath);
                textureHandleInfo.setBgTextureWidth(bitmap.getWidth());
                textureHandleInfo.setBgTextureHeight(bitmap.getHeight());
                int texture = BZOpenGlUtils.loadTexture(bitmap);
                textureHandleInfo.setBgTextureId(texture);
                BZRenderEngine.setTextureHandleInfo(nativeHandle, textureHandleInfo);
                bitmap.recycle();
            }
        });
        bz_video_view.requestRender();
    }

    public void setBgColor(View view) {
        if (!hasInit()) {
            return;
        }
//        bz_video_view.pause();
        textureHandleInfo.setBgFillType(BgFillType.COMMON_COLOR);
        textureHandleInfo.setBgColor(new BZColor(1, 0, 0, 1));
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                BZRenderEngine.setTextureHandleInfo(nativeHandle, textureHandleInfo);
            }
        });
        bz_video_view.requestRender();
    }

    public void resetBgColor(View view) {
//        bz_video_view.pause();
        textureHandleInfo.setBgFillType(BgFillType.GAUSS_BLUR);
        textureHandleInfo.setBgColor(null);
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                BZRenderEngine.setTextureHandleInfo(nativeHandle, textureHandleInfo);
            }
        });
        bz_video_view.requestRender();
    }

    public void cropRect(View view) {
//        bz_video_view.pause();
        bz_video_view.queueEvent(new Runnable() {
            @Override
            public void run() {
                //这个坐标是View系统的坐标
                cropRect.set(0, 0.25f, 1, 0.75f);
            }
        });
        bz_video_view.requestRender();
    }

    private void matrixConversion(Matrix viewMatrix, float[] glMatrix, int viewWidth, int viewHeight) {
        if (null == viewMatrix || null == glMatrix || glMatrix.length < 16 || viewWidth <= 0 || viewHeight <= 0) {
            return;
        }
        viewMatrix.getValues(viewMatrixValues);

        float[] floats = {viewWidth / 2f, viewHeight / 2f};
        viewMatrix.mapPoints(floats);

        //gl -1~1
        float translationX = ((floats[0] - viewWidth / 2f) / viewWidth) * 2;
        float translationY = -((floats[1] - viewHeight / 2f) / viewHeight) * 2;

        float scaleX = viewMatrixValues[0];
        float scaleY = viewMatrixValues[4];

        android.opengl.Matrix.setIdentityM(glMatrix, 0);
        android.opengl.Matrix.translateM(glMatrix, 0, translationX, translationY, 0);
        android.opengl.Matrix.scaleM(glMatrix, 0, scaleX, scaleY, 1);
    }

    public void enableZoomAndTranslation(View view) {
        test_view.setVisibility(View.VISIBLE);
    }
}
