/*
 * power_mgmt.h — deep sleep + wake-on-button.
 */
#ifndef PO33_POWER_MGMT_H
#define PO33_POWER_MGMT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t power_mgmt_init(void);
void      power_mgmt_reset_idle_timer(void);
void      power_mgmt_enter_deep_sleep(void);

#ifdef __cplusplus
}
#endif

#endif