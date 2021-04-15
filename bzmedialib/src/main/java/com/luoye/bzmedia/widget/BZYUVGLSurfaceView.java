package com.luoye.bzmedia.widget;

import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glViewport;

import android.content.Context;
import android.util.AttributeSet;

import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.luoye.bzmedia.opengl.YUV420Program;

import javax.microedition.khronos.opengles.GL10;

/**
 * Created by bookzhan on 2022-02-08 23:23.
 * description:
 */
public class BZYUVGLSurfaceView extends BZBaseGLSurfaceView {
    private byte[] yuvDataBuffer = null;
    private long yuv420ProgramNativeHandle = 0;
    private boolean requestReleaseProgram = false;
    private FrameBufferUtil frameBufferUtil;

    public BZYUVGLSurfaceView(Context context) {
        super(context);
    }

    public BZYUVGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void updateYUVData(byte[] yuvData, int inputWidth, int inputHeight) {
        if (null == yuvData) {
            return;
        }
        setInputSize(inputWidth, inputHeight);
        if (null == yuvDataBuffer || yuvDataBuffer.length != yuvData.length) {
            yuvDataBuffer = new byte[yuvData.length];
        }
        synchronized (this) {
            System.arraycopy(yuvData, 0, yuvDataBuffer, 0, yuvData.length);
        }
        requestRender();
    }

    @Override
    public void onPause() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                releaseGLResource();
            }
        });
        super.onPause();
    }

    @Override
    public void setFlip(final boolean needFlipHorizontal, final boolean needFlipVertical) {
        if (this.needFlipHorizontal != needFlipHorizontal || this.needFlipVertical != needFlipVertical) {
            requestReleaseProgram = true;
        }
        this.needFlipHorizontal = needFlipHorizontal;
        this.needFlipVertical = needFlipVertical;
    }

    private void releaseGLResource() {
        if (yuv420ProgramNativeHandle != 0) {
            YUV420Program.release(yuv420ProgramNativeHandle);
            yuv420ProgramNativeHandle = 0;
        }
        if (null != baseProgram) {
            baseProgram.release();
            baseProgram = null;
        }
        if (null != frameBufferUtil) {
            frameBufferUtil.release();
            frameBufferUtil = null;
        }
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
        if (null == yuvDataBuffer) {
            return;
        }
        if (requestReleaseProgram) {
            releaseGLResource();
            requestReleaseProgram = false;
        }
        if (yuv420ProgramNativeHandle == 0) {
            yuv420ProgramNativeHandle = YUV420Program.init();
            YUV420Program.setFlip(yuv420ProgramNativeHandle, needFlipHorizontal, needFlipVertical);
        }
        if (null != mFirstDrawViewPort) {
            glViewport(mFirstDrawViewPort.x, mFirstDrawViewPort.y, mFirstDrawViewPort.width, mFirstDrawViewPort.height);
        }

        if (null != onDrawFrameListener) {
            if (null == baseProgram) {
                baseProgram = new BaseProgram(0, needFlipHorizontal, needFlipVertical);
            }
            if (null == frameBufferUtil) {
                frameBufferUtil = new FrameBufferUtil(inputWidth, inputHeight);
            }
            frameBufferUtil.bindFrameBuffer();
            glViewport(0, 0, frameBufferUtil.getWidth(), frameBufferUtil.getHeight());
            synchronized (this) {
                YUV420Program.draw(yuv420ProgramNativeHandle, yuvDataBuffer, inputWidth, inputHeight);
            }
            frameBufferUtil.unbindFrameBuffer();
            onDrawFrameListener.onDrawFrame(frameBufferUtil.getFrameBufferTextureID(), 0);
        } else {
            synchronized (this) {
                YUV420Program.draw(yuv420ProgramNativeHandle, yuvDataBuffer, inputWidth, inputHeight);
            }
        }
    }
}
