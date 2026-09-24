/*
 * sync.c — PO-style sync pulse. 1 ms high pulse on each 16th note.
 */
#include "sync.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "sync";

esp_err_t sync_init(void)
{
    gpio_reset_pin(SYNC_OUT_GPIO);
    gpio_set_direction(SYNC_OUT_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(SYNC_OUT_GPIO, 0);

    gpio_reset_pin(SYNC_IN_GPIO);
    gpio_set_direction(SYNC_IN_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(SYNC_IN_GPIO);
    ESP_LOGI(TAG, "Sync ready.");
    return ESP_OK;
}

void sync_pulse(void)
{
    gpio_set_level(SYNC_OUT_GPIO, 1);
    /* Hold high for 1 ms then release; a small busy-wait is fine. */
    esp_rom_delay_us(1000);
    gpio_set_level(SYNC_OUT_GPIO, 0);
}