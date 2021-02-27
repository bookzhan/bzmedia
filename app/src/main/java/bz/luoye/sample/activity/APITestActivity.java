package bz.luoye.sample.activity;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.bzcommon.glutils.BZOpenGlUtils;
import com.bzcommon.glutils.BaseProgram;
import com.bzcommon.glutils.FrameBufferUtil;
import com.bzcommon.utils.BZDeviceUtils;
import com.bzcommon.utils.BZLogUtil;
import com.luoye.bzcamera.BZCameraView;
import com.luoye.bzmedia.BZMedia;
import com.luoye.bzmedia.bean.VideoTransCodeParams;

import java.util.Locale;

import bz.luoye.sample.R;
import bz.luoye.sample.utils.FilePathUtil;

import static android.opengl.GLES20.GL_BLEND;
import static android.opengl.GLES20.GL_COLOR_BUFFER_BIT;
import static android.opengl.GLES20.GL_ONE_MINUS_SRC_ALPHA;
import static android.opengl.GLES20.GL_SRC_ALPHA;
import static android.opengl.GLES20.glBlendFunc;
import static android.opengl.GLES20.glClear;
import static android.opengl.GLES20.glClearColor;
import static android.opengl.GLES20.glEnable;
import static android.opengl.GLES20.glViewport;

public class APITestActivity extends AppCompatActivity {
    private static final String TAG = "bz_APITestActivity";
    private int watermarkTextureId = -1;
    private FrameBufferUtil frameBufferUtil = null;
    private BaseProgram baseProgramSrc;
    private BaseProgram baseProgramWatermark;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_other_test);
    }

    public void addBackgroundMusic(View view) {
        String inputPath = FilePathUtil.getWorkDir() + "/VID_29.mp4";
        final String outputPath = FilePathUtil.getAVideoPath();
        String musicPath = FilePathUtil.getWorkDir() + "/mix_4.m4a";
        float srcMusicVolume = 1;
        float bgMusicVolume = 0.5f;
        int ret = BZMedia.addBackgroundMusic(inputPath, outputPath, musicPath, srcMusicVolume, bgMusicVolume, new BZMedia.OnActionListener() {
            @Override
            public void progress(float progress) {
                BZLogUtil.d(TAG, "progress=" + progress);
            }

            @Override
            public void fail() {
                BZLogUtil.e(TAG, "addBackgroundMusic fail");
            }

            @Override
            public void success() {
                BZLogUtil.d(TAG, "addBackgroundMusic success outputPath=" + outputPath);
            }
        });
        if (ret < 0) {
            BZLogUtil.e(TAG, "addBackgroundMusic fail ret=" + ret);
        }
    }

    public void replaceBackgroundMusic(View view) {
        String inputPath = FilePathUtil.getWorkDir() + "/VID_29.mp4";
        final String outputPath = FilePathUtil.getAVideoPath();
        String musicPath = FilePathUtil.getWorkDir() + "/mix_4.m4a";
        int ret = BZMedia.replaceBackgroundMusic(inputPath, musicPath, outputPath, new BZMedia.OnActionListener() {
            @Override
            public void progress(float progress) {
                BZLogUtil.d(TAG, "progress=" + progress);
            }

            @Override
            public void fail() {
                BZLogUtil.e(TAG, "replaceBackgroundMusic fail");
            }

            @Override
            public void success() {
                BZLogUtil.d(TAG, "replaceBackgroundMusic success outputPath=" + outputPath);
            }
        });
        if (ret < 0) {
            BZLogUtil.e(TAG, "replaceBackgroundMusic fail ret=" + ret);
        }
    }

    public void GetImageFromVideoActivity(View view) {
        startActivity(new Intent(this, GetImageFromVideoActivity.class));
    }


    public void clipAudio(View view) {
        String path = FilePathUtil.getWorkDir() + "/许多葵,蔡翊昇 - 千樽雪.mp3";
        BZMedia.clipAudio(path, FilePathUtil.getWorkDir() + "/out.mp3", 3000, 10000);
    }

    public void clipVideo(View view) {
        String inputPath = FilePathUtil.getWorkDir() + "/testvideo.mp4";
        BZMedia.clipVideo(inputPath, FilePathUtil.getAVideoPath(), 1000, 4500);
    }

    public void getMediaDuration(View view) {
//        String path = FilePathUtil.getWorkDir() + "/许多葵,蔡翊昇 - 千樽雪.mp3";
        String path = FilePathUtil.getWorkDir() + "/testvideo.mp4";
        long mediaDuration = BZMedia.getMediaDuration(path);
        BZLogUtil.d(TAG, "mediaDuration=" + mediaDuration);
    }

    public void startVideoTransCode(View view) {
        //BZMedia.stopVideoTransCode()停止转码
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                long startTime = System.currentTimeMillis();
                String inputPath = FilePathUtil.getWorkDir() + "/testvideo.mp4";
                String outputPath = FilePathUtil.getAVideoPath();
                VideoTransCodeParams videoTransCodeParams = new VideoTransCodeParams();
                videoTransCodeParams.setInputPath(inputPath);
                videoTransCodeParams.setOutputPath(outputPath);
                videoTransCodeParams.setDoWithVideo(true);
                videoTransCodeParams.setUserSoftDecode(!BZDeviceUtils.hardDecoderEnable());

//                videoTransCodeParams.setTargetWidth(480);
//                videoTransCodeParams.setTargetHeight(720);
//                videoTransCodeParams.setVideoRotate(90);
                //用来处理音频的,不建议使用
                videoTransCodeParams.setDoWithAudio(false);
                //视频总时间通过 BZMedia.getMediaDuration()获取
//                videoTransCodeParams.setStartTime(1803);//从什么时间开始截取
//                videoTransCodeParams.setEndTime(6859);//截取到什么时间
                videoTransCodeParams.setMaxWidth(240);//视频的最大宽度,通过这个参数来修改分辨率,建议这个值设置为屏幕的宽度,大了没有意义,还会影响后续流程的性能
//                videoTransCodeParams.setFrameRate(24);//建议使用24帧
//                videoTransCodeParams.setVideoRotate(90);//视频旋转的角度,会兼容视频本身就有旋转角度
                //控制是否回调纹理,如果没有特殊需求,不建议使用
                videoTransCodeParams.setNeedCallBackVideo(true);

                long videoTransCodeNativeHandle = BZMedia.initVideoTransCode();

                //添加文字和贴纸需要采用这种方式,实时添加可以根据时间戳大小,用Canvas来画到Bitmap上
                Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.ic_launcher);
                int ret = BZMedia.startVideoTransCode(videoTransCodeNativeHandle, videoTransCodeParams, new BZMedia.OnVideoTransCodeListener() {

                    @Override
                    public int onTextureCallBack(int textureId, int width, int height, long pts, long videoTime) {
                        BZLogUtil.d(TAG, "onTextureCallBack textureId=" + textureId + " width=" + width + " height=" + height + " pts=" + pts + " videoTime=" + videoTime);
                        if (null == baseProgramSrc) {
                            baseProgramSrc = new BaseProgram(false);
                        }
                        if (watermarkTextureId < 0) {
                            //Watermark picture
                            Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.ic_launcher);
                            watermarkTextureId = BZOpenGlUtils.loadTexture(bitmap);
                            bitmap.recycle();
                        }
                        if (null == frameBufferUtil) {
                            frameBufferUtil = new FrameBufferUtil(width, height);
                        }
                        if (null == baseProgramSrc) {
                            baseProgramSrc = new BaseProgram(false);
                        }
                        if (null == baseProgramWatermark) {
                            baseProgramWatermark = new BaseProgram(false);
                        }
                        frameBufferUtil.bindFrameBuffer();
                        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glViewport(0, 0, width, height);
                        baseProgramSrc.draw(textureId);
                        //Watermark area
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        //贴纸的位置需要在这里用坐标映射过来,要注意OpenGL的坐标与View坐标Y轴是反的
                        glViewport(width - 100, height - 100, 100, 100);
                        baseProgramWatermark.draw(watermarkTextureId);
                        frameBufferUtil.unbindFrameBuffer();
                        return frameBufferUtil.getFrameBufferTextureID();
                    }

                    @Override
                    public byte[] onPcmCallBack(byte[] pcmData) {
                        BZLogUtil.d(TAG, "onPcmCallBack=" + pcmData + " length=" + pcmData.length);
                        return pcmData;
                    }

                    @Override
                    public void videoTransCodeProgress(float progress) {
                        BZLogUtil.d(TAG, "videoTransCodeProgress progress=" + progress);
                    }

                    @Override
                    public void videoTransCodeFinish() {
                        BZLogUtil.d(TAG, "videoTransCodeFinish");
                        BZOpenGlUtils.deleteTexture(watermarkTextureId);
                        watermarkTextureId = -1;
                        if (null != baseProgramSrc) {
                            baseProgramSrc.release();
                            baseProgramSrc = null;
                        }
                        if (null != baseProgramWatermark) {
                            baseProgramWatermark.release();
                            baseProgramWatermark = null;
                        }
                        if (null != frameBufferUtil) {
                            frameBufferUtil.release();
                            frameBufferUtil = null;
                        }
                    }
                });

                if (ret < 0) {
                    BZLogUtil.d(TAG, "转码失败");
                }

                BZMedia.stopVideoTransCode(videoTransCodeNativeHandle);
                bitmap.recycle();
                BZLogUtil.d(TAG, "videoTransCode 耗时=" + (System.currentTimeMillis() - startTime));
            }
        });
        thread.setPriority(Thread.MAX_PRIORITY);
        thread.start();
    }
}
