idf_component_register(
    SRC_DIRS "."
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