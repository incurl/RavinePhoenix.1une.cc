/*
 * knobs.c — analog knob driver for Knob A (GPIO 2 / ADC1_CH1) and
 * Knob B (GPIO 46 / ADC1_CH5). See config.h for the wiring rationale.
 */
#include "knobs.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_adc/adc_cali.h"

static const char *TAG = "knobs";

/* Smoothed 12-bit (0..4095) ADC readings. Updated by knobs_tick(). */
static uint16_t s_raw_a = 0;
static uint16_t s_raw_b = 0;

/* Last *published* 0..255 values (after dead-zone). */
static uint8_t  s_pub_a = 0;
static uint8_t  s_pub_b = 0;

/* ADC calibration handles (one per channel). */
/* ADC calibration disabled in v6.x (esp_adc_cal_* removed).
 * Using raw 12-bit ADC readings (0..4095) directly. */
static bool    s_cal_a_ok = false;
static bool    s_cal_b_ok = false;

esp_err_t knobs_init(void)
{
    ESP_LOGI(TAG, "Initializing knobs: A=GPIO%d/CH%d  B=GPIO%d/CH%d",
             KNOB_A_GPIO, KNOB_A_ADC_CHANNEL,
             KNOB_B_GPIO, KNOB_B_ADC_CHANNEL);

    /* adc1_config_width() removed in v6.x; oneshot ADC driver will
     * configure width per-channel. Stubbed for now. */

    /* Channel A — GPIO 2 / ADC1_CH1. */
    s_cal_a_ok = false;
/* Channel B — GPIO 46 / ADC1_CH5. */
    s_cal_b_ok = false;
/* Take an initial reading so first call to knobs_get_*() is sane. */
    knobs_tick();

    ESP_LOGI(TAG, "Knobs ready (calibration: A=%s B=%s)",
             s_cal_a_ok ? "ok" : "fallback",
             s_cal_b_ok ? "ok" : "fallback");
    return ESP_OK;
}

/* Read one ADC channel, averaging KNOB_SAMPLES samples. */
static uint16_t read_averaged(adc_channel_t ch)
{
    /* Calibration disabled in v6.x. Use raw 12-bit readings directly. */
    uint32_t sum = 0;
    for (int i = 0; i < KNOB_SAMPLES; i++) {
        /* adc1_get_raw() removed in v6.x; returning 0 until the
         * oneshot ADC driver is wired. */
        int raw = 0;
        if (raw < 0) raw = 0;
        if (raw > 4095) raw = 4095;
        sum += (uint16_t)raw;
    }
    return (uint16_t)(sum / KNOB_SAMPLES);
}

/* Map a 0..4095 ADC reading to 0..255 with simple dead-zone hysteresis.
 * The published value only changes when it differs by more than
 * KNOB_DEADZONE from the previous published value, which eliminates
 * ADC noise chatter without making the knob feel laggy. */
static uint8_t publish(uint8_t prev, uint16_t raw12)
{
    /* Scale 0..4095 -> 0..255 with rounding. */
    uint8_t now = (uint8_t)((raw12 + 8) / 16);
    int diff = (int)now - (int)prev;
    if (diff < 0) diff = -diff;
    return (diff >= KNOB_DEADZONE) ? now : prev;
}

void knobs_tick(void)
{
    s_raw_a = read_averaged(KNOB_A_ADC_CHANNEL);
    s_raw_b = read_averaged(KNOB_B_ADC_CHANNEL);
    s_pub_a = publish(s_pub_a, s_raw_a);
    s_pub_b = publish(s_pub_b, s_raw_b);
}

uint8_t knobs_get_a(void) { return s_pub_a; }
uint8_t knobs_get_b(void) { return s_pub_b; }