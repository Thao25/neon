#include <jni.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <stdint.h>
#include <string.h>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

#define LOG_TAG "native-neon"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/*
 * Trả về 0 nếu OK, negative nếu lỗi
 */
JNIEXPORT jint JNICALL
Java_com_example_hellorendering_NativeLib_grayScaleNeon(JNIEnv *env, jclass clazz, jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels = NULL;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) != ANDROID_BITMAP_RESULT_SUCCESS) {
        ALOGE("AndroidBitmap_getInfo failed: %d", ret);
        return -1;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888 && info.format != ANDROID_BITMAP_FORMAT_RGB_565) {
        // We expect RGBA_8888 (4 bytes per pixel). If not, return error.
        ALOGE("Unsupported bitmap format: %d", info.format);
        return -2;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) != ANDROID_BITMAP_RESULT_SUCCESS) {
        ALOGE("AndroidBitmap_lockPixels failed: %d", ret);
        return -3;
    }

    // process only RGBA_8888 (4 bytes per pixel)
    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        uint8_t *base = (uint8_t *) pixels;
        const int width = info.width;
        const int height = info.height;
        const int stride = info.stride; // bytes per row
        const int bpp = 4;

#ifdef __ARM_NEON
        // NEON path: process 8 pixels per loop using vld4/vst4
        for (int y = 0; y < height; y++) {
            uint8_t *row = base + y * stride;
            int x = 0;
            for (; x + 8 <= width; x += 8) {
                uint8x8x4_t pix = vld4_u8(row + x * bpp); // B,G,R,A deinterleaved

                // widen to 16-bit and compute acc = R*77 + G*150 + B*29
                uint16x8_t r16 = vmovl_u8(pix.val[2]);
                uint16x8_t g16 = vmovl_u8(pix.val[1]);
                uint16x8_t b16 = vmovl_u8(pix.val[0]);

                // multiply by constants (uint16 x uint16 -> uint32 for safe accumulation)
                // but we can do accum in u16 with careful shifts; simpler: use vmull_u8 + vaddl
                // compute using u16 accumulators:
                uint32x4_t sum_lo, sum_hi;
                // split to low/high 4 lanes
                uint16x4_t r_lo = vget_low_u16(r16);
                uint16x4_t r_hi = vget_high_u16(r16);
                uint16x4_t g_lo = vget_low_u16(g16);
                uint16x4_t g_hi = vget_high_u16(g16);
                uint16x4_t b_lo = vget_low_u16(b16);
                uint16x4_t b_hi = vget_high_u16(b16);

                // constants in 32-bit lanes
                const uint32x4_t cR = vdupq_n_u32(77);
                const uint32x4_t cG = vdupq_n_u32(150);
                const uint32x4_t cB = vdupq_n_u32(29);

                sum_lo = vmulq_u32(vmovl_u16(r_lo), cR);
                sum_lo = vmlaq_u32(sum_lo, vmovl_u16(g_lo), cG);
                sum_lo = vmlaq_u32(sum_lo, vmovl_u16(b_lo), cB);

                sum_hi = vmulq_u32(vmovl_u16(r_hi), cR);
                sum_hi = vmlaq_u32(sum_hi, vmovl_u16(g_hi), cG);
                sum_hi = vmlaq_u32(sum_hi, vmovl_u16(b_hi), cB);

                // shift right by 8 -> 0..255
                uint16x4_t gray_lo = vmovn_u32(vshrq_n_u32(sum_lo, 8));
                uint16x4_t gray_hi = vmovn_u32(vshrq_n_u32(sum_hi, 8));

                uint16x8_t gray_u16 = vcombine_u16(gray_lo, gray_hi);
                uint8x8_t gray_u8 = vmovn_u16(gray_u16);

                // build output: B=G=R=gray, A unchanged
                uint8x8x4_t out;
                out.val[0] = gray_u8;
                out.val[1] = gray_u8;
                out.val[2] = gray_u8;
                out.val[3] = pix.val[3]; // keep alpha

                vst4_u8(row + x * bpp, out);
            }

            // tail
            for (; x < width; x++) {
                uint8_t *px = row + x * bpp;
                uint8_t B = px[0];
                uint8_t G = px[1];
                uint8_t R = px[2];
                uint32_t yv = (R * 77u + G * 150u + B * 29u) >> 8;
                uint8_t yy = (uint8_t) yv;
                px[0] = yy; px[1] = yy; px[2] = yy;
                // alpha unchanged
            }
        }
#else
        // Fallback C loop (no NEON)
        for (int y = 0; y < height; y++) {
            uint8_t *row = base + y * stride;
            for (int x = 0; x < width; x++) {
                uint8_t *px = row + x * bpp;
                uint8_t B = px[0];
                uint8_t G = px[1];
                uint8_t R = px[2];
                uint32_t yv = (R * 77u + G * 150u + B * 29u) >> 8;
                uint8_t yy = (uint8_t) yv;
                px[0] = yy; px[1] = yy; px[2] = yy;
            }
        }
#endif
    } else {
        // RGB_565 or other formats not supported in this example
        AndroidBitmap_unlockPixels(env, bitmap);
        return -4;
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_example_native_1neon_NativeLib_grayScaleNeon(JNIEnv *env, jclass clazz, jobject bmp) {
    // TODO: implement grayScaleNeon()
}

