/*
 * storage.h — LittleFS wrapper for samples + patterns.
 *
 * One partition in partitions.csv:
 *   - "sketches" (littlefs)  → sketch folders + sketches.lst, 8 MB
 *
 * v1 firmware uses flat p0.bin..p15.bin / s0.bin..s15.bin in the
 * partition root. The v2 multi-sketch system (per docs/DESIGN.md §11)
 * uses per-sketch folders under /0000/, /0001/, ... and a master index
 * file sketches.lst. The v2 APIs coexist with v1: storage_save_all()
 * / storage_load_all() continue to manage the v1 layout for now
 * (Commit 3 will retire them).
 */
#ifndef PO33_STORAGE_H
#define PO33_STORAGE_H

#include "esp_err.h"
#include <stdint.h>
#include "config.h"   /* SKETCH_NAME_MAX used by SKETCH_NAME_LEN alias */
/* */

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

/* F-037 factory reset: wipe the sketches partition (every saved
 * pattern, the chain, and all sketch directories) and reset the
 * in-RAM pattern/chain state to defaults. Samples in PSRAM are
 * preserved. Caller is responsible for gating (UI gesture, warning
 * before invocation, etc.) -- this is destructive and irreversible. */
esp_err_t storage_factory_reset(void);

/* Multi-sketch (v2) APIs. */
#define SKETCH_ID_LEN   4      /* "0000"..SKETCH_ID_MAX */
#define SKETCH_NAME_LEN  SKETCH_NAME_MAX  /* alias — name is now the
                                          * Docker-style jazz string,
                                          * see sketch/sketch_name.h */
#define SKETCHES_MAX     16

/* Read the sketches.lst index into out_ids. out_ids must point at
 * SKETCHES_MAX * (SKETCH_ID_LEN+1) bytes. */
esp_err_t storage_sketch_list(uint8_t *out_count,
                              char (*out_ids)[SKETCH_ID_LEN + 1]);

/* Load sketch `id_str` (4-hex digits) into the active patterns.
 * Stub for Commit 2: full implementation lands in Commit 3 (storage
 * layer + UART verbs). Returns ESP_ERR_NOT_SUPPORTED until then. */
esp_err_t storage_sketch_load(const char *id_str);

/* Stub for Commit 2; full impl in Commit 3. */
esp_err_t storage_sketch_create(void);
esp_err_t storage_sketch_save_active(void);
esp_err_t storage_sketch_delete(const char *id_str);

#ifdef __cplusplus
}
#endif

#endif
