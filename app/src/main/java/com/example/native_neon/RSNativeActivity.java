package com.example.native_neon;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.InputStream;

public class RSNativeActivity extends AppCompatActivity {
    private static final String TAG = "RSNativeActivity";
    private ImageView imgInput, imgOutput;
    private TextView tvTime;
    private Button btnRun;
    private Bitmap original;

    @Override
    protected void onCreate(Bundle s) {
        super.onCreate(s);
        setContentView(R.layout.activity_rs_native);

        imgInput = findViewById(R.id.imgInput);
        imgOutput = findViewById(R.id.imgOutput);
        tvTime = findViewById(R.id.tvTime);
        btnRun = findViewById(R.id.btnRun);

        // load bitmap from assets (immutable), create mutable copy ARGB_8888
        try {
            InputStream is = getAssets().open("test_4k.jpg");
            Bitmap tmp = BitmapFactory.decodeStream(is);
            is.close();
            // ensure ARGB_8888 mutable
            original = tmp.copy(Bitmap.Config.ARGB_8888, false); // keep immutable original for re-copy
            imgInput.setImageBitmap(original);
        } catch (Exception e) {
            e.printStackTrace();
        }

        btnRun.setOnClickListener(v -> runNative());
    }

    private void runNative() {
        // make a mutable copy to be processed in-place
        Bitmap bmp = original.copy(Bitmap.Config.ARGB_8888, true);

        long t0 = System.nanoTime();
        int res = NativeLib.grayScaleNeon(bmp);
        long t1 = System.nanoTime();

        double procMs = (t1 - t0) / 1_000_000.0;
        if (res != 0) {
            tvTime.setText("native failed: " + res);
            Log.e(TAG, "native failed: " + res);
        } else {
            tvTime.setText(String.format("nativeProc=%.2f ms", procMs));
            imgOutput.setImageBitmap(bmp);
            Log.i(TAG, "nativeProc= " + procMs + " ms");
        }
    }
}
