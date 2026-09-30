idf_component_register(
    SRC_DIRS "."
    EXCLUDE_SRCS "tests/*.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        driver
        esp_lcd
        amy
        esp_littlefs
        esp_timer
        nvs_flash
        esp_lcd_ili9341
    EMBED_FILES
    REQUIRES
)

# Unity-based test files live under main/tests/ but are not part of the
# firmware build. They belong in a separate `test` component (or a
# dedicated test runner) that links `unity` and runs on the host or in
# a dedicated firmware-test app. Keeping them out of SRC_DIRS prevents
# the firmware build from trying to compile them and failing on the
# missing `<unity.h>` dependency. See TODO note in main/tests/README
# (none yet; tracked as a separate task).