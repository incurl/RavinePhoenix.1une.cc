/*
 * sync.c — PO-style sync pulse. 1 ms high pulse on each 16th note.
 *
 * F-031: also listens on SYNC_IN for incoming pulses. While the
 * sequencer is playing, each incoming pulse drives one step. While
 * stopped, pulses are ignored (the PO-33 is similar -- you press
 * PLAY and the device listens).
 */
#include "sync.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_rom_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sequencer/sequencer.h"

static const char *TAG = "sync";

/* F-031 sync IN. The ISR gives this semaphore; a FreeRTOS task
 * blocks on it and calls sequencer_tick(). Debouncing is in the task
 * (ignore pulses < 20 ms apart -- a real PO-33 sends ~1 ms pulses at
 * rates from ~60 BPM up to ~240 BPM, so the minimum interval at 240
 * BPM is ~62 ms; a 20 ms threshold catches accidental bounces). */
static SemaphoreHandle_t s_in_sem  = NULL;
static volatile int64_t  s_last_edge_us = 0;
#define SYNC_IN_DEBOUNCE_US  (20 * 1000)

static void IRAM_ATTR sync_in_isr(void *arg)
{
    (void)arg;
    int64_t now = esp_timer_get_time();
    if (now - s_last_edge_us < SYNC_IN_DEBOUNCE_US) return;
    s_last_edge_us = now;
    /* From an ISR: xSemaphoreGiveFromISR is safe on a binary
     * semaphore; we don't care about higher-priority tasks waking
     * immediately -- one tick of latency is fine. */
    BaseType_t higher_woken = pdFALSE;
    xSemaphoreGiveFromISR(s_in_sem, &higher_woken);
    if (higher_woken) portYIELD_FROM_ISR();
}

static void sync_in_task(void *arg)
{
    (void)arg;
    while (1) {
        /* Block until the ISR gives the semaphore. */
        if (xSemaphoreTake(s_in_sem, portMAX_DELAY) == pdTRUE) {
            if (sequencer_is_playing()) {
                sequencer_tick();
            }
            /* When stopped, just consume the pulse -- the next play
             * will be aligned to fresh incoming pulses. */
        }
    }
}

esp_err_t sync_init(void)
{
    gpio_reset_pin(SYNC_OUT_GPIO);
    gpio_set_direction(SYNC_OUT_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(SYNC_OUT_GPIO, 0);

    gpio_reset_pin(SYNC_IN_GPIO);
    gpio_set_direction(SYNC_IN_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(SYNC_IN_GPIO);

    /* F-031 sync IN. Falling-edge interrupt -- SYNC_IN is active-low
     * (PO-33 sends a 1 ms low pulse per 16th). */
    s_in_sem = xSemaphoreCreateBinary();
    if (!s_in_sem) {
        ESP_LOGE(TAG, "semaphore create failed");
        return ESP_ERR_NO_MEM;
    }
    gpio_set_intr_type(SYNC_IN_GPIO, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);  /* ESP_INTR_FLAG_LEVEL1 */
    gpio_isr_handler_add(SYNC_IN_GPIO, sync_in_isr, NULL);
    gpio_intr_enable(SYNC_IN_GPIO);

    xTaskCreate(sync_in_task, "sync_in", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Sync ready (IN listening on GPIO %d).", SYNC_IN_GPIO);
    return ESP_OK;
}

void sync_pulse(void)
{
    gpio_set_level(SYNC_OUT_GPIO, 1);
    /* Hold high for 1 ms then release; a small busy-wait is fine. */
    esp_rom_delay_us(1000);
    gpio_set_level(SYNC_OUT_GPIO, 0);
}