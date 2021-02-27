# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.


add_library( # Sets the name of the library.
        bzmedia

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ./jni/android_bzmedia.cpp
        ./jni/ffmpeg_base_info.cpp
        ./jni/OnActionListener.cpp
        ./jni/ffmpeg_audio_player.cpp

        ./common/BZLogUtil.cpp
        ./common/bz_time.cpp
        ./common/JvmManager.cpp
        ./common/GLUtil.cpp
        ./common/PngReader.cpp

        ./mediaedit/VideoUtil.cpp
        ./mediaedit/BackgroundMusicUtil.cpp
        ./mediaedit/MergeVideoUtil.cpp
        ./mediaedit/AdjustVideoSpeedUtil.cpp
        ./mediaedit/ClipVideoFrameToImage.cpp

        ./glutils/FrameBufferUtils.cpp;
        ./glutils/GLImageTextureUtil.cpp;
        ./glutils/GLMatrixUtils.cpp;
        ./glutils/MatrixVaryTools.cpp;
        ./glutils/CropTextureUtil.cpp;
        ./glutils/BZRenderEngine.cpp;
        ./glutils/VideoTextureManger.cpp;
        ./glutils/EGLContextUtil.cpp;
        ./glutils/TextureConvertYUVUtil.cpp;
        ./glutils/TextureUtil.cpp;

        ./glprogram/BaseProgram.cpp;
        ./glprogram/BaseMatrixProgram.cpp;
        ./glprogram/BaseParticleProgram.cpp;
        ./glprogram/CircleProgram.cpp;
        ./glprogram/ImageExternalProgram.cpp;
        ./glprogram/BaseYUVDrawProgram.cpp;
        ./glprogram/YUV420DrawProgram.cpp;
        ./glprogram/YUVNVDrawProgram.cpp;
        ./glprogram/AVFrameDrawProgram.cpp;
        ./glprogram/TextureYUVProgram.cpp;
        ./glprogram/GaussBlurProgram.cpp;

        ./player/PCMPlayerNative.cpp
        ./player/AudioPlayer.cpp
        ./recorder/VideoRecorder.cpp
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
configure_file(${Project_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libbzffmpeg.so ${Project_BINARY_DIR}/libbzffmpeg.so COPYONLY)
configure_file(${Project_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libbzffmpegcmd.so ${Project_BINARY_DIR}/libbzffmpegcmd.so COPYONLY)

#ffmpeg
add_library(bzffmpeg-lib
        SHARED
        IMPORTED)
set_target_properties(bzffmpeg-lib
        PROPERTIES IMPORTED_LOCATION
        libbzffmpeg.so)
#ffmpegcmd
add_library(bzffmpegcmd-lib
        SHARED
        IMPORTED)
set_target_properties(bzffmpegcmd-lib
        PROPERTIES IMPORTED_LOCATION
        libbzffmpegcmd.so)


find_library(log-lib log)
find_library(m-lib m)
find_library(z-lib z)
find_library(android-lib android)
find_library(EGL-lib EGL)
find_library(jnigraphics-lib jnigraphics)
find_library(GLES-lib GLESv2)


include_directories(
        ./
        ./include/
        ./common
        ./bean
        ./cmdutilt
        ./include
        ./include/libavcodec
        ./include/libavdevice
        ./include/libavfilter
        ./include/libavformat
        ./include/libavutil
        ./include/libpostproc
        ./include/libswresample
        ./include/libswscale
)
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        bzmedia

        # Links the target library to the log library
        # included in the NDK.
        bzffmpeg-lib bzffmpegcmd-lib ${log-lib} ${m-lib} ${z-lib} ${android-lib} ${EGL-lib} ${GLES-lib} ${jnigraphics-lib})