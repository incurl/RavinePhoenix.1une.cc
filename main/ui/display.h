/*
 * display.h — 2.4" ILI9341 TFT driver + custom 2D draw API.
 *
 * Two screens are supported:
 *   0  = main / sequencer view (default after boot)
 *   1  = sketch picker (entered via WRITE long-press; set
 *         automatically by sketch_picker_enter / _exit)
 *
 * The display module reads the picker's state via the read-only
 * getters in sketch_picker.h; it doesn't own the picker state
 * machine.
 */
#ifndef PO33_DISPLAY_H
#define PO33_DISPLAY_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t display_init(void);
void      display_show_boot_screen(void);
void      display_tick(void);

#ifdef __cplusplus
}
#endif

#endif
