/*
 * sync.h — jam-sync pulse on a GPIO (output) + listen (input).
 */
#ifndef PO33_SYNC_H
#define PO33_SYNC_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sync_init(void);
void      sync_pulse(void);

#ifdef __cplusplus
}
#endif

#endif