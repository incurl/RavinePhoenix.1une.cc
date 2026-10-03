/*
 * knobs.c — analog knob driver for Knob A (GPIO 2 / ADC1_CH1) and
 * Knob B (GPIO 46 / ADC1_CH5). See config.h for the wiring rationale.
 */
#include "knobs.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_adc_cal.h"

static const char *TAG = "knobs";

/* Smoothed 12-bit (0..4095) ADC readings. Updated by knobs_tick(). */
static uint16_t s_raw_a = 0;
static uint16_t s_raw_b = 0;

/* Last *published* 0..255 values (after dead-zone). */
static uint8_t  s_pub_a = 0;
static uint8_t  s_pub_b = 0;

/* ADC calibration handles (one per channel). */
static esp_adc_cal_characteristics_t s_cal_a;
static esp_adc_cal_characteristics_t s_cal_b;
static bool    s_cal_a_ok = false;
static bool    s_cal_b_ok = false;

esp_err_t knobs_init(void)
{
    ESP_LOGI(TAG, "Initializing knobs: A=GPIO%d/CH%d  B=GPIO%d/CH%d",
             KNOB_A_GPIO, KNOB_A_ADC_CHANNEL,
             KNOB_B_GPIO, KNOB_B_ADC_CHANNEL);

    /* ADC1 unit-wide config (driver-managed; safe to call once). */
    adc1_config_width(ADC_WIDTH_BIT_12);

    /* Channel A — GPIO 2 / ADC1_CH1. */
    adc1_config_channel_atten(KNOB_A_ADC_CHANNEL, KNOB_ADC_ATTEN);
    s_cal_a_ok = esp_adc_cal_characterize(ADC_UNIT_1, KNOB_ADC_ATTEN,
                                          ADC_WIDTH_BIT_12, 0,
                                          &s_cal_a) == ESP_OK;

    /* Channel B — GPIO 46 / ADC1_CH5. */
    adc1_config_channel_atten(KNOB_B_ADC_CHANNEL, KNOB_ADC_ATTEN);
    s_cal_b_ok = esp_adc_cal_characterize(ADC_UNIT_1, KNOB_ADC_ATTEN,
                                          ADC_WIDTH_BIT_12, 0,
                                          &s_cal_b) == ESP_OK;

    /* Take an initial reading so first call to knobs_get_*() is sane. */
    knobs_tick();

    ESP_LOGI(TAG, "Knobs ready (calibration: A=%s B=%s)",
             s_cal_a_ok ? "ok" : "fallback",
             s_cal_b_ok ? "ok" : "fallback");
    return ESP_OK;
}

/* Read one ADC channel, averaging KNOB_SAMPLES samples. */
static uint16_t read_averaged(adc_channel_t ch,
                              const esp_adc_cal_characteristics_t *cal,
                              bool cal_ok)
{
    uint32_t sum = 0;
    for (int i = 0; i < KNOB_SAMPLES; i++) {
        int raw = adc1_get_raw(ch);
        if (cal_ok) {
            raw = esp_adc_cal_raw_to_voltage(raw, cal);
            /* map mV (0..3300) back into 0..4095 (proportional) */
            raw = (raw * 4095 + 1650) / 3300;
        }
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
    s_raw_a = read_averaged(KNOB_A_ADC_CHANNEL, &s_cal_a, s_cal_a_ok);
    s_raw_b = read_averaged(KNOB_B_ADC_CHANNEL, &s_cal_b, s_cal_b_ok);
    s_pub_a = publish(s_pub_a, s_raw_a);
    s_pub_b = publish(s_pub_b, s_raw_b);
}

uint8_t knobs_get_a(void) { return s_pub_a; }
uint8_t knobs_get_b(void) { return s_pub_b; }