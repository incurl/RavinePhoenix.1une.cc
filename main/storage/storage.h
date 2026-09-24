/*
 * storage.h — LittleFS wrapper for samples + patterns.
 *
 * Two partitions in partitions.csv:
 *   - "samples"   (littlefs)  → raw sample pool, 6 MB
 *   - "patterns"  (littlefs)  → patterns.json + meta, 1 MB
 */
#ifndef PO33_STORAGE_H
#define PO33_STORAGE_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t storage_init(void);
esp_err_t storage_mount(void);

esp_err_t storage_save_all(void);
esp_err_t storage_load_all(void);

/* Per-pattern save/load (raw blob for simplicity) */
esp_err_t storage_save_pattern(uint8_t idx);
esp_err_t storage_load_pattern(uint8_t idx);

#ifdef __cplusplus
}
#endif

#endif