/*
 * display.h — 2.4" ILI9341 TFT driver + custom 2D draw API.
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