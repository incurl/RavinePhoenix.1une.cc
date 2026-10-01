/*
 * encoder.h — left-side rotary encoder (Alps EC11E, 24 detents, click-switch).
 *
 * Hardware: GPIO 22 = phase A, GPIO 23 = phase B, GPIO 24 = click-switch.
 * Phase A/B are counted in hardware by ESP32-S3 PCNT unit 0; the click
 * is polled on the same 10 ms button-scan tick that drives buttons and
 * knobs, with the existing 30 ms debounce.
 *
 * The encoder is REQUIRED at build: if pcnt_new_unit() fails (which
 * shouldn't happen on a stock ESP32-S3 but might on a port with no
 * PCNT), encoder_init() returns ESP_ERR_NOT_SUPPORTED and the boot
 * halts. There is no fallback path -- the picker relies on the
 * encoder for scroll.
 *
 * API contract:
 *   encoder_get_delta()  Returns the signed count since the last call
 *                        (consumed) and resets to zero. Positive =
 *                        clockwise, negative = counter-clockwise.
 *                        Range: [-127, +127] per call (the count is
 *                        atomic but clamped to int8_t to make
 *                        caller-side overflow impossible).
 *   encoder_was_clicked() Returns true once if the click-switch
 *                        transitioned to pressed since the last call
 *                        (consumed). Mirrors the buttons Tick/Clear
 *                        flag model in ui/buttons.h.
 *   encoder_consume_click() Convenience: drain the click flag.
 */
#ifndef PO33_ENCODER_H
#define PO33_ENCODER_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the PCNT unit + click-switch GPIO. Required: returns
 * ESP_ERR_NOT_SUPPORTED if PCNT unit 0 cannot be allocated. */
esp_err_t encoder_init(void);

/* Periodic tick called from button_scan_task at ~10 ms cadence.
 * Polls the click-switch debounce and accumulates the click flag. */
void encoder_tick(void);

/* Read the accumulated encoder delta since the last call. The
 * underlying PCNT count is read-and-cleared; the returned value is
 * signed (positive = CW, negative = CCW). */
int8_t encoder_get_delta(void);

/* Consume-on-read click flag. Returns true exactly once per
 * press transition. The press is debounced inside encoder_tick(). */
bool encoder_was_clicked(void);

/* Convenience: drop the click flag without checking. */
void encoder_consume_click(void);

#ifdef __cplusplus
}
#endif

#endif /* PO33_ENCODER_H */
