#include <jni.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <stdint.h>

#define LOG_TAG "native-neon"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/*
 * Hàm native được Java gọi:
 * NativeLib.grayScaleNeon(Bitmap bitmap)
 */
JNIEXPORT jint JNICALL
Java_com_example_native_1neon_NativeLib_grayScaleNeon(
        JNIEnv *env,
        jclass clazz,
        jobject bitmap) {

    AndroidBitmapInfo info;
    void *pixels = NULL;

    // Lấy thông tin bitmap
    if (AndroidBitmap_getInfo(env, bitmap, &info) != ANDROID_BITMAP_RESULT_SUCCESS) {
        ALOGE("AndroidBitmap_getInfo failed");
        return -1;
    }

    // Chỉ hỗ trợ ARGB_8888
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        ALOGE("Unsupported bitmap format");
        return -2;
    }

    // Khóa pixel để truy cập trực tiếp
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != ANDROID_BITMAP_RESULT_SUCCESS) {
        ALOGE("AndroidBitmap_lockPixels failed");
        return -3;
    }

    uint8_t *base = (uint8_t *) pixels;
    int width = info.width;
    int height = info.height;
    int stride = info.stride;

    // Duyệt từng pixel và chuyển sang grayscale
    for (int y = 0; y < height; y++) {
        uint8_t *row = base + y * stride;
        for (int x = 0; x < width; x++) {
            uint8_t *px = row + x * 4; // ARGB_8888

            uint8_t b = px[0];
            uint8_t g = px[1];
            uint8_t r = px[2];

            // Công thức grayscale (integer, nhanh)
            uint8_t gray = (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);

            px[0] = gray;
            px[1] = gray;
            px[2] = gray;
            // px[3] = alpha giữ nguyên
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    return 0; // SUCCESS
}
