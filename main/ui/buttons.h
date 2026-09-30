/*
 * buttons.h — 4×4 matrix scanner.
 */
#ifndef PO33_BUTTONS_H
#define PO33_BUTTONS_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t btn_id;
    bool    long_press;
} button_event_t;

esp_err_t buttons_init(void);

/* Returns one event (or {0xFF,false} if queue empty). */
button_event_t buttons_pop(void);

/* Periodic scan — call from button_scan_task. */
void buttons_tick(void);

/* Returns the current debounced state of the given button ID.
 * For modifier buttons (BTN_SOUND..BTN_WRITE) returns true while held.
 * For step buttons, returns true while the matrix sees the press.
 * Used by mode logic (e.g. "while BPM is held, Knob A = BPM adjust"). */
bool buttons_is_pressed(uint8_t btn_id);

#ifdef __cplusplus
}
#endif

#endif