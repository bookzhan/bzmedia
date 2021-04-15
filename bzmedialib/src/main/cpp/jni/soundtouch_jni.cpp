////////////////////////////////////////////////////////////////////////////////
///
/// Example Interface class for SoundTouch native compilation
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
/// WWW           : http://www.surina.net
///
////////////////////////////////////////////////////////////////////////////////

#include <jni.h>
#include <android/log.h>
#include <stdexcept>
#include <string>
#include <BZLogUtil.h>

using namespace std;

#include "SoundTouch.h"
#include "soundtouch/WavFile.h"


// String for keeping possible c++ exception error messages. Notice that this isn't
// thread-safe but it's expected that exceptions are special situations that won't
// occur in several threads in parallel.
static string _errMsg = "";


#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#define BUFF_SIZE 4096


using namespace soundtouch;


// Set error message to return
static void _setErrmsg(const char *msg) {
    _errMsg = msg;
}

#if 0   // apparently following workaround not needed any more with concurrent Android SDKs
#ifdef _OPENMP

#include <pthread.h>
extern pthread_key_t gomp_tls_key;
static void * _p_gomp_tls = NULL;

/// Function to initialize threading for OpenMP.
///
/// This is a workaround for bug in Android NDK v10 regarding OpenMP: OpenMP works only if
/// called from the Android App main thread because in the main thread the gomp_tls storage is
/// properly set, however, Android does not properly initialize gomp_tls storage for other threads.
/// Thus if OpenMP routines are invoked from some other thread than the main thread,
/// the OpenMP routine will crash the application due to NULL pointer access on uninitialized storage.
///
/// This workaround stores the gomp_tls storage from main thread, and copies to other threads.
/// In order this to work, the Application main thread needws to call at least "getVersionString"
/// routine.
static int _init_threading(bool warn)
{
    void *ptr = pthread_getspecific(gomp_tls_key);
    BZLogUtil::logD("JNI thread-specific TLS storage %ld", (long)ptr);
    if (ptr == NULL)
    {
        BZLogUtil::logD("JNI set missing TLS storage to %ld", (long)_p_gomp_tls);
        pthread_setspecific(gomp_tls_key, _p_gomp_tls);
    }
    else
    {
        BZLogUtil::logD("JNI store this TLS storage");
        _p_gomp_tls = ptr;
    }
    // Where critical, show warning if storage still not properly initialized
    if ((warn) && (_p_gomp_tls == NULL))
    {
        _setErrmsg("Error - OpenMP threading not properly initialized: Call SoundTouch.getVersionString() from the App main thread!");
        return -1;
    }
    return 0;
}

#else
static int _init_threading(bool warn)
{
    // do nothing if not OpenMP build
    return 0;
}
#endif

#endif

// Processes the sound file
static void _processFile(SoundTouch *pSoundTouch, const char *inFileName, const char *outFileName) {
    int nSamples;
    int nChannels;
    int buffSizeSamples;
    SAMPLETYPE sampleBuffer[BUFF_SIZE];

    // open input file
    WavInFile inFile(inFileName);
    int sampleRate = inFile.getSampleRate();
    int bits = inFile.getNumBits();
    nChannels = inFile.getNumChannels();

    // create output file
    WavOutFile outFile(outFileName, sampleRate, bits, nChannels);

    pSoundTouch->setSampleRate(sampleRate);
    pSoundTouch->setChannels(nChannels);

    assert(nChannels > 0);
    buffSizeSamples = BUFF_SIZE / nChannels;

    // Process samples read from the input file
    while (inFile.eof() == 0) {
        int num;

        // Read a chunk of samples from the input file
        num = inFile.read(sampleBuffer, BUFF_SIZE);
        nSamples = num / nChannels;

        // Feed the samples into SoundTouch processor
        pSoundTouch->putSamples(sampleBuffer, nSamples);

        // Read ready samples from SoundTouch processor & write them output file.
        // NOTES:
        // - 'receiveSamples' doesn't necessarily return any samples at all
        //   during some rounds!
        // - On the other hand, during some round 'receiveSamples' may have more
        //   ready samples than would fit into 'sampleBuffer', and for this reason
        //   the 'receiveSamples' call is iterated for as many times as it
        //   outputs samples.
        do {
            nSamples = pSoundTouch->receiveSamples(sampleBuffer, buffSizeSamples);
            outFile.write(sampleBuffer, nSamples * nChannels);
        } while (nSamples != 0);
    }

    // Now the input file is processed, yet 'flush' few last samples that are
    // hiding in the SoundTouch's internal processing pipeline.
    pSoundTouch->flush();
    do {
        nSamples = pSoundTouch->receiveSamples(sampleBuffer, buffSizeSamples);
        outFile.write(sampleBuffer, nSamples * nChannels);
    } while (nSamples != 0);
}


extern "C" DLL_PUBLIC jstring
Java_com_luoye_bzmedia_utils_SoundTouchUtil_getVersionString(JNIEnv *env, jclass thiz) {
    const char *verStr;

    BZLogUtil::logD("JNI call SoundTouch.getVersionString");

    // Call example SoundTouch routine
    verStr = SoundTouch::getVersionString();

    // gomp_tls storage bug workaround - see comments in _init_threading() function!
    // update: apparently this is not needed any more with concurrent Android SDKs
    // _init_threading(false);

    int threads = 0;
#pragma omp parallel
    {
#pragma omp atomic
        threads++;
    }
    BZLogUtil::logD("JNI thread count %d", threads);

    // return version as string
    return env->NewStringUTF(verStr);
}



extern "C" DLL_PUBLIC jlong
Java_com_luoye_bzmedia_utils_SoundTouchUtil_newInstance(JNIEnv *env, jclass thiz) {
    return (jlong) (new SoundTouch());
}


extern "C" DLL_PUBLIC void
Java_com_luoye_bzmedia_utils_SoundTouchUtil_deleteInstance(JNIEnv *env, jobject thiz,
                                                           jlong handle) {
    if (handle == 0) {
        return;
    }
    auto *ptr = (SoundTouch *) handle;
    delete ptr;
}


extern "C" DLL_PUBLIC void
Java_com_luoye_bzmedia_utils_SoundTouchUtil_setTempo(JNIEnv *env, jobject thiz, jlong handle,
                                                     jfloat tempo) {
    if (handle == 0) {
        return;
    }
    auto *ptr = (SoundTouch *) handle;
    ptr->setTempo(tempo);
}


extern "C" DLL_PUBLIC void
Java_com_luoye_bzmedia_utils_SoundTouchUtil_setPitchSemiTones(JNIEnv *env, jobject thiz,
                                                              jlong handle,
                                                              jfloat pitch) {
    if (handle == 0) {
        return;
    }
    auto *ptr = (SoundTouch *) handle;
    ptr->setVirtualPitch(pitch);
}


extern "C" DLL_PUBLIC void
Java_com_luoye_bzmedia_utils_SoundTouchUtil_setSpeed(JNIEnv *env, jobject thiz, jlong handle,
                                                     jfloat speed) {
    if (handle == 0) {
        return;
    }
    auto *ptr = (SoundTouch *) handle;
    ptr->setRate(speed);
}


extern "C" DLL_PUBLIC jstring
Java_com_luoye_bzmedia_utils_SoundTouchUtil_getErrorString(JNIEnv *env, jclass thiz) {
    jstring result = env->NewStringUTF(_errMsg.c_str());
    _errMsg.clear();

    return result;
}


extern "C" DLL_PUBLIC int
Java_com_luoye_bzmedia_utils_SoundTouchUtil_processFile(JNIEnv *env, jobject thiz, jlong handle,
                                                        jstring jinputFile, jstring joutputFile) {
    if (handle == 0) {
        return -1;
    }
    auto *ptr = (SoundTouch *) handle;

    const char *inputFile = env->GetStringUTFChars(jinputFile, 0);
    const char *outputFile = env->GetStringUTFChars(joutputFile, 0);

    BZLogUtil::logD("JNI process file %s", inputFile);

    /// gomp_tls storage bug workaround - see comments in _init_threading() function!
    // update: apparently this is not needed any more with concurrent Android SDKs
    // if (_init_threading(true)) return -1;

    try {
        _processFile(ptr, inputFile, outputFile);
    }
    catch (const runtime_error &e) {
        const char *err = e.what();
        // An exception occurred during processing, return the error message
        BZLogUtil::logD("JNI exception in SoundTouch::processFile: %s", err);
        _setErrmsg(err);
        return -1;
    }


    env->ReleaseStringUTFChars(jinputFile, inputFile);
    env->ReleaseStringUTFChars(joutputFile, outputFile);

    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_utils_SoundTouchUtil_setChannels(JNIEnv *env, jobject thiz, jlong handle,
                                                        jint num_channels) {
    if (handle == 0) {
        return;
    }
    auto *ptr = (SoundTouch *) handle;
    ptr->setChannels(num_channels);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_luoye_bzmedia_utils_SoundTouchUtil_setSampleRate(JNIEnv *env, jobject thiz, jlong handle,
                                                          jint sample_rate) {
    if (handle == 0) {
        return;
    }
    auto *ptr = (SoundTouch *) handle;
    ptr->setSampleRate(sample_rate);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_luoye_bzmedia_utils_SoundTouchUtil_putSamples(JNIEnv *env, jobject thiz, jlong handle,
                                                       jshortArray sample_buffer, jint n_samples) {
    if (handle == 0) {
        return -1;
    }
    auto *ptr = (SoundTouch *) handle;
    jshort *input_samples = env->GetShortArrayElements(sample_buffer, nullptr);
    ptr->putSamples(input_samples, (uint) n_samples);
    env->ReleaseShortArrayElements(sample_buffer, input_samples, 0);
    return 0;
}
extern "C"
JNIEXPORT jshortArray JNICALL
Java_com_luoye_bzmedia_utils_SoundTouchUtil_receiveSamples(JNIEnv *env, jobject thiz,
                                                           jlong handle,jint size) {
    if (handle == 0) {
        return nullptr;
    }
    auto *ptr = (SoundTouch *) handle;
    SAMPLETYPE buffer[size];
    int nSamples = ptr->receiveSamples(buffer, size);
    if (nSamples <= 0) {
        return nullptr;
    }
    // 局部引用，创建一个short数组
    jshortArray receiveSamples = env->NewShortArray(nSamples);
    // 给short数组设置值
    env->SetShortArrayRegion(receiveSamples, 0, nSamples, buffer);
    return receiveSamples;
}