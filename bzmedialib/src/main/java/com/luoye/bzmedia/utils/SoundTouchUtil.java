package com.luoye.bzmedia.utils;

/**
 * Created by bookzhan on 2022-04-02 22:36.
 * description:
 */
public class SoundTouchUtil {
    public native static String getVersionString();

    /**
     * @param tempo -50 .. +100 %
     */
    private native void setTempo(long handle, float tempo);

    /**
     * @param pitch Original pitch = 1.0, smaller values represent lower pitches, larger values higher pitch.
     *              (0-8)
     */
    private native void setPitchSemiTones(long handle, float pitch);

    /**
     * @param speed Normal rate = 1.0, smaller values represent slower rate, larger faster rates.
     */
    private native void setSpeed(long handle, float speed);

    private native void setChannels(long handle, int numChannels);

    /// Sets sample rate.
    private native void setSampleRate(long handle, int sampleRate);

    private native int processFile(long handle, String inputFile, String outputFile);

    private native int putSamples(long handle, short[] sampleBuffer, int nSamples);

    private native short[] receiveSamples(long handle, int size);

    public native static String getErrorString();

    private native static long newInstance();

    private native void deleteInstance(long handle);

    long handle = 0;

    public SoundTouchUtil() {
        handle = newInstance();
    }

    public void close() {
        deleteInstance(handle);
        handle = 0;
    }

    public void setTempo(float tempo) {
        setTempo(handle, tempo);
    }

    /**
     * @param pitch (0 ..8)
     */
    public void setPitchSemiTones(float pitch) {
        setPitchSemiTones(handle, pitch);
    }

    public void setSpeed(float speed) {
        setSpeed(handle, speed);
    }

    public int processFile(String inputFile, String outputFile) {
        return processFile(handle, inputFile, outputFile);
    }

    public void setChannels(int numChannels) {
        setChannels(handle, numChannels);
    }

    public void setSampleRate(int sampleRate) {
        setSampleRate(handle, sampleRate);
    }

    public int putSamples(short[] sampleBuffer, int nSamples) {
        return putSamples(handle, sampleBuffer, nSamples);
    }

    public short[] receiveSamples(int size) {
        return receiveSamples(handle, size);
    }
}
