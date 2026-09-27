/*
 * knobs.h — PO-33-style "Knob A" and "Knob B" analog inputs.
 *
 * Each knob is a 10 kΩ linear potentiometer on an ADC pin (see config.h).
 * The driver returns a 0..255 value (0 = fully CCW, 255 = fully CW)
 * with simple moving-average smoothing and a small dead-zone so the
 * caller doesn't see jitter from ADC noise.
 */
#ifndef PO33_KNOBS_H
#define PO33_KNOBS_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t knobs_init(void);

/* Call periodically (e.g. from the button-scan task) to refresh
 * the smoothed values. Internally averages KNOB_SAMPLES ADC reads. */
void knobs_tick(void);

/* Returns the current 0..255 position of Knob A. */
uint8_t knobs_get_a(void);

/* Returns the current 0..255 position of Knob B. */
uint8_t knobs_get_b(void);

#ifdef __cplusplus
}
#endif

#endif /* PO33_KNOBS_H */