/*
 * clock.h — RTC time + alarm.
 */
#ifndef PO33_CLOCK_H
#define PO33_CLOCK_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t clock_init(void);
void      clock_get_hhmm(uint16_t *hh, uint16_t *mm);
void      clock_set_hhmm(uint16_t hh, uint16_t mm);
esp_err_t clock_set_alarm(uint16_t hh, uint16_t mm, uint8_t slot);

#ifdef __cplusplus
}
#endif

#endif