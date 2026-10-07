/*
 * encoder.c — left-side rotary encoder driver.
 *
 * PCNT unit 0 watches phase A (GPIO 22) and phase B (GPIO 23). The
 * PCNT peripheral counts pulses and tracks direction in hardware; we
 * read the cumulative count and compute the delta from the last read.
 *
 * Click-switch debounce: the EC11E has typical bounce time ~5 ms,
 * well below our 30 ms BTN_DEBOUNCE_MS. We sample the click GPIO
 * inside encoder_tick() (called at 10 ms cadence) and use a 3-state
 * debounce similar to the existing matrix button driver.
 */
#include "encoder.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "encoder";

/* PCNT handles + click state. Static because there's exactly one
 * encoder. */
static pcnt_unit_handle_t  s_pcnt_unit      = NULL;
static pcnt_channel_handle_t s_pcnt_chan    = NULL;
static int32_t              s_last_count    = 0;
static volatile bool        s_click_flag    = false;

/* Click-switch debounce state. The encoder's click-switch is
 * momentary-to-GND; we use the same model as the matrix buttons:
 * state = raw level, prev_state = last raw, press_ms = debounce
 * time of the current stable level. */
static int      s_click_raw = 1;          /* idle high w/ pull-up */
static int      s_click_prev = 1;
static bool     s_click_pressed = false;  /* stable pressed state */
static int64_t  s_click_press_ms = 0;

static int64_t now_ms(void)
{
    return esp_timer_get_time() / 1000;
}

static esp_err_t init_pcnt(void)
{
    pcnt_unit_config_t unit_cfg = {
        .low_limit = -32768,
        .high_limit = 32767,
        .intr_priority = 0,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_cfg, &s_pcnt_unit));

    pcnt_chan_config_t chan_cfg = {
        .edge_gpio_num   = ENCODER_A_GPIO,
        .level_gpio_num  = ENCODER_B_GPIO,
        .flags = { 0 },
    };
    ESP_ERROR_CHECK(pcnt_new_channel(s_pcnt_unit, &chan_cfg, &s_pcnt_chan));

    /* Rising edge on phase A: count up when B is high (CW rotation),
     * count down when B is low (CCW). Falling edge is the symmetric
     * pair. This is the standard quadrature decoder. */
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(s_pcnt_chan,
                                                PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                                PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(s_pcnt_chan,
                                                 PCNT_CHANNEL_LEVEL_ACTION_HOLD,
                                                 PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_enable(s_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(s_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(s_pcnt_unit));
    s_last_count = 0;

    return ESP_OK;
}

static esp_err_t init_click_switch(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << ENCODER_CLICK_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,    /* polled, debounced in tick() */
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
    return ESP_OK;
}

esp_err_t encoder_init(void)
{
    ESP_LOGI(TAG, "Initializing encoder: A=GPIO%d B=GPIO%d click=GPIO%d",
             ENCODER_A_GPIO, ENCODER_B_GPIO, ENCODER_CLICK_GPIO);

    esp_err_t err_pcnt = init_pcnt();
    if (err_pcnt != ESP_OK) {
        ESP_LOGE(TAG, "PCNT unit 0 init failed: %s -- encoder is REQUIRED",
                 esp_err_to_name(err_pcnt));
        return ESP_ERR_NOT_SUPPORTED;
    }
    esp_err_t err_click = init_click_switch();
    if (err_click != ESP_OK) return err_click;

    ESP_LOGI(TAG, "Encoder ready (PCNT unit 0, debounce %u ms)",
             (unsigned)BTN_DEBOUNCE_MS);
    return ESP_OK;
}

void encoder_tick(void)
{
    if (!s_pcnt_unit) return;

    /* Click-switch debounce. Mirror the matrix-button state machine:
     * track raw, prev, and the time of the last stable transition. */
    int raw = gpio_get_level(ENCODER_CLICK_GPIO);
    int64_t t = now_ms();
    if (raw != s_click_prev) {
        s_click_prev = raw;
        s_click_press_ms = t;
    } else if (raw != s_click_raw &&
               (t - s_click_press_ms) >= BTN_DEBOUNCE_MS) {
        s_click_raw = raw;
        /* EC11E click is active-low (switch to GND). 0 == pressed. */
        bool now_pressed = (s_click_raw == 0);
        if (now_pressed && !s_click_pressed) {
            s_click_flag = true;
        }
        s_click_pressed = now_pressed;
    }
}

int8_t encoder_get_delta(void)
{
    if (!s_pcnt_unit) return 0;
    int cur = 0;
    esp_err_t err = pcnt_unit_get_count(s_pcnt_unit, &cur);
    if (err != ESP_OK) return 0;
    int32_t delta32 = (int32_t)cur - s_last_count;
    /* Read-and-clear; the next read starts from zero delta. */
    (void)pcnt_unit_clear_count(s_pcnt_unit);
    s_last_count = 0;
    /* Clamp to int8_t so callers can never overflow on accumulation
     * (e.g. a fast spin generates many deltas per tick; without the
     * clamp a downstream arithmetic could wrap). The encoder is 1-
     * detent-per-click in our UX, so [-127, +127] is way more than
     * any reasonable per-tick delta. */
    if (delta32 >  127) return  127;
    if (delta32 < -127) return -127;
    return (int8_t)delta32;
}

bool encoder_was_clicked(void)
{
    if (!s_click_flag) return false;
    s_click_flag = false;
    return true;
}

void encoder_consume_click(void)
{
    s_click_flag = false;
}
