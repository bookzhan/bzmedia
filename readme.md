### BZMedia 是一个Android 音视频库，包含视频录制，视频编解码，视频编辑，常见的OpenGL操作

> 项目介绍：这套音视频库包含了常见的音视频处理技术，服务于用户量高达到8亿的APP，里面使用的技术在市场上深度验证过，现在逐步整理出来然后开源，共大家学习交流


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
           maven { url 'https://dl.bintray.com/bookzhan/bzlib' }
       }
   }
   ```

2. 然后添加依赖

   ```
   implementation 'com.luoye.bzlib:bzmedia:1.0.5' 
   implementation 'com.luoye.bzlib:bzcommon:1.1.4'
   implementation 'com.luoye.bzlib:bzyuv:1.1.15'
   ```

3. 具体使用在工程里面都有Demo


#### 相关链接:

#### [FFmpeg 在Android下的编译](https://www.bzblog.online/wordpress/index.php/2020/05/26/ffmpeg-build-android/)

#### [Android FFmpeg命令行工具支持进度回调](https://www.bzblog.online/wordpress/index.php/2020/06/29/bzcmdffmpeg/)

#### [Android YUV转换，支持Camera1，Camera2](https://www.bzblog.online/wordpress/index.php/2020/05/25/androidlibyuv/)

