/*
 * power_mgmt.h — deep sleep + wake-on-button.
 */
#ifndef PO33_POWER_MGMT_H
#define PO33_POWER_MGMT_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t power_mgmt_init(void);
void      power_mgmt_reset_idle_timer(void);
void      power_mgmt_enter_deep_sleep(void);

/* F-035 battery monitor (single-shot ADC read). Returns 0..100
 * (integer percent), or 0xFF on read failure / init failure.
 *
 * Caller is responsible for not calling this in a tight loop -- each
 * call does one ADC read + voltage-to-percent conversion. The TFT
 * main screen calls it ~once per display_tick (every ~30 Hz), which
 * is fine; the ADC is not a contended resource on this chip.
 *
 * Initialisation is lazy: the first call sets up the ADC1 channel
 * attenuation; subsequent calls reuse it. Returns 0xFF if the ADC
 * read fails (e.g. ADC was not yet initialised by the knob driver
 * -- in that case the value is meaningless). */
uint8_t   power_mgmt_battery_get_percent(void);

#ifdef __cplusplus
}
#endif

#endif