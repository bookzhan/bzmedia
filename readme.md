### BZMedia 是一个Android 音视频库，包含视频录制，视频编解码，视频编辑，常见的OpenGL操作

> 项目介绍：这套音视频库包含了常见的音视频处理技术，服务于用户量高达到8亿的APP，里面使用的技术在市场上深度验证过，现在逐步整理出来然后开源，供大家学习交流


##### 如果帮到你了请点一个star

#### 版本更新记录：

##### V1.0.5 ：

1. 修复录制无声视频, 时间戳的bug

##### V1.0.4 ：

1. 支持添加背景音乐
2. 支持完全替换掉背景音乐

##### V1.0.3 支持的主要功能是视频录制：

1. 支持的采集源有YUVI420,YV12,TEXTURE,Bitmap，详细的请参照Demo
2. 支持输入缩放，比如输入720的视频缩放成480的视频，默认会根据CPU性能自行缩放来保证录制的流畅性，如果不想采用默认缩放策略，可以把输入的款宽高和目标宽高设置成一致即可
3. 支持录制倍速视频



后续会把硬编码以及其它视频编辑功能陆续开源



使用方法：

1. 先添加工程目录下添加bzlib路径，如下

   ```
   allprojects {
       repositories {
           google()
           jcenter()
            maven { url "https://nexus.raoyunsoft.com/repository/maven-releases/" }
       }
   }
   ```

2. 然后添加依赖

   ```
       implementation 'com.guaishou.bzlib:bzyuv:1.1.17@aar'
       implementation 'com.guaishou.bzlib:bzcommon:1.1.16@aar'
       implementation 'com.guaishou.bzlib:bzmedia:1.0.8@aar'
   ```

3. 具体使用在工程里面都有Demo


#### 相关链接:

- [FFmpeg 在Android下的编译](https://blog.raoyunsoft.com/post/android-audio-video-development-ffmpeg-android-build)
- [Android FFmpeg命令行工具支持进度回调](https://blog.raoyunsoft.com/post/android-ffmpeg-cli-tool-progress-callback)
- [Android YUV转换，支持Camera1，Camera2](https://blog.raoyunsoft.com/post/android-yuv-cropping-camera1-camera2-support)
- [开源一个Android 音视频库，包含了视频录制，编解码，以及编辑，和常见的OpenGL，FFmpeg操作](https://blog.raoyunsoft.com/post/android-audio-video-library-record-encode-edit)
- [Dlib 在Android的编译](https://blog.raoyunsoft.com/post/dlib-android-build-guide)
- [libyuv在Android下的编译](https://blog.raoyunsoft.com/post/libyuv-android-build-guide)

