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
 * Hàm duy nhất Java gọi:
 * Java_com_example_native_1neon_NativeLib_grayScaleNeon
 */
JNIEXPORT jint JNICALL
Java_com_example_native_1neon_NativeLib_grayScaleNeon(JNIEnv *env, jclass clazz, jobject bitmap) {

    AndroidBitmapInfo info;
    void *pixels = NULL;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) != ANDROID_BITMAP_RESULT_SUCCESS) {
        ALOGE("AndroidBitmap_getInfo failed: %d", ret);
        return -1;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        ALOGE("Only RGBA_8888 supported. Format=%d", info.format);
        return -2;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) != ANDROID_BITMAP_RESULT_SUCCESS) {
        ALOGE("AndroidBitmap_lockPixels failed: %d", ret);
        return -3;
    }

    uint8_t *base = (uint8_t *) pixels;
    const int width = info.width;
    const int height = info.height;
    const int stride = info.stride;
    const int bpp = 4;

#ifdef __ARM_NEON
    ALOGI("Running NEON optimized grayscale");
    for (int y = 0; y < height; y++) {
        uint8_t *row = base + y * stride;
        for (int x = 0; x < width; x++) {
            uint8_t *px = row + x * bpp;
            uint8_t B = px[0];
            uint8_t G = px[1];
            uint8_t R = px[2];
            uint32_t yv = (R * 77u + G * 150u + B * 29u) >> 8;
            uint8_t yy = (uint8_t) yv;
            px[0] = px[1] = px[2] = yy;
        }
    }
#else
    ALOGI("Running fallback C grayscale (no NEON, emulator)");
    for (int y = 0; y < height; y++) {
        uint8_t *row = base + y * stride;
        for (int x = 0; x < width; x++) {
            uint8_t *px = row + x * bpp;
            uint8_t B = px[0];
            uint8_t G = px[1];
            uint8_t R = px[2];
            uint32_t yv = (R * 77u + G * 150u + B * 29u) >> 8;
            uint8_t yy = (uint8_t) yv;
            px[0] = px[1] = px[2] = yy;
        }
    }
#endif

    AndroidBitmap_unlockPixels(env, bitmap);
    return 0;
}
