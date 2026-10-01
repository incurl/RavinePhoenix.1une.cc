/*
 * display.c — 2.4" ILI9341 (SPI) using esp_lcd + custom 2D draw API on a
 * PSRAM framebuffer.
 */
#include "display.h"
#include "config.h"
#include "sequencer/sequencer.h"
#include "sketch/sketch_picker.h"
#include "system/clock.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "disp";

static esp_lcd_panel_handle_t s_panel = NULL;
static esp_lcd_panel_io_handle_t s_io = NULL;
static uint16_t *s_fb = NULL;

/* 5x7 font (digits, A-Z, basic punctuation). */
static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, {0x3E,0x51,0x49,0x45,0x3E},
    {0x00,0x42,0x7F,0x40,0x00}, {0x42,0x61,0x51,0x49,0x46},
    {0x21,0x41,0x45,0x4B,0x31}, {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39}, {0x3C,0x4A,0x49,0x49,0x30},
    {0x01,0x71,0x09,0x05,0x03}, {0x36,0x49,0x49,0x49,0x36},
    {0x06,0x49,0x49,0x29,0x1E}, {0x00,0x36,0x36,0x00,0x00},
    {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
    {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
    {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
    {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01},
    {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
    {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06},
    {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
    {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
    {0x7F,0x20,0x18,0x20,0x7F}, {0x63,0x14,0x08,0x14,0x63},
    {0x03,0x04,0x78,0x04,0x03}, {0x61,0x51,0x49,0x45,0x43},
};

static inline uint8_t font_glyph(char c, uint8_t *out5)
{
    if (c >= '0' && c <= '9') { memcpy(out5, font5x7[1 + (c-'0')], 5); return 1; }
    if (c >= 'A' && c <= 'Z') { memcpy(out5, font5x7[12 + (c-'A')], 5); return 1; }
    if (c >= 'a' && c <= 'z') { memcpy(out5, font5x7[12 + (c-'a')], 5); return 1; }
    switch (c) {
    case ' ': memcpy(out5, font5x7[0], 5);  return 1;
    case ':': memcpy(out5, font5x7[11], 5); return 1;
    case '.': memcpy(out5, font5x7[1], 5);  return 1;
    }
    memset(out5, 0, 5);
    return 0;
}

static inline void fb_set_pixel(int x, int y, uint16_t color)
{
    if (x < 0 || y < 0 || x >= TFT_WIDTH || y >= TFT_HEIGHT) return;
    s_fb[y * TFT_WIDTH + x] = color;
}

static void fb_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            fb_set_pixel(xx, yy, color);
}

static void fb_draw_text(int x, int y, const char *s, uint16_t color)
{
    while (*s) {
        uint8_t g[5];
        font_glyph(*s++, g);
        for (int col = 0; col < 5; col++) {
            uint8_t bits = g[col];
            for (int row = 0; row < 7; row++) {
                if (bits & (1 << row)) fb_set_pixel(x + col, y + row, color);
            }
        }
        x += 6;
    }
}

static void fb_flush(void)
{
    if (!s_panel || !s_fb) return;
    esp_lcd_panel_draw_bitmap(s_panel, 0, 0, TFT_WIDTH, TFT_HEIGHT, s_fb);
}

static void backlight_init(void)
{
    ledc_timer_config_t tcfg = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num       = TFT_BL_PWM_TIMER,
        .freq_hz         = TFT_BL_PWM_HZ,
    };
    ledc_timer_config(&tcfg);
    ledc_channel_config_t ccfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = TFT_BL_PWM_CHANNEL,
        .timer_sel  = TFT_BL_PWM_TIMER,
        .gpio_num   = TFT_BL_GPIO,
        .duty       = (TFT_BL_DEFAULT_DUTY * 255) / 100,
    };
    ledc_channel_config(&ccfg);
}

esp_err_t display_init(void)
{
    ESP_LOGI(TAG, "Initializing 2.4\" TFT (ILI9341) on SPI");

    s_fb = heap_caps_malloc(TFT_WIDTH * TFT_HEIGHT * sizeof(uint16_t),
                            MALLOC_CAP_SPIRAM);
    if (!s_fb) {
        ESP_LOGE(TAG, "Framebuffer alloc failed");
        return ESP_ERR_NO_MEM;
    }
    memset(s_fb, 0, TFT_WIDTH * TFT_HEIGHT * sizeof(uint16_t));

    backlight_init();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = TFT_MOSI_GPIO,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = TFT_SCK_GPIO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(TFT_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num       = TFT_DC_GPIO,
        .cs_gpio_num       = TFT_CS_GPIO,
        .pclk_hz           = TFT_SPI_CLK_HZ,
        .lcd_cmd_bits      = 8,
        .lcd_param_bits    = 8,
        .spi_mode          = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)TFT_SPI_HOST, &io_cfg, &s_io));

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = TFT_RST_GPIO,
        .rgb_endian     = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(s_io, &panel_cfg, &s_panel));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(s_panel, 0, 0));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel, true));

    ESP_LOGI(TAG, "TFT ready.");
    return ESP_OK;
}

void display_show_boot_screen(void)
{
    fb_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, 0x0000);
    fb_draw_text(60, 140, "PO 33 K O",  0xFFFF);
    fb_draw_text(40, 160, "ESP32 S3 FW", 0x07E0);
    fb_flush();
}

static char s_buf[64];

/* Top status bar text color: blue (ILI9341 RGB565 BGR = 0x001F). */
#define COLOR_STATUS_BAR_BG  0x001F
#define COLOR_STATUS_BAR_FG  0xFFFF
#define COLOR_BG              0x0000
#define COLOR_TEXT            0xFFFF
#define COLOR_TEXT_DIM        0xC618   /* grey */
#define COLOR_HIGHLIGHT_BG    0xF800   /* red */
#define COLOR_SLOT_EMPTY      0x4208   /* dark grey */

static void render_main_screen(void)
{
    fb_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, COLOR_BG);

    /* top status bar */
    fb_fill_rect(0, 0, TFT_WIDTH, 18, COLOR_STATUS_BAR_BG);

    snprintf(s_buf, sizeof(s_buf), "PAT%u", sequencer_get_current_pattern());
    fb_draw_text(4, 4, s_buf, COLOR_STATUS_BAR_FG);

    snprintf(s_buf, sizeof(s_buf), "BPM %u", sequencer_get_bpm());
    fb_draw_text(80, 4, s_buf, COLOR_STATUS_BAR_FG);

    uint16_t h, m;
    clock_get_hhmm(&h, &m);
    snprintf(s_buf, sizeof(s_buf), "%02u:%02u", h, m);
    fb_draw_text(180, 4, s_buf, COLOR_STATUS_BAR_FG);

    /* 16 step grid */
    int step = sequencer_get_current_step();
    for (int i = 0; i < STEPS_PER_PATTERN; i++) {
        int x = 8 + i * 14;
        int y = 60;
        if (i == step) fb_fill_rect(x - 1, y - 1, 12, 12, COLOR_HIGHLIGHT_BG);
        fb_fill_rect(x, y, 10, 10, COLOR_TEXT);
    }

    snprintf(s_buf, sizeof(s_buf), "STEP %02u", step);
    fb_draw_text(8, 100, s_buf, 0x07FF);
}

/* Sketch picker screen. Renders a top bar with "SKETCHES N / 16" + a
 * 4x4 grid (16 slots, 4 rows x 4 cols). The currently-highlighted
 * slot (from sketch_picker_get_highlight) is filled with a red
 * background and the ID rendered in inverse. Empty slots (beyond
 * sketch_picker_get_count) are rendered in dim grey.
 *
 * Layout (240x320):
 *   y=0..18   status bar (same blue as main)
 *   y=24      "SKETCHES  N / 16"        (white text)
 *   y=24      "Click=load WRITE=exit"   (grey text, right side)
 *   y=40..300 16 slots in a 4x4 grid, each row 64px tall
 *
 * The matrix rows mirror the matrix: slot index 0..3 in row 0, 4..7
 * in row 1, etc. This means the picker matrix maps naturally to the
 * physical 4x4 button matrix: pressing STEP 1 selects the first row
 * leftmost slot, pressing STEP 5 selects the second row leftmost
 * slot, etc. */
static void render_picker_screen(void)
{
    fb_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, COLOR_BG);

    /* top status bar (same look as main) */
    fb_fill_rect(0, 0, TFT_WIDTH, 18, COLOR_STATUS_BAR_BG);
    fb_draw_text(4, 4, "PICKER", COLOR_STATUS_BAR_FG);
    fb_draw_text(80, 4, "TURN=scroll", COLOR_STATUS_BAR_FG);

    /* "SKETCHES N / 16" header */
    uint8_t count = sketch_picker_get_count();
    snprintf(s_buf, sizeof(s_buf), "SKETCHES %u / 16", (unsigned)count);
    fb_draw_text(4, 24, s_buf, COLOR_TEXT);

    /* Hint text */
    fb_draw_text(120, 24, "click=load", COLOR_TEXT_DIM);

    /* 4x4 grid of slots. Each cell 56x64 px, 4-px gap. */
    const int cell_w = 56;
    const int cell_h = 64;
    const int gap = 4;
    const int grid_x = (TFT_WIDTH - (4 * cell_w + 3 * gap)) / 2;  /* centred */
    const int grid_y = 40;

    uint8_t highlight = sketch_picker_get_highlight();

    for (uint8_t i = 0; i < 16; i++) {
        int row = i / 4;
        int col = i % 4;
        int x = grid_x + col * (cell_w + gap);
        int y = grid_y + row * (cell_h + gap);

        if (i == highlight) {
            fb_fill_rect(x, y, cell_w, cell_h, COLOR_HIGHLIGHT_BG);
            fb_fill_rect(x + 2, y + 2, cell_w - 4, cell_h - 4, COLOR_BG);
        } else {
            fb_fill_rect(x, y, cell_w, cell_h, COLOR_SLOT_EMPTY);
        }

        char id[8] = {0};
        if (i < count) {
            sketch_picker_get_id(i, id);
        } else {
            strcpy(id, "----");
        }
        /* ID at top of cell, white on highlighted, dim on others. */
        uint16_t text_color = (i == highlight) ? COLOR_TEXT_DIM : COLOR_TEXT;
        fb_draw_text(x + 6, y + 6, id, text_color);

        /* Sketch index at bottom of cell. */
        snprintf(s_buf, sizeof(s_buf), "%u", (unsigned)(i + 1));
        fb_draw_text(x + 22, y + 22, s_buf, text_color);

        /* Star marker for the highlighted slot. */
        if (i == highlight) {
            fb_draw_text(x + 22, y + 50, "*", 0xFFE0);  /* yellow */
        }
    }

    /* Footer with rotate/click hint. */
    fb_draw_text(4, 308, "WRITE=exit", COLOR_TEXT_DIM);
    fb_draw_text(170, 308, "tap step=load", COLOR_TEXT_DIM);
}

void display_tick(void)
{
    if (sketch_picker_is_active()) {
        render_picker_screen();
    } else {
        render_main_screen();
    }
    fb_flush();
}