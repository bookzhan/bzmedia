package com.luoye.bzmedia.widget;

import android.content.Context;
import android.os.SystemClock;
import android.util.AttributeSet;

import com.bzcommon.utils.BZDeviceUtils;
import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZStringUtils;
import com.google.android.exoplayer2.MediaItem;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.luoye.bzmedia.player.VideoPlayer;

import java.io.File;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glViewport;

/**
 * Created by bookzhan on 2021-05-11 09:25.
 * Description:
 */
@Deprecated
public class BZNativeVideoView extends BZBaseGLSurfaceView {
    private static final String TAG = "bz_BZVideoView";
    private static final int MSG_WHAT_UPDATE_PROGRESS = 0;
    private static final int MSG_WHAT_UPDATE_SURFACE = 1;
    private int videoRotate;
    private String videoPath;
    private OnCompletionListener onCompletionListener;
    private OnProgressChangedListener onProgressChangedListener;
    private long frameStartTimeMs;
    private float volume = 1;

    private boolean playLoop = true;
    private boolean isFirstRender = true;
    private boolean autoStartPlay = true;
    private OnStartRenderListener onStartRenderListener = null;
    private boolean needFadeShow = true;
    private float videoFps = 30;
    private int videoWidth = -1;
    private int videoHeight = -1;
    private boolean continuouslyRender = true;
    private float speed = 1;
    private long currentPosition = 0;
    private long videoDuration = -1;
    private long videoPlayerHandle = 0;
    private long lastDrawTime;


    public BZNativeVideoView(Context context) {
        this(context, null);
    }

    public BZNativeVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setRenderMode(RENDERMODE_CONTINUOUSLY);
    }

    public SimpleExoPlayer getExoPlayer() {
        return null;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        super.onSurfaceCreated(gl, config);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        super.onSurfaceChanged(gl, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);

        if (null != onDrawFrameListener) {
            //通过调用videoPlayerOnDrawFrame完成最终的绘制
            onDrawFrameListener.onDrawFrame(0,0);
        } else {
            glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
            if (null != mDrawViewPort)
                glViewport(mDrawViewPort.x, mDrawViewPort.y, mDrawViewPort.width, mDrawViewPort.height);
            videoPlayerOnDrawFrame();
        }
        if (continuouslyRender) {
            limitFrameRate();
        }
        if (isFirstRender) {
            isFirstRender = false;
            if (null != onStartRenderListener) {
                onStartRenderListener.onStartRender();
            }
        }
    }

    protected void limitFrameRate() {
        try {
            long elapsedFrameTimeMs = SystemClock.elapsedRealtime() - frameStartTimeMs;
            //33ms
            long expectedFrameTimeMs = 1000 / 33;
            long timeToSleepMs = expectedFrameTimeMs - elapsedFrameTimeMs;
            if (timeToSleepMs > 0) {
                SystemClock.sleep(timeToSleepMs);
            }
            frameStartTimeMs = SystemClock.elapsedRealtime();
        } catch (Throwable e) {
            BZLogUtil.e(TAG, e);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        VideoPlayer.onPause(videoPlayerHandle);
    }

    public void setMediaItem(MediaItem mediaItems) {
        String mediaId = mediaItems.mediaId;
        BZLogUtil.d(TAG, "mediaId=" + mediaId);
        if (!checkVideoPath(mediaId)) {
            BZLogUtil.e(TAG, "videoPath is not available");
            return;
        }
//        exoPlayer.setMediaItem(mediaItems);
    }

    public void addMediaItems(List<MediaItem> mediaItems) {
        if (null == mediaItems || mediaItems.isEmpty()) {
            BZLogUtil.e(TAG, "null == mediaItems || mediaItems.isEmpty()");
            return;
        }
        for (MediaItem mediaItem : mediaItems) {
            String mediaId = mediaItem.mediaId;
            BZLogUtil.d(TAG, "mediaId=" + mediaId);
            if (!checkVideoPath(mediaId)) {
                BZLogUtil.e(TAG, "videoPath is not available");
                return;
            }
        }
//        exoPlayer.addMediaItems(mediaItems);
    }

    public void addMediaItems(int index, List<MediaItem> mediaItems) {
        if (null == mediaItems || mediaItems.isEmpty()) {
            BZLogUtil.e(TAG, "null == mediaItems || mediaItems.isEmpty()");
            return;
        }
        for (MediaItem mediaItem : mediaItems) {
            String mediaId = mediaItem.mediaId;
            BZLogUtil.d(TAG, "mediaId=" + mediaId);
            if (!checkVideoPath(mediaId)) {
                BZLogUtil.e(TAG, "videoPath is not available");
                return;
            }
        }
//        exoPlayer.addMediaItems(index, mediaItems);
    }

    public void moveMediaItem(int currentIndex, int newIndex) {
//        exoPlayer.moveMediaItem(currentIndex, newIndex);
    }

    public int getMediaItemCount() {
        return 1;
    }

    public MediaItem getMediaItemAt(int index) {
        return null;
    }

    public void addMediaItem(MediaItem mediaItem) {
        String mediaId = mediaItem.mediaId;
        BZLogUtil.d(TAG, "mediaId=" + mediaId);
        if (!checkVideoPath(mediaId)) {
            BZLogUtil.e(TAG, "videoPath is not available");
            return;
        }
//        exoPlayer.addMediaItem(mediaItem);
    }

    private static boolean checkVideoPath(String videoPath) {
        if (null == videoPath) {
            return false;
        }
        if (videoPath.startsWith("/")) {
            return checkFile(videoPath);
        }
        return true;
    }

    private static boolean checkFile(String path) {
        if (BZStringUtils.isNotEmpty(path)) {
            File f = new File(path);
            return f.exists() && f.canRead() && f.isFile() && f.length() > 0;
        }
        return false;
    }

    public void setDataSource(final String videoPath) {
        if (!checkVideoPath(videoPath)) {
            BZLogUtil.e(TAG, "videoPath is not available");
            return;
        }
        videoPlayerHandle = VideoPlayer.init(videoPath, new VideoPlayer.VideoPlayerStateListener() {
            @Override
            public void onProgressChanged(float progress) {

            }

            @Override
            public void onVideoPlayCompletion(int flag) {

            }

            @Override
            public void onVideoInfoAvailable(int videoWidth, int videoHeight, int videoRotate, long videoDuration, float videoFps) {
                BZNativeVideoView.this.videoRotate = videoRotate;
                BZNativeVideoView.this.videoWidth = videoWidth;
                BZNativeVideoView.this.videoHeight = videoHeight;
                BZNativeVideoView.this.videoFps = videoFps;
                BZNativeVideoView.this.videoDuration = videoDuration;
                if (videoRotate == 90 || videoRotate == 270) {
                    BZNativeVideoView.this.videoWidth = videoHeight;
                    BZNativeVideoView.this.videoHeight = videoWidth;
                }
                setInputSize(BZNativeVideoView.this.videoWidth, BZNativeVideoView.this.videoHeight);
            }
        }, !BZDeviceUtils.hardDecoderEnable());
        this.videoPath = videoPath;
        BZLogUtil.d(TAG, "setDataSource videoPath=" + videoPath);
    }

    public int getVideoWidth() {
        return videoWidth;
    }

    public int getVideoHeight() {
        return videoHeight;
    }

    public void prepare() {
        isFirstRender = true;
        calcViewport();
    }

    /**
     * @return pts ms
     */
    public long videoPlayerOnDrawFrame() {
        lastDrawTime += 1000 / videoFps;
        lastDrawTime = VideoPlayer.onDrawFrame(videoPlayerHandle, lastDrawTime);
        return lastDrawTime;
    }


    public String getVideoPath() {
        return videoPath;
    }

    public void setPlayLoop(final boolean playLoop) {
        this.playLoop = playLoop;
    }

    public void start() {
        if (continuouslyRender && getRenderMode() != RENDERMODE_CONTINUOUSLY)
            setRenderMode(RENDERMODE_CONTINUOUSLY);
        VideoPlayer.pause(videoPlayerHandle, false);
    }

    public boolean isPlaying() {
        return VideoPlayer.isPlaying(videoPlayerHandle);
    }

    public boolean isPause() {
        return !isPlaying();
    }

    public int getVideoRotate() {
        return videoRotate;
    }

    public void pause() {
        if (continuouslyRender && getRenderMode() != RENDERMODE_WHEN_DIRTY)
            setRenderMode(RENDERMODE_WHEN_DIRTY);
        VideoPlayer.pause(videoPlayerHandle, true);
    }


    public void release() {
        if (videoPlayerHandle != 0) {
            VideoPlayer.release(videoPlayerHandle);
            videoPlayerHandle = 0;
        }
    }

    public long getVideoDuration() {
        return videoDuration;
    }

    public void seek(final float progress) {
        VideoPlayer.seek(videoPlayerHandle, (long) (progress * videoDuration));
        requestRender();
    }

    public void setSpeed(float speed) {
        this.speed = speed;
    }

    public void setNeedFadeShow(boolean needFadeShow) {
        this.needFadeShow = needFadeShow;
    }


    public void setAutoStartPlay(boolean autoStartPlay) {
        this.autoStartPlay = autoStartPlay;
    }

    public void setOnCompletionListener(OnCompletionListener onCompletionListener) {
        this.onCompletionListener = onCompletionListener;
    }

    public void setOnProgressChangedListener(OnProgressChangedListener onProgressChangedListener) {
        this.onProgressChangedListener = onProgressChangedListener;
    }

    public void setOnStartRenderListener(OnStartRenderListener onStartRenderListener) {
        this.onStartRenderListener = onStartRenderListener;
    }

    public void setVolume(float volume) {
        this.volume = volume;
        VideoPlayer.setVolume(videoPlayerHandle, volume);
    }

    public float getVideoFps() {
        return videoFps;
    }

    public boolean isContinuouslyRender() {
        return continuouslyRender;
    }

    /**
     * @param continuouslyRender 做特效的时候要保证流畅度,需要用这个参数
     */
    public void setContinuouslyRender(boolean continuouslyRender) {
        this.continuouslyRender = continuouslyRender;
    }

    @Override
    public void setFlip(final boolean needFlipHorizontal, final boolean needFlipVertical) {
        queueEvent(() -> {
            int videoRotate = getVideoRotate();
//            if ((videoRotate == 90 || videoRotate == 270) && needFlipHorizontal) {
//                ExternalTextureProgram.setFlip(externalTextureProgramHandle, false, false);
//            } else {
//                ExternalTextureProgram.setFlip(externalTextureProgramHandle, needFlipHorizontal, needFlipVertical);
//            }
        });
        super.setFlip(needFlipHorizontal, needFlipVertical);
    }

    public interface OnCompletionListener {
        void onCompletion(boolean isCompletion);
    }

    public interface OnStartRenderListener {
        void onStartRender();
    }

    public interface OnProgressChangedListener {
        void onProgressChanged(float progress, int mediaItemIndex);
    }
}
