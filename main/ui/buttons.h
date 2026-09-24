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

#ifdef __cplusplus
}
#endif

#endif