/*
 * leds.c — simple GPIO LEDs.
 */
#include "leds.h"
#include "config.h"
#include "driver/gpio.h"

esp_err_t leds_init(void)
{
    gpio_reset_pin(LED_REC_GPIO);
    gpio_set_direction(LED_REC_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_PLAY_GPIO);
    gpio_set_direction(LED_PLAY_GPIO, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

void leds_set_rec(bool on)  { gpio_set_level(LED_REC_GPIO,  on ? 1 : 0); }
void leds_set_play(bool on) { gpio_set_level(LED_PLAY_GPIO, on ? 1 : 0); }