cmake_minimum_required(VERSION 3.10.2)
project("native_NEON")

add_library(
        native_lib
        SHARED
        src/main/cpp/native-lib.c
)

find_library(log-lib log)

# Bật tối ưu hóa O3 + NEON (arm64 NEON mặc định)
target_compile_options(native-lib PRIVATE -O3)

target_link_libraries(
        native-lib
        ${log-lib}
        jnigraphics     # <-- bắt buộc để dùng AndroidBitmap_*
        android         # <-- khuyến nghị, cần thiết cho vài API của Android NDK
)
