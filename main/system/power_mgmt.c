/*
 * power_mgmt.c — idle timer + deep sleep.
 *
 * Wakes on any column pin going low (active-low matrix).
 */
#include "power_mgmt.h"
#include "config.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "ui/buttons.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "power";

static esp_timer_handle_t s_idle_timer = NULL;

static void on_idle(void *arg)
{
    (void)arg;
    ESP_LOGW(TAG, "Idle timeout — entering deep sleep");
    power_mgmt_enter_deep_sleep();
}

esp_err_t power_mgmt_init(void)
{
    const esp_timer_create_args_t cfg = {
        .name = "idle_timer",
        .callback = on_idle,
        .arg = NULL,
        .skip_unhandled_events = true,
    };
    ESP_ERROR_CHECK(esp_timer_create(&cfg, &s_idle_timer));
    esp_timer_start_once(s_idle_timer, BTN_IDLE_SLEEP_MS * 1000ULL);

    /* Configure wake sources.
     *
     *   - Any matrix column low  -> wake (a step button press)
     *   - Any modifier button low -> wake (SOUND, PATTERN, BPM, REC, FX, PLAY, WRITE)
     */
    const gpio_num_t cols[BTN_COL_COUNT] = BTN_COL_PINS;
    for (int i = 0; i < BTN_COL_COUNT; i++) {
        gpio_wakeup_enable(cols[i], GPIO_INTR_LOW_LEVEL);
    }
    const gpio_num_t modifier_gpios[] = {
        BTN_SOUND_GPIO, BTN_PATTERN_GPIO, BTN_BPM_GPIO,
        BTN_REC_GPIO, BTN_FX_GPIO, BTN_PLAY_GPIO, BTN_WRITE_GPIO,
    };
    for (size_t i = 0; i < sizeof(modifier_gpios) / sizeof(modifier_gpios[0]); i++) {
        gpio_wakeup_enable(modifier_gpios[i], GPIO_INTR_LOW_LEVEL);
    }
    esp_sleep_enable_gpio_wakeup();
    return ESP_OK;
}

void power_mgmt_reset_idle_timer(void)
{
    if (s_idle_timer) {
        esp_timer_restart(s_idle_timer, BTN_IDLE_SLEEP_MS * 1000ULL);
    }
}

void power_mgmt_enter_deep_sleep(void)
{
    ESP_LOGI(TAG, "Going to deep sleep now.");
    esp_deep_sleep_start();
}

/* ─── F-035 battery monitor ────────────────────────────────────── */

/* Lazy-init flag: the knob driver (`ui/knobs.c`) also touches ADC1
 * at boot (configures width 12 + per-channel attenuation). Our
 * battery channel needs the same ATTEN setting, so we initialise
 * lazily on the first call rather than at power_mgmt_init() time,
 * to avoid an ordering conflict with the knob driver. */
static bool s_battery_adc_initialised = false;

/* ESP32-S3 ADC full-scale at 12-bit, ATTEN_DB_12 = ~3.1 V usable
 * range. We measure at the divider midpoint, so cell_voltage =
 * adc_voltage * BATTERY_DIVIDER_RATIO. */
#define ADC_MAX_COUNTS_AT_12BIT 4095.0f
#define ADC_REFERENCE_VOLTAGE   3.1f

/* Li-ion discharge curve approximation. The cell voltage at 100%
 * charge is ~4.2 V; at 0% (cutoff) ~3.0 V. The shape is non-linear --
 * voltage drops quickly at first, then plateaus around 3.7 V for the
 * middle 60% of capacity, then drops sharply at the end. The piecewise
 * mapping below captures that "S-curve" to within ~10% accuracy,
 * which is plenty for a 5-bar display. */
static uint8_t voltage_to_percent(float v)
{
    if (v >= 4.20f) return 100;
    if (v >= 4.05f) return (uint8_t)(80 + (v - 4.05f) * (100 - 80) / (4.20f - 4.05f));
    if (v >= 3.85f) return (uint8_t)(60 + (v - 3.85f) * (80 - 60) / (4.05f - 3.85f));
    if (v >= 3.70f) return (uint8_t)(40 + (v - 3.70f) * (60 - 40) / (3.85f - 3.70f));
    if (v >= 3.50f) return (uint8_t)(20 + (v - 3.50f) * (40 - 20) / (3.70f - 3.50f));
    if (v >= 3.30f) return (uint8_t)( 5 + (v - 3.30f) * (20 -  5) / (3.50f - 3.30f));
    if (v >= 3.00f) return (uint8_t)( 0 + (v - 3.00f) * ( 5 -  0) / (3.30f - 3.00f));
    return 0;
}

uint8_t power_mgmt_battery_get_percent(void)
{
    if (!s_battery_adc_initialised) {
        /* Configure our channel at the same ATTEN the knob driver uses.
         * adc1_config_width() is a no-op if already called by the knob
         * driver, but it doesn't hurt to call it. */
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(BATTERY_ADC_CHANNEL, BATTERY_ADC_ATTEN);
        s_battery_adc_initialised = true;
    }

    int raw = adc1_get_raw(BATTERY_ADC_CHANNEL);
    if (raw < 0) {
        ESP_LOGW(TAG, "battery ADC read failed (raw=%d)", raw);
        return 0xFF;
    }

    /* Convert raw ADC counts to a cell voltage, then to a percent. */
    float adc_v   = ((float)raw / ADC_MAX_COUNTS_AT_12BIT) * ADC_REFERENCE_VOLTAGE;
    float cell_v  = adc_v * BATTERY_DIVIDER_RATIO;
    uint8_t pct   = voltage_to_percent(cell_v);

    /* Throttled log: avoid spamming every display_tick (~30 Hz). */
    static uint32_t last_log_ms = 0;
    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
    if (now_ms - last_log_ms > 5000) {
        ESP_LOGD(TAG, "battery: raw=%d adc=%.2fV cell=%.2fV pct=%u",
                 raw, adc_v, cell_v, (unsigned)pct);
        last_log_ms = now_ms;
    }
    return pct;
}