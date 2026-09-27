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
     *   - Any modifier button low -> wake (REC, PLAY, FUNC, FX, BPM±, PAT±)
     */
    const gpio_num_t cols[BTN_COL_COUNT] = BTN_COL_PINS;
    for (int i = 0; i < BTN_COL_COUNT; i++) {
        gpio_wakeup_enable(cols[i], GPIO_INTR_LOW_LEVEL);
    }
    const gpio_num_t modifier_gpios[] = {
        BTN_REC_GPIO, BTN_PLAY_GPIO, BTN_FUNC_GPIO, BTN_FX_GPIO,
        BTN_BPM_UP_GPIO, BTN_BPM_DN_GPIO, BTN_PAT_UP_GPIO, BTN_PAT_DN_GPIO,
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