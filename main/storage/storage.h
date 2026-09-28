/*
 * storage.h — LittleFS wrapper for samples + patterns.
 *
 * One partition in partitions.csv:
 *   - "sketches" (littlefs)  → sketch folders + sketches.lst, 8 MB
 *
 * (The v2 multi-sketch system replaces the old single-set layout with
 * per-sketch folders. The per-sketch 16 patterns (g_patterns[16]) keep
 * the name "pattern" to match the PO-33's terminology. See
 * docs/DESIGN.md S11.)
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