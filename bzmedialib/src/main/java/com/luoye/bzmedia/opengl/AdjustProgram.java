package com.luoye.bzmedia.opengl;

import com.luoye.bzmedia.bean.AdjustEffectConfig;

/**
 * Created by bookzhan on 2021-03-11 15:19.
 * description:
 */
public class AdjustProgram {
    public native static long initNative(boolean flipVertical);

    public native static int setAdjustConfig(long nativeHandle, AdjustEffectConfig adjustEffectConfig);

    public native static int onDrawFrame(long nativeHandle, int textureId);

    public native static int setFlip(long nativeHandle, boolean needFlipHorizontal, boolean needFlipVertical);

    public native static int release(long nativeHandle);
}
