/*
 * leds.h — status LEDs (record / play).
 */
#ifndef PO33_LEDS_H
#define PO33_LEDS_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t leds_init(void);
void      leds_set_rec(bool on);
void      leds_set_play(bool on);

#ifdef __cplusplus
}
#endif

#endif