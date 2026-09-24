/*
 * buttons.c — 4×4 matrix scan with debounce + short/long press detection.
 */
#include "buttons.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/queue.h"
#include <string.h>

static const char *TAG = "btns";

static const gpio_num_t s_rows[BTN_ROW_COUNT] = BTN_ROW_PINS;
static const gpio_num_t s_cols[BTN_COL_COUNT] = BTN_COL_PINS;

static bool     s_state[BTN_ROW_COUNT][BTN_COL_COUNT];
static int64_t  s_press_time[BTN_ROW_COUNT][BTN_COL_COUNT];
static bool     s_long_fired[BTN_ROW_COUNT][BTN_COL_COUNT];
static QueueHandle_t s_queue = NULL;

static int64_t now_ms(void)
{
    return esp_timer_get_time() / 1000;
}

esp_err_t buttons_init(void)
{
    for (int r = 0; r < BTN_ROW_COUNT; r++) {
        gpio_reset_pin(s_rows[r]);
        gpio_set_direction(s_rows[r], GPIO_MODE_OUTPUT);
        gpio_set_level(s_rows[r], 1);   /* idle high */
    }
    for (int c = 0; c < BTN_COL_COUNT; c++) {
        gpio_reset_pin(s_cols[c]);
        gpio_set_direction(s_cols[c], GPIO_MODE_INPUT);
        gpio_pullup_en(s_cols[c]);
    }
    memset(s_state, 0, sizeof(s_state));
    memset(s_press_time, 0, sizeof(s_press_time));
    memset(s_long_fired, 0, sizeof(s_long_fired));

    s_queue = xQueueCreate(32, sizeof(button_event_t));
    if (!s_queue) return ESP_ERR_NO_MEM;

    ESP_LOGI(TAG, "Button matrix ready (%dx%d)", BTN_ROW_COUNT, BTN_COL_COUNT);
    return ESP_OK;
}

static uint8_t btn_id_for(int r, int c)
{
    /* Row-major mapping: btn = r * COL_COUNT + c */
    return (uint8_t)(r * BTN_COL_COUNT + c);
}

void buttons_tick(void)
{
    for (int r = 0; r < BTN_ROW_COUNT; r++) {
        gpio_set_level(s_rows[r], 0);
        for (int c = 0; c < BTN_COL_COUNT; c++) {
            int level = gpio_get_level(s_cols[c]);
            bool pressed = (level == 0);
            bool prev = s_state[r][c];
            if (pressed != prev) {
                int64_t t = now_ms();
                if (pressed) {
                    s_press_time[r][c] = t;
                    s_long_fired[r][c]  = false;
                } else {
                    if (!s_long_fired[r][c] &&
                        (t - s_press_time[r][c]) >= BTN_DEBOUNCE_MS) {
                        button_event_t ev = { .btn_id = btn_id_for(r, c),
                                              .long_press = false };
                        xQueueSend(s_queue, &ev, 0);
                    }
                }
                s_state[r][c] = pressed;
            } else if (pressed && !s_long_fired[r][c]) {
                int64_t t = now_ms();
                if ((t - s_press_time[r][c]) >= BTN_LONG_PRESS_MS) {
                    s_long_fired[r][c] = true;
                    button_event_t ev = { .btn_id = btn_id_for(r, c),
                                          .long_press = true };
                    xQueueSend(s_queue, &ev, 0);
                }
            }
        }
        gpio_set_level(s_rows[r], 1);
    }
}

button_event_t buttons_pop(void)
{
    button_event_t ev = { .btn_id = 0xFF, .long_press = false };
    if (s_queue) xQueueReceive(s_queue, &ev, 0);
    return ev;
}