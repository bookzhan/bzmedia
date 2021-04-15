# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    add_definitions(-DDEBUG_TYPE)
endif()

aux_source_directory(./glprogram DIR_GLPROGRAM_SRCS)
aux_source_directory(./mediaedit DIR_MEDIAEDIT_SRCS)
aux_source_directory(./player DIR_PLAYER_SRCS)
aux_source_directory(./soundtouch DIR_SOUNDTOUCH_SRCS)

add_library( # Sets the name of the library.
        bzmedia

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ./jni/android_bzmedia.cpp
        ./jni/ffmpeg_base_info.cpp
        ./jni/OnActionListener.cpp
        ./jni/ffmpeg_audio_player.cpp
        ./jni/video_edit_sdk.cpp
        ./jni/jni_VideoFrameGetter.cpp
        ./jni/video_recorder_jni.cpp
        ./jni/android_gl_program.cpp
        ./jni/jni_BZRenderEngine.cpp;
        ./jni/video_player_jni.cpp;
        ./jni/soundtouch_jni.cpp;


        ./common/BZLogUtil.cpp
        ./common/bz_time.cpp
        ./common/JvmManager.cpp
        ./common/GLUtil.cpp
        ./common/PngReader.cpp

        ${DIR_MEDIAEDIT_SRCS}

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

        ${DIR_GLPROGRAM_SRCS}
        ${DIR_PLAYER_SRCS}
        ${DIR_SOUNDTOUCH_SRCS}

        ./recorder/VideoRecorder.cpp

        ./utils/AdjustConfigUtil.cpp
        ./utils/PcmDeque.cpp

        ./permission/base64.c
        ./permission/PermissionUtil.cpp
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
configure_file(${Project_SOURCE_DIR}/lib/${ANDROID_ABI}/libbzffmpeg.so ${Project_BINARY_DIR}/libbzffmpeg.so COPYONLY)
configure_file(${Project_SOURCE_DIR}/lib/${ANDROID_ABI}/libbzffmpegcmd.so ${Project_BINARY_DIR}/libbzffmpegcmd.so COPYONLY)

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
add_library(yuv-lib
        STATIC
        IMPORTED)
set_target_properties(yuv-lib
        PROPERTIES IMPORTED_LOCATION
        ${Project_SOURCE_DIR}/lib/${ANDROID_ABI}/libyuv_static.a)

find_library(log-lib log)
find_library(m-lib m)
find_library(z-lib z)
find_library(android-lib android)
find_library(EGL-lib EGL)
find_library(jnigraphics-lib jnigraphics)
find_library(GLES-lib GLESv2)
find_library(OpenSLES-lib OpenSLES)


include_directories(
        ./
        ./include/
        ./common
        ./bean
        ./cmdutilt
        ./include
        ./utils
        ./permission
        ./include/libavcodec
        ./include/libavdevice
        ./include/libavfilter
        ./include/libavformat
        ./include/libavutil
        ./include/libpostproc
        ./include/libswresample
        ./include/libswscale
        ./include/libyuv
        ./include/soundtouch
)
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        bzmedia

        # Links the target library to the log library
        # included in the NDK.
        bzffmpeg-lib bzffmpegcmd-lib yuv-lib ${log-lib} ${m-lib} ${z-lib} ${android-lib} ${EGL-lib} ${GLES-lib} ${jnigraphics-lib} ${OpenSLES-lib})