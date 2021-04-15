package com.luoye.bzmedia.widget;

import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glViewport;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.view.Surface;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.bzcommon.utils.BZFileUtils;
import com.bzcommon.utils.BZLogUtil;
import com.bzcommon.utils.BZStringUtils;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.MediaItem;
import com.google.android.exoplayer2.PlaybackParameters;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.analytics.AnalyticsListener;
import com.google.android.exoplayer2.decoder.DecoderReuseEvaluation;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.video.VideoFrameMetadataListener;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.SoundEffect;
import com.luoye.bzmedia.bean.VideoItem;
import com.luoye.bzmedia.opengl.CropTextureUtil;
import com.luoye.bzmedia.opengl.ExternalTextureProgram;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by bookzhan on 2017-09-29 09:47.
 * Description:
 */
public class BZVideoView extends BZBaseGLSurfaceView {
    private static final String TAG = "bz_BZVideoView";
    private static final int MSG_WHAT_UPDATE_PROGRESS = 0;
    private static final int MSG_WHAT_UPDATE_SURFACE = 1;
    private final DefaultTrackSelector trackSelector;
    private int videoRotate;
    private String videoPath;
    private OnPlayerStateChangeListener onPlayerStateChangeListener;
    private OnProgressChangedListener onProgressChangedListener;
    private long frameStartTimeMs;
    private float volume = 1;

    private boolean playLoop = false;
    private long externalTextureProgramHandle = 0;
    private SurfaceTexture surfaceTexture = null;
    private Surface mSurface = null;
    private int onFrameAvailableCount = 0;
    private boolean onFrameAvailable = false;
    private boolean isFirstRender = true;
    private boolean autoStartPlay = true;
    private OnStartRenderListener onStartRenderListener = null;
    private boolean needFadeShow = true;
    private float videoFps = 30;
    private int videoWidth = -1;
    private int videoHeight = -1;
    private boolean continuouslyRender = true;
    private float speed = 1;
    private final SimpleExoPlayer exoPlayer;
    private final SimpleExoPlayer audioPlayer;
    private final MessageHandler messageHandler;
    private long currentPosition = 0;
    private int currentMediaItemIndex = 0;
    private long totalDuration = 0;
    private long totalPosition = 0;
    private List<MediaItem> mediaItemList = new ArrayList<>();
    private HashMap<String, String> videoAudioPathMap = new HashMap<>();
    private FrameBufferUtil frameBufferUtil_1;
    private FrameBufferUtil frameBufferUtil_2;
    private FrameBufferUtil codecFrameBufferUtil;
    private int frameBufferIndex = 0;
    private int codecWidth = -1;
    private int codecHeight = -1;
    private int crop_right = -1;
    private int crop_bottom = -1;
    private long cropTextureHandel = 0;
    private SoundEffect mSoundEffect;


    public BZVideoView(Context context) {
        this(context, null);
    }

    public BZVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        needFlipVertical = false;
        messageHandler = new MessageHandler(new WeakReference<>(this));
        trackSelector = new DefaultTrackSelector(context);
        exoPlayer = new SimpleExoPlayer.Builder(context).setTrackSelector(trackSelector).build();
//        exoPlayer.setSeekParameters(SeekParameters.CLOSEST_SYNC);

        audioPlayer = new SimpleExoPlayer.Builder(context).build();
//        audioPlayer.setSeekParameters(SeekParameters.CLOSEST_SYNC);
        exoPlayer.addAnalyticsListener(new AnalyticsListener() {
            @Override
            public void onVideoInputFormatChanged(EventTime eventTime, Format format, @Nullable DecoderReuseEvaluation decoderReuseEvaluation) {
                BZLogUtil.d(TAG, "onVideoInputFormatChanged rotationDegrees=" + format.rotationDegrees + " frameRate=" + format.frameRate + " format=" + format.toString() + " metadata=" + format.metadata);
                if (null != decoderReuseEvaluation) {
                    BZLogUtil.d(TAG, "DecoderReuseEvaluation" + decoderReuseEvaluation.newFormat.rotationDegrees + " frameRate=" + decoderReuseEvaluation.newFormat.frameRate + " format=" + decoderReuseEvaluation.newFormat.toString());
                }
                videoRotate = format.rotationDegrees;
                videoWidth = format.width;
                videoHeight = format.height;
                videoFps = format.frameRate;
                if (videoRotate == 90 || videoRotate == 270) {
                    int temp = videoWidth;
                    videoWidth = videoHeight;
                    videoHeight = temp;
                }
                queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        ExternalTextureProgram.setVideoRotation(externalTextureProgramHandle, videoRotate);
                    }
                });
            }

            @Override
            public void onRenderedFirstFrame(EventTime eventTime, Object output, long renderTimeMs) {
                BZLogUtil.d(TAG, "onRenderedFirstFrame renderTimeMs=" + renderTimeMs);
                setInputSize(videoWidth, videoHeight);
            }

            @Override
            public void onTimelineChanged(EventTime eventTime, int reason) {
                BZLogUtil.d(TAG, "onTimelineChanged reason=" + reason);
                if (reason == Player.TIMELINE_CHANGE_REASON_PLAYLIST_CHANGED) {
                    BZLogUtil.d(TAG, "TIMELINE_CHANGE_REASON_PLAYLIST_CHANGED");
                    mediaItemList.clear();
                    for (int i = 0; i < exoPlayer.getMediaItemCount(); i++) {
                        MediaItem mediaItemAt = exoPlayer.getMediaItemAt(i);
                        mediaItemList.add(mediaItemAt);
                    }
                    if (speed > 3 && audioPlayer.getMediaItemCount() > 0) {
                        audioPlayer.clearMediaItems();
                        audioPlayer.setMediaItems(mediaItemList);
                        audioPlayer.seekTo(exoPlayer.getCurrentWindowIndex(), exoPlayer.getCurrentPosition());
                    }
                }
            }

            @Override
            public void onPlaybackStateChanged(EventTime eventTime, int state) {
                BZLogUtil.d(TAG, "onPlaybackStateChanged state=" + state);
                if (state == Player.STATE_ENDED) {
                    if (null != onPlayerStateChangeListener) {
                        onPlayerStateChangeListener.onPLayerEnd(true);
                    }
                    if (!playLoop) {
                        pause();
                    }
                }
            }

            @Override
            public void onPositionDiscontinuity(@NonNull EventTime eventTime, @NonNull Player.PositionInfo oldPosition, @NonNull Player.PositionInfo newPosition, int reason) {
                if (null != onPlayerStateChangeListener && newPosition.windowIndex == 0 && oldPosition.windowIndex == getVideoItemCount() - 1) {
                    onPlayerStateChangeListener.onRestart();
                }
            }
        });
        exoPlayer.setVideoFrameMetadataListener(new VideoFrameMetadataListener() {
            @Override
            public void onVideoFrameAboutToBeRendered(long presentationTimeUs, long releaseTimeNs,
                                                      @NonNull Format format, @Nullable MediaFormat mediaFormat) {
                codecWidth = -1;
                codecHeight = -1;
                crop_right = -1;
                crop_bottom = -1;
                if (null != mediaFormat) {
//                    BZLogUtil.d(TAG, "mediaFormat=" + mediaFormat);
                    codecWidth = mediaFormat.getInteger("width");
                    codecHeight = mediaFormat.getInteger("height");
                    crop_right = mediaFormat.getInteger("crop-right");
                    crop_bottom = mediaFormat.getInteger("crop-bottom");
                    if (videoRotate == 90 || videoRotate == 270) {
                        int temp = codecWidth;
                        codecWidth = codecHeight;
                        codecHeight = temp;
                        temp = crop_right;
                        crop_right = crop_bottom;
                        crop_bottom = temp;
                    }
                }
            }
        });
    }

    public SimpleExoPlayer getExoPlayer() {
        return exoPlayer;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        super.onSurfaceCreated(gl, config);
        externalTextureProgramHandle = ExternalTextureProgram.initNative(true, needFadeShow);
        int textureID = ExternalTextureProgram.initGlResource(externalTextureProgramHandle);
        ExternalTextureProgram.setVideoRotation(externalTextureProgramHandle, videoRotate);

        surfaceTexture = new SurfaceTexture(textureID);
        mSurface = new Surface(surfaceTexture);
        onFrameAvailableCount = 0;
        onFrameAvailable = false;
        surfaceTexture.setOnFrameAvailableListener(surfaceTexture -> {
            onFrameAvailableCount++;
            onFrameAvailable = true;
            requestRender();
        });
        messageHandler.sendEmptyMessage(MSG_WHAT_UPDATE_SURFACE);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        super.onSurfaceChanged(gl, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
        if (null == surfaceTexture || null == mSurface || !onFrameAvailable || videoWidth <= 0 || videoHeight <= 0) {
            return;
        }
        messageHandler.sendEmptyMessage(MSG_WHAT_UPDATE_PROGRESS);
        updateTexImage();

        long videoTime = 0;
        int lastTextureId = 0;
        FrameBufferUtil frameBuffer;
        int videoWidth = crop_right > 0 ? crop_right : this.videoWidth;
        int videoHeight = crop_bottom > 0 ? crop_bottom : this.videoHeight;
        if (codecWidth > 0 && codecHeight > 0 && (codecWidth != videoWidth || codecHeight != videoHeight)) {
            //need crop
            if (null == codecFrameBufferUtil || codecWidth != codecFrameBufferUtil.getWidth()
                    || codecHeight != codecFrameBufferUtil.getHeight()) {
                if (null != codecFrameBufferUtil) {
                    codecFrameBufferUtil.release();
                }
                codecFrameBufferUtil = new FrameBufferUtil(codecWidth, codecHeight);
            }
            codecFrameBufferUtil.bindFrameBuffer();
            glViewport(0, 0, codecFrameBufferUtil.getWidth(), codecFrameBufferUtil.getHeight());
            glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
            videoTime = videoPlayerOnDrawFrame();
            codecFrameBufferUtil.unbindFrameBuffer();
            lastTextureId = codecFrameBufferUtil.getFrameBufferTextureID();
            if (cropTextureHandel == 0) {
                cropTextureHandel = CropTextureUtil.initCropTexture();
            }
            lastTextureId = CropTextureUtil.cropTexture(cropTextureHandel, lastTextureId, codecFrameBufferUtil.getWidth(), codecFrameBufferUtil.getHeight(), 0, codecHeight - videoHeight, videoWidth, videoHeight);
        } else {
            frameBuffer = getNextFrameBuffer(mDrawViewPort.width, mDrawViewPort.height);
            //Convert to normal texture
            glViewport(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
            glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
            frameBuffer.bindFrameBuffer();
            videoTime = videoPlayerOnDrawFrame();
            frameBuffer.unbindFrameBuffer();
            lastTextureId = frameBuffer.getFrameBufferTextureID();
        }
        if (null == baseProgram) {
            baseProgram = new BaseProgram(0, needFlipHorizontal, needFlipVertical);
        }
        if (null == srcProgram) {
            srcProgram = new BaseProgram(0, false, false);
        }
        if (needFlipHorizontal || needFlipVertical) {
            frameBuffer = getNextFrameBuffer(mDrawViewPort.width, mDrawViewPort.height);
            glViewport(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
            glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
            frameBuffer.bindFrameBuffer();
            baseProgram.draw(lastTextureId);
            frameBuffer.unbindFrameBuffer();
            lastTextureId = frameBuffer.getFrameBufferTextureID();
        }

        if (null != onDrawFrameListener) {
            //通过调用videoPlayerOnDrawFrame完成最终的绘制
            onDrawFrameListener.onDrawFrame(lastTextureId, videoTime);
        } else {
            glClearColor(bzClearColor.r, bzClearColor.g, bzClearColor.b, bzClearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
            if (null != mDrawViewPort)
                glViewport(mDrawViewPort.x, mDrawViewPort.y, mDrawViewPort.width, mDrawViewPort.height);
            srcProgram.draw(lastTextureId);
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

    protected FrameBufferUtil getNextFrameBuffer(int width, int height) {
        //Double buffer exchange
        if (null == frameBufferUtil_1 || frameBufferUtil_1.getWidth() != width
                || frameBufferUtil_1.getHeight() != height) {
            frameBufferUtil_1 = new FrameBufferUtil(width, height);
        }
        if (null == frameBufferUtil_2 || frameBufferUtil_2.getWidth() != width
                || frameBufferUtil_2.getHeight() != height) {
            frameBufferUtil_2 = new FrameBufferUtil(width, height);
        }
        frameBufferIndex++;
        return frameBufferIndex % 2 == 1 ? frameBufferUtil_1 : frameBufferUtil_2;
    }

    private void updateTexImage() {
        if (onFrameAvailableCount <= 0) {
            return;
        }
        try {
            if (null != surfaceTexture)
                surfaceTexture.updateTexImage();
        } catch (Throwable e) {
            BZLogUtil.e(TAG, e);
        }
        onFrameAvailableCount--;
    }

    @Override
    public void onPause() {
        if (null != exoPlayer) {
            exoPlayer.clearVideoSurface();
        }
        if (null != mSurface) {
            mSurface.release();
            mSurface = null;
        }
        if (null != surfaceTexture) {
            surfaceTexture.release();
            surfaceTexture = null;
        }
        queueEvent(() -> {
            ExternalTextureProgram.releaseGlResource(externalTextureProgramHandle);
            externalTextureProgramHandle = 0;
            if (null != frameBufferUtil_1) {
                frameBufferUtil_1.release();
                frameBufferUtil_1 = null;
            }
            if (null != frameBufferUtil_2) {
                frameBufferUtil_2.release();
                frameBufferUtil_2 = null;
            }
            if (null != codecFrameBufferUtil) {
                codecFrameBufferUtil.release();
                codecFrameBufferUtil = null;
            }
            if (null != srcProgram) {
                srcProgram.release();
                srcProgram = null;
            }
            if (cropTextureHandel != 0) {
                CropTextureUtil.cropTextureRelease(cropTextureHandel);
                cropTextureHandel = 0;
            }
        });
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        requestRender();
    }


    public void setVideoItem(VideoItem videoItem) {
        if (null == videoItem || null == videoItem.getVideoPath()) {
            BZLogUtil.e(TAG, "null==videoItem||null ==videoItem.getVideoPath()");
            return;
        }
        if (!checkVideoPath(videoItem)) {
            BZLogUtil.e(TAG, "videoPath is not available");
            return;
        }
        exoPlayer.setMediaItem(getMediaItem(videoItem));
    }

    public void clearMediaItems() {
        exoPlayer.clearMediaItems();
    }

    public void removeVideoItemAt(int index) {
        exoPlayer.removeMediaItem(index);
    }

    public void setVideoItems(List<VideoItem> videoItems) {
        if (null == videoItems || videoItems.isEmpty()) {
            BZLogUtil.e(TAG, "null == mediaItems || mediaItems.isEmpty()");
            return;
        }
        List<MediaItem> mediaItemList = new ArrayList<>();
        for (VideoItem item : videoItems) {
            if (!checkVideoPath(item)) {
                BZLogUtil.e(TAG, "videoPath is not available");
                continue;
            }
            MediaItem mediaItem = getMediaItem(item);
            if (null == mediaItem) {
                continue;
            }
            mediaItemList.add(mediaItem);
        }
        if (mediaItemList.isEmpty()) {
            return;
        }
        exoPlayer.setMediaItems(mediaItemList);
    }

    public void setVideoItems(int index, List<VideoItem> videoItems) {
        if (null == videoItems || videoItems.isEmpty()) {
            BZLogUtil.e(TAG, "null == mediaItems || mediaItems.isEmpty()");
            return;
        }
        List<MediaItem> mediaItemList = new ArrayList<>();
        for (VideoItem item : videoItems) {
            if (!checkVideoPath(item)) {
                BZLogUtil.e(TAG, "videoPath is not available");
                continue;
            }
            MediaItem mediaItem = getMediaItem(item);
            if (null == mediaItem) {
                continue;
            }
            mediaItemList.add(mediaItem);
        }
        if (mediaItemList.isEmpty()) {
            return;
        }
        exoPlayer.setMediaItems(mediaItemList);
        exoPlayer.addMediaItems(index, mediaItemList);
    }

    public void moveVideoItem(int currentIndex, int newIndex) {
        exoPlayer.moveMediaItem(currentIndex, newIndex);
    }

    private MediaItem getMediaItem(VideoItem item) {
        if (null == item || null == item.getVideoPath()) {
            return null;
        }
        MediaItem.Builder builder = new MediaItem.Builder().setUri(item.getVideoPath()).setDuration(item.getDuration());
        if (item.getEndTime() - item.getStartTime() > 0) {
            builder.setDuration(item.getEndTime() - item.getStartTime());
            builder.setClipStartPositionMs(item.getStartTime());
            builder.setClipEndPositionMs(item.getEndTime());
        } else {
            if (item.getDuration() <= 0) {
                item.setStartTime(0);
                item.setEndTime(BZMedia.getMediaDuration(item.getVideoPath()));
            }
            builder.setDuration(item.getDuration());
        }
        return builder.build();
    }


    public int getVideoItemCount() {
        return exoPlayer.getMediaItemCount();
    }

    public MediaItem getMediaItemAt(int index) {
        return exoPlayer.getMediaItemAt(index);
    }

    public void addVideoItem(VideoItem videoItem) {
        if (null == videoItem || null == videoItem.getVideoPath()) {
            BZLogUtil.e(TAG, "null==videoItem||null ==videoItem.getVideoPath()");
            return;
        }
        if (!checkVideoPath(videoItem)) {
            BZLogUtil.e(TAG, "videoPath is not available");
            return;
        }
        exoPlayer.addMediaItem(getMediaItem(videoItem));
    }

    public void addVideoItem(int index, VideoItem videoItem) {
        if (null == videoItem || null == videoItem.getVideoPath()) {
            BZLogUtil.e(TAG, "null==videoItem||null ==videoItem.getVideoPath()");
            return;
        }
        if (!checkVideoPath(videoItem)) {
            BZLogUtil.e(TAG, "videoPath is not available");
            return;
        }
        exoPlayer.addMediaItem(index, getMediaItem(videoItem));
    }

    public int getCurrentVideoItemIndex() {
        return currentMediaItemIndex;
    }

    private static boolean checkVideoPath(VideoItem videoItem) {
        if (null == videoItem || null == videoItem.getVideoPath()) {
            return false;
        }
        BZLogUtil.d(TAG, "videoPath=" + videoItem.getVideoPath());
        if (videoItem.getVideoPath().startsWith("/")) {
            return checkFile(videoItem.getVideoPath());
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
        if (!checkFile(videoPath)) {
            BZLogUtil.e(TAG, "videoPath is not available");
            return;
        }
        if (exoPlayer.isPlaying()) {
            exoPlayer.pause();
            exoPlayer.stop();
        }
        this.videoPath = videoPath;
        exoPlayer.clearMediaItems();
        exoPlayer.setMediaItem(getMediaItem(new VideoItem(videoPath, -1, -1)));
        BZLogUtil.d(TAG, "setDataSource videoPath=" + videoPath);
    }

    public void setSoundEffect(SoundEffect soundEffect) {
        if (this.mSoundEffect == soundEffect) {
            return;
        }
        mSoundEffect = soundEffect;
        if (null == exoPlayer) {
            return;
        }
        if (null != mSoundEffect) {
            exoPlayer.setPlaybackParameters(new PlaybackParameters(soundEffect.getSpeed(), soundEffect.getPitch()));
        } else {
            exoPlayer.setPlaybackParameters(PlaybackParameters.DEFAULT);
        }
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
        try {
            exoPlayer.setPlayWhenReady(autoStartPlay);
            exoPlayer.setRepeatMode(playLoop ? Player.REPEAT_MODE_ALL : Player.REPEAT_MODE_OFF);

            exoPlayer.setPlaybackParameters(new PlaybackParameters(speed));
            exoPlayer.setVolume(volume);
            exoPlayer.prepare();
        } catch (Exception e) {
            BZLogUtil.e(TAG, e);
        }
    }

    /**
     * @return pts ms
     */
    private long videoPlayerOnDrawFrame() {
        ExternalTextureProgram.onDrawFrame(externalTextureProgramHandle);
        return totalPosition;
    }

    public long getCurrentPosition() {
        return currentPosition;
    }

    public long getTotalPosition() {
        return totalPosition;
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
        if (!exoPlayer.isPlaying())
            exoPlayer.play();
        if (!audioPlayer.isPlaying() && audioPlayer.getMediaItemCount() > 0)
            audioPlayer.play();
    }

    public boolean isPlaying() {
        return exoPlayer.isPlaying();
    }

    public int getVideoRotate() {
        return videoRotate;
    }

    public void pause() {
        if (continuouslyRender && getRenderMode() != RENDERMODE_WHEN_DIRTY)
            setRenderMode(RENDERMODE_WHEN_DIRTY);
        if (exoPlayer.isPlaying())
            exoPlayer.pause();

        if (audioPlayer.isPlaying())
            audioPlayer.pause();
    }

    public boolean isPause() {
        return !exoPlayer.isPlaying();
    }

    public void release() {
        if (exoPlayer.isPlaying()) {
            exoPlayer.pause();
            exoPlayer.stop();
        }
        if (audioPlayer.isPlaying()) {
            audioPlayer.pause();
            audioPlayer.stop();
        }
        audioPlayer.release();
        exoPlayer.release();

        for (Map.Entry<String, String> stringStringEntry : videoAudioPathMap.entrySet()) {
            BZFileUtils.deleteFile(stringStringEntry.getValue());
        }
    }

    public long getCurrentVideoDuration() {
        return exoPlayer.getDuration();
    }

    public long getTotalVideoDuration() {
        return totalDuration;
    }

    public void seek(final float totalProgress) {
        int targetIndex = 0;
        float currentPosition = totalProgress * totalDuration;
        long targetDuration = 0;
        long lastVideoTotalDuration = 0;
        for (int i = 0; i < exoPlayer.getMediaItemCount(); i++) {
            MediaItem mediaItemAt = exoPlayer.getMediaItemAt(i);
            if (null == mediaItemAt.playbackProperties) {
                continue;
            }
            long duration = mediaItemAt.playbackProperties.duration;
            duration = (long) (duration / speed);
            targetDuration += duration;

            if (targetDuration >= currentPosition) {
                targetIndex = i;
                break;
            }
            lastVideoTotalDuration += duration;
        }
        long positionMs = (long) ((currentPosition - lastVideoTotalDuration) * speed);
        BZLogUtil.d(TAG, "seek targetIndex=" + targetIndex + " positionMs=" + positionMs);
        exoPlayer.seekTo(targetIndex, positionMs);
        if (audioPlayer.getMediaItemCount() > 0) {
            audioPlayer.seekTo(targetIndex, positionMs);
        }
        requestRender();
    }

    public float getSpeed() {
        return speed;
    }

    public void setSpeed(float speed) {
        BZLogUtil.d(TAG, "setSpeed speed=" + speed);
        this.speed = speed;
        audioPlayer.stop(true);
        if (speed > 3) {
            trackSelector.setParameters(trackSelector.buildUponParameters().setRendererDisabled(
                    C.TRACK_TYPE_AUDIO, true));
            //No audio will be played if it is greater than 10 times
            if (speed <= 10) {
                for (MediaItem mediaItem : mediaItemList) {
                    if (null == mediaItem.playbackProperties) {
                        continue;
                    }
                    String path = mediaItem.playbackProperties.uri.getPath();
                    BZLogUtil.d(TAG, "video path=" + path);
                    if (!videoAudioPathMap.containsKey(path)) {
                        String audioPath = getContext().getFilesDir().getAbsolutePath() + "/audio_" + System.nanoTime() + ".m4a";
                        int ret = BZMedia.separateAudioStream(path, audioPath);
                        if (ret >= 0) {
                            videoAudioPathMap.put(path, audioPath);
                            //Replace with audio
                            audioPlayer.addMediaItem(mediaItem.buildUpon().setUri(audioPath).build());
                        } else {
                            audioPlayer.addMediaItem(mediaItem);
                        }
                    }
                }
                audioPlayer.setPlayWhenReady(autoStartPlay);
                audioPlayer.setRepeatMode(playLoop ? Player.REPEAT_MODE_ALL : Player.REPEAT_MODE_OFF);

                audioPlayer.setPlaybackParameters(new PlaybackParameters(speed));
                audioPlayer.setVolume(volume);

                audioPlayer.prepare();
                audioPlayer.seekTo(exoPlayer.getCurrentWindowIndex(), exoPlayer.getCurrentPosition());
                audioPlayer.play();
            }
        } else {
            trackSelector.setParameters(trackSelector.buildUponParameters().setRendererDisabled(
                    C.TRACK_TYPE_AUDIO, false));
        }
        exoPlayer.setPlaybackParameters(new PlaybackParameters(speed));
    }

    public void setNeedFadeShow(boolean needFadeShow) {
        this.needFadeShow = needFadeShow;
    }


    public void setAutoStartPlay(boolean autoStartPlay) {
        this.autoStartPlay = autoStartPlay;
    }

    public void setOnCompletionListener(OnPlayerStateChangeListener onPlayerStateChangeListener) {
        this.onPlayerStateChangeListener = onPlayerStateChangeListener;
    }

    public void setOnProgressChangedListener(OnProgressChangedListener onProgressChangedListener) {
        this.onProgressChangedListener = onProgressChangedListener;
    }

    public void setOnStartRenderListener(OnStartRenderListener onStartRenderListener) {
        this.onStartRenderListener = onStartRenderListener;
    }

    public void setVolume(float volume) {
        this.volume = volume;
        exoPlayer.setVolume(volume);
        audioPlayer.setVolume(volume);
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

    public interface OnPlayerStateChangeListener {
        void onPLayerEnd(boolean isEnd);

        void onRestart();
    }

    public interface OnStartRenderListener {
        void onStartRender();
    }

    public interface OnProgressChangedListener {
        void onProgressChanged(float currentProgress, int currentMediaItemIndex, float totalProgress);
    }

    private static class MessageHandler extends Handler {
        private WeakReference<BZVideoView> bzVideoViewWeakReference;

        private MessageHandler(WeakReference<BZVideoView> bzVideoViewWeakReference) {
            this.bzVideoViewWeakReference = bzVideoViewWeakReference;
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            BZVideoView bzVideoView = bzVideoViewWeakReference.get();
            if (null == bzVideoView) return;
            switch (msg.what) {
                case MSG_WHAT_UPDATE_PROGRESS:
                    long currentPosition = (long) (bzVideoView.exoPlayer.getCurrentPosition() / bzVideoView.speed);
                    bzVideoView.currentPosition = currentPosition;
                    bzVideoView.currentMediaItemIndex = bzVideoView.exoPlayer.getCurrentWindowIndex();

                    long totalDuration = 0;
                    long lastVideoTotalDuration = 0;
                    for (int i = 0; i < bzVideoView.exoPlayer.getMediaItemCount(); i++) {
                        MediaItem mediaItemAt = bzVideoView.exoPlayer.getMediaItemAt(i);
                        if (null == mediaItemAt.playbackProperties) {
                            continue;
                        }
                        long duration = mediaItemAt.playbackProperties.duration;
                        duration = (long) (duration / bzVideoView.speed);
                        totalDuration += duration;
                        if (i < bzVideoView.currentMediaItemIndex) {
                            lastVideoTotalDuration += duration;
                        }
                    }
                    bzVideoView.totalDuration = totalDuration;
                    bzVideoView.totalPosition = lastVideoTotalDuration + currentPosition;
                    if (null != bzVideoView.onProgressChangedListener) {
                        float currentProgress = currentPosition * 1.0f / bzVideoView.exoPlayer.getDuration();
                        if (currentProgress < 0) currentProgress = 0;
                        if (currentProgress > 1) currentProgress = 1;

                        float totalProgress = (lastVideoTotalDuration + currentPosition) * 1.0f / totalDuration;
                        if (totalProgress < 0) totalProgress = 0;
                        if (totalProgress > 1) totalProgress = 1;
                        bzVideoView.onProgressChangedListener.onProgressChanged(currentProgress, bzVideoView.currentMediaItemIndex, totalProgress);
                    }
                    break;
                case MSG_WHAT_UPDATE_SURFACE:
                    bzVideoView.exoPlayer.setVideoSurface(bzVideoView.mSurface);
                    break;

            }
        }
    }
}
