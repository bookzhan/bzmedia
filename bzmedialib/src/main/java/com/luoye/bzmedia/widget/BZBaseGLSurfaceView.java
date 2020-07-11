package com.luoye.bzmedia.widget;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.bzcommon.glutils.BZOpenGlUtils;
import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzmedia.bean.BZColor;
import com.luoye.bzmedia.bean.ViewPort;

import java.nio.IntBuffer;
import java.util.Locale;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.GL_RGBA;
import static android.opengl.GLES20.GL_UNSIGNED_BYTE;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glViewport;

/**
 * Created by zhandalin on 2017-10-11 11:13.
 * 说明:
 */
public class BZBaseGLSurfaceView extends GLSurfaceView implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {
    private static final String TAG = "bz_BZBaseGLSurfaceView";
    protected SurfaceCallback surfaceCallback;
    private TakePictureCallback takePictureCallback;

    protected boolean mIsUsingMask = false;

    protected boolean mFitFullView = false;

    protected float mMaskAspectRatio = 1.0f;

    private OnViewportCalcCompleteListener onViewportCalcCompleteListener;

    protected int viewWidth;
    protected int viewHeight;
    protected int inputWidth = 1080;
    protected int inputHeight = 1080;
    private boolean isNeedTakeShot;
    protected ViewPort mDrawViewPort = new ViewPort();
    protected BZColor bzClearColor = new BZColor(1, 1, 1, 1);


    protected OnDrawFrameListener onDrawFrameListener;
    private boolean needFlipHorizontal = false;
    private boolean needFlipVertical = true;
    private byte[] dataBuffer = null;
    private BaseProgram baseProgram;

    public BZBaseGLSurfaceView(Context context) {
        this(context, null);
    }

    public BZBaseGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    protected void init() {
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8, 8, 8, 8, 0, 0);
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
        if (null != surfaceCallback) {
            surfaceCallback.surfaceCreated(null);
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        if (null != surfaceCallback)
            surfaceCallback.onSurfaceChanged(width, height);
        this.viewWidth = width;
        this.viewHeight = height;
        calcViewport();
    }

    public void updateData(byte[] rgba, int inputWidth, int inputHeight) {
        if (null == rgba) {
            return;
        }
        if (this.inputWidth != inputWidth || this.inputHeight != inputHeight) {
            this.inputWidth = inputWidth;
            this.inputHeight = inputHeight;
            queueEvent(new Runnable() {
                @Override
                public void run() {
                    calcViewport();
                }
            });
        }
        if (null == dataBuffer || dataBuffer.length != rgba.length) {
            dataBuffer = new byte[rgba.length];
        }
        synchronized (this) {
            System.arraycopy(rgba, 0, dataBuffer, 0, rgba.length);
        }
        requestRender();
    }


    @Override
    public void onPause() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                if (null != baseProgram) {
                    baseProgram.release();
                    baseProgram = null;
                }
            }
        });
        super.onPause();
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
        int textureID;
        synchronized (this) {
            textureID = BZOpenGlUtils.loadTexture(dataBuffer, inputWidth, inputHeight);
        }

        if (null == baseProgram) {
            baseProgram = new BaseProgram(0, needFlipHorizontal, needFlipVertical);
        }
        glViewport(mDrawViewPort.x, mDrawViewPort.y, mDrawViewPort.width, mDrawViewPort.height);
        baseProgram.draw(textureID);
        if (isNeedTakeShot && null != takePictureCallback) {
            IntBuffer buffer;
            Bitmap bmp;

            BZLogUtil.d(TAG, String.format(Locale.CHINESE, "w: %d, h: %d", mDrawViewPort.width, mDrawViewPort.height));
            buffer = IntBuffer.allocate(mDrawViewPort.width * mDrawViewPort.height);
            GLES20.glReadPixels(mDrawViewPort.x, mDrawViewPort.y, mDrawViewPort.width, mDrawViewPort.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
            bmp = Bitmap.createBitmap(mDrawViewPort.width, mDrawViewPort.height, Bitmap.Config.ARGB_8888);
            bmp.copyPixelsFromBuffer(buffer);

            Matrix matrix = new Matrix();
            matrix.postScale(1, -1);   //镜像垂直翻转

            Bitmap result = Bitmap.createBitmap(bmp, 0, 0,
                    bmp.getWidth(), bmp.getHeight(), matrix, true);

            takePictureCallback.takePictureOK(result);
            isNeedTakeShot = false;
        }
        if (null != onDrawFrameListener) {
            onDrawFrameListener.onDrawFrame(textureID);
        }
        BZOpenGlUtils.deleteTexture(textureID);
    }


    public void setClearColor(BZColor bzColor) {
        this.bzClearColor.r = bzColor.r;
        this.bzClearColor.g = bzColor.g;
        this.bzClearColor.b = bzColor.b;
        this.bzClearColor.a = bzColor.a;
    }

    public void addSurfaceCallback(SurfaceCallback surfaceCallback) {
        this.surfaceCallback = surfaceCallback;
    }

    public void setFitFullView(boolean fit) {
        mFitFullView = fit;
        queueEvent(new Runnable() {
            @Override
            public void run() {
                calcViewport();
            }
        });
    }

    public void setOnViewportCalcCompleteListener(OnViewportCalcCompleteListener onViewportCalcCompleteListener) {
        this.onViewportCalcCompleteListener = onViewportCalcCompleteListener;
    }

    public void setOnDrawFrameListener(OnDrawFrameListener onDrawFrameListener) {
        this.onDrawFrameListener = onDrawFrameListener;
    }

    public void takeShot(final TakePictureCallback takePictureCallback) {
        if (null == takePictureCallback) return;
        this.takePictureCallback = takePictureCallback;
        isNeedTakeShot = true;
        requestRender();
    }

    public ViewPort getDrawViewport() {
        return mDrawViewPort;
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        requestRender();
    }

    protected synchronized void calcViewport() {
        if (inputWidth <= 0 || inputHeight <= 0 || viewWidth <= 0 || viewHeight <= 0) {
            return;
        }

        float scaling;
        if (mIsUsingMask) {
            scaling = mMaskAspectRatio;
        } else {
            scaling = inputWidth / (float) inputHeight;
        }

        float viewRatio = viewWidth / (float) viewHeight;
        float s = scaling / viewRatio;

        int w, h;

        if (mFitFullView) {
            //撑满全部view(内容大于view)
            if (s > 1.0) {
                w = (int) Math.ceil(viewHeight * scaling);
                h = viewHeight;
            } else {
                w = viewWidth;
                h = (int) Math.ceil(viewWidth / scaling);
            }
        } else {
            //显示全部内容(内容小于view)
            if (s > 1.0) {
                w = viewWidth;
                h = (int) Math.ceil(viewWidth / scaling);
            } else {
                h = viewHeight;
                w = (int) Math.ceil(viewHeight * scaling);
            }
        }

        //需要转换成2的整数倍
        mDrawViewPort.width = w;
        mDrawViewPort.height = h;
        mDrawViewPort.x = (viewWidth - mDrawViewPort.width) / 2;
        mDrawViewPort.y = (viewHeight - mDrawViewPort.height) / 2;

        if (null != onViewportCalcCompleteListener)
            onViewportCalcCompleteListener.onViewportCalcCompleteListener(mDrawViewPort);
        BZLogUtil.d(TAG, mDrawViewPort.toString());
    }

    public void setFlip(final boolean needFlipHorizontal, final boolean needFlipVertical) {
        this.needFlipHorizontal = needFlipHorizontal;
        this.needFlipVertical = needFlipVertical;
    }


    @Override
    public boolean performClick() {
        return super.performClick();
    }

    public interface SurfaceCallback {
        void surfaceCreated(SurfaceTexture mSurface);

        void onSurfaceChanged(int width, int height);
    }

    public interface TakePictureCallback {
        //传入的bmp可以由接收者recycle
        void takePictureOK(Bitmap bmp);
    }

    public interface OnViewportCalcCompleteListener {
        void onViewportCalcCompleteListener(ViewPort viewport);
    }

    public interface OnDrawFrameListener {
        void onDrawFrame(int textureId);
    }
}
