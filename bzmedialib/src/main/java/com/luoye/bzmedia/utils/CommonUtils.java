package com.luoye.bzmedia.utils;

/**
 * Created by bookzhan on 2022-04-03 15:12.
 * description:
 */
public class CommonUtils {
    public static short[] getShort(byte[] b) {
        if (b == null) {
            return null;
        }
        short[] s = new short[b.length / 2];
        for (int i = 0; i < s.length; i++) {
            s[i] = (short) (((b[i * 2 + 1] << 8) | b[i * 2] & 0xff));
        }
        return s;
    }

    public static byte[] shortToByteSmall(short[] buf) {
        byte[] bytes = new byte[buf.length * 2];
        for (int i = 0, j = 0; i < buf.length; i++, j += 2) {
            short s = buf[i];
            byte b1 = (byte) (s & 0xff);
            byte b0 = (byte) ((s >> 8) & 0xff);
            bytes[j] = b1;
            bytes[j + 1] = b0;
        }
        return bytes;
    }
}
