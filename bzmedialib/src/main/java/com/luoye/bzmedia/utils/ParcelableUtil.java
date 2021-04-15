package com.luoye.bzmedia.utils;


import android.os.Parcel;
import android.os.Parcelable;

/**
 * Created by bookzhan on 2021-12-04 17:44.
 * description:
 */
public class ParcelableUtil {

    public static Parcelable copy(Parcelable input) {
        Parcel parcel = null;
        try {
            parcel = Parcel.obtain();
            parcel.writeParcelable(input, 0);

            parcel.setDataPosition(0);
            return parcel.readParcelable(input.getClass().getClassLoader());
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (null != parcel) {
                parcel.recycle();
            }
        }
        return null;
    }
}
