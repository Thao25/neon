package com.example.native_neon;

import android.graphics.Bitmap;

public class NativeLib {
    static {
        System.loadLibrary("native-lib");
    }

    // Trả về 0 nếu ok, negative nếu lỗi
    public static native int grayScaleNeon(Bitmap bmp);
}
