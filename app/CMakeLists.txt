cmake_minimum_required(VERSION 3.10.2)
project("native_neon")

add_library(
        native-lib
        SHARED
        src/main/cpp/native-lib.c
)

find_library(log-lib log)

# ABI-specific optimization flags
if (CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
    message(STATUS ">>> Using -mfpu=neon for armeabi-v7a")
    target_compile_options(native-lib PRIVATE -O3 -mfpu=neon)

elseif (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    message(STATUS ">>> ARM64 has NEON by default, no -mfpu needed")
    target_compile_options(native-lib PRIVATE -O3)

else()
    message(STATUS ">>> Non-ARM arch, fallback -O3")
    target_compile_options(native-lib PRIVATE -O3)
endif()

target_link_libraries(
        native-lib
        ${log-lib}
        jnigraphics
        android
)
