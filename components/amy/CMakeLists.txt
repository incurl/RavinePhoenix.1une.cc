# AMY component wrapper for ESP-IDF.
#
# The actual AMY sources live upstream at https://github.com/shorepine/amy
# and should be vendored under this directory by either:
#   - `git submodule add https://github.com/shorepine/amy.git components/amy`
#     (then copy/rename the upstream `src/` directory here), or
#   - `git clone https://github.com/shorepine/amy.git /tmp/amy && cp -r /tmp/amy/src/* components/amy/src/`
#
# This file registers them as an ESP-IDF component and supplies the
# platform flags needed for ESP32-S3 + v6.0 IDF.

idf_component_register(
    SRC_DIRS "src"
    INCLUDE_DIRS "src"
    EXCLUDE_SRCS "src/usb.c"        # usb.c is the TinyUSB MIDI/CDC path;
                                    # we don't depend on TinyUSB and don't
                                    # define AMYBOARD/AMYBOARD_ARDUINO.
    PRIV_REQUIRES driver freertos esp_timer esp_driver_i2s
    REQUIRES esp_driver_uart
)

# Compile-time knobs AMY exposes. See src/amy.h.
# Note: AMY_MCU and ESP_PLATFORM are set automatically by amy.h when
# building for an ESP target (and by the IDF toolchain respectively).
# Don't pass them here — doing so triggers "macro redefined -Werror".
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    AMY_WAVETABLE              # build the small built-in wavetable
    BLOCK_SIZE_BITS=8          # 256-frame render block
    AMY_NCHANS=2               # stereo output
    # Drop baked-in PCM banks — we record our own.
    AMY_NO_PCM_PRESETS
)

# AMY uses a few constructs the strict IDF warning flags don't love.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -O3
    -Wno-strict-aliasing
    -Wno-unused-parameter
    -Wno-float-conversion
    -Wno-missing-declarations
    -Wno-cast-function-type
    -Wno-implicit-fallthrough
)