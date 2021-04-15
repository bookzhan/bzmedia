package com.luoye.bzmedia.widget;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.AttributeSet;

import com.bzcommon.glutils.BZOpenGlUtils;
import com.bzcommon.glutils.BaseProgram;

import java.lang.ref.WeakReference;

import javax.microedition.khronos.opengles.GL10;

import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glIsTexture;
import static android.opengl.GLES20.glViewport;

/**
 * Created by bookzhan on 2021-03-10 15:48.
 * description:
 */
public class BZImageGLSurfaceView extends BZBaseGLSurfaceView {
    private static final String TAG = "bz_BZImageGLSurfaceView";
    private int textureID = -1;
    private WeakReference<Bitmap> bitmapWeakReference = null;

    public BZImageGLSurfaceView(Context context) {
        super(context);
    }

    public BZImageGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setBitmap(final Bitmap bitmap) {
        bitmapWeakReference = new WeakReference<>(bitmap);
        setInputSize(bitmap.getWidth(), bitmap.getHeight());
        requestRender();
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!glIsTexture(textureID)) {
            if (null == bitmapWeakReference) {
                return;
            }
            Bitmap bitmap = bitmapWeakReference.get();
            if (null == bitmap) {
                return;
            }
            textureID = BZOpenGlUtils.loadTexture(bitmap);
        }
        if (null == baseProgram) {
            baseProgram = new BaseProgram(0, needFlipHorizontal, needFlipVertical);
        }
        glViewport(mDrawViewPort.x, mDrawViewPort.y, mDrawViewPort.width, mDrawViewPort.height);
        baseProgram.draw(textureID);
        if (null != onDrawFrameListener) {
            onDrawFrameListener.onDrawFrame(textureID, 0);
        }
    }
}
