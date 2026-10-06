/*
 * storage.h — LittleFS wrapper for samples + patterns + sketch
 * folders + sketches.lst, with NVS-backed active-sketch pointer.
 *
 * One partition in partitions.csv:
 *   - "sketches" (littlefs)  → sketch folders + sketches.lst, 8 MB
 *
 * On-flash layout (v2 — see docs/DESIGN.md §11.2):
 *
 *     /sketches/
 *       sketches.lst                 # master index: "<id>\t<name>\n"
 *       0000/
 *         meta.bin                    # fixed 24-byte NUL-padded name
 *         samples.bin                 # 256-B slot table + raw PCM
 *         patterns.bin                # PATTERN_COUNT * sizeof(pattern_t)
 *         chain.bin                   # 1 byte len + len bytes entries
 *       0001/
 *         ...
 *
 * NVS namespace "sketches" key "active_id" holds the 4-hex ID of
 * the last-active sketch; boot calls storage_load_all() which reads
 * it and pulls patterns + samples for that sketch into PSRAM.
 *
 * Samples belong to *one* sketch at a time. PSRAM holds at most one
 * sketch's 3.37 MB sample pool (see main/audio/amy_bridge.c). Switching
 * sketches = save current RAM → flash, then load new sketch → RAM.
 * ADR-0004 records this invariant.
 */
#ifndef PO33_STORAGE_H
#define PO33_STORAGE_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>
#include "config.h"   /* SKETCH_NAME_MAX used by SKETCH_NAME_LEN alias */

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t storage_init(void);
esp_err_t storage_mount(void);

/* Save the in-RAM state of the ACTIVE sketch (patterns, chain,
 * sample pool) to its folder on flash, and bump the NVS active_id
 * so the next boot lands here. This is the user-facing "save"
 * verb; also called automatically before deep sleep.
 *
 * If no sketch is currently active (fresh flash, post factory-reset),
 * the active sketch is implicitly "0000" — a default sketch is
 * created if the sketches.lst is empty, and storage_set_active_id()
 * is called to make the next boot land on it. */
esp_err_t storage_save_all(void);

/* Boot-time load: read NVS "sketches/active_id" (default "0000"),
 * then load patterns + chain + samples for that sketch. If the
 * sketch folder does not exist (first ever boot, or the partition
 * was wiped), this returns ESP_OK with everything still empty —
 * i.e. behaves like a fresh device. */
esp_err_t storage_load_all(void);

/* Per-pattern save/load to the *active sketch's* patterns.bin.
 * Kept public for legacy callers and the UART `pattern_save` verb;
 * new code should prefer storage_save_all() / storage_load_all(). */
esp_err_t storage_save_pattern(uint8_t idx);
esp_err_t storage_load_pattern(uint8_t idx);

/* F-037 factory reset: wipe the sketches partition (every saved
 * pattern, the chain, and all sketch directories), reset the
 * in-RAM pattern/chain state to defaults, and clear the NVS
 * active_id so the next boot starts fresh. Samples in PSRAM are
 * preserved (callers can clear them with slot_clear if needed).
 *
 * Destructive and irreversible. Caller is responsible for gating
 * (UI gesture, warning before invocation, etc.). */
esp_err_t storage_factory_reset(void);

/* ─── Active sketch identity (NVS-backed) ─────────────────────── */

/* Multi-sketch (v2) APIs. */
#define SKETCH_ID_LEN   4      /* "0000"..SKETCH_ID_MAX */
#define SKETCH_NAME_LEN  SKETCH_NAME_MAX  /* alias — name is now the
                                          * Docker-style jazz string,
                                          * see sketch/sketch_name.h */
#define SKETCHES_MAX     16

/* Read the 4-hex active sketch ID into out_id (must be at least
 * SKETCH_ID_LEN + 1 bytes). On a fresh device with no NVS entry
 * this writes "0000" and returns ESP_OK. Returns ESP_ERR_INVALID_ARG
 * on bad NVS contents. */
esp_err_t storage_get_active_sketch_id(char *out_id);

/* Persist the active sketch ID in NVS so the next boot can load
 * it. Called automatically by storage_save_all() and
 * storage_sketch_save_active(); callers rarely need this directly.
 * Id must be exactly 4 lowercase hex chars. */
esp_err_t storage_set_active_sketch_id(const char *id);

/* Read the sketches.lst index into out_ids. out_ids must point at
 * SKETCHES_MAX * (SKETCH_ID_LEN+1) bytes. */
esp_err_t storage_sketch_list(uint8_t *out_count,
                              char (*out_ids)[SKETCH_ID_LEN + 1]);

/* Load sketch `id_str` (4-hex digits) — patterns + chain only.
 * The sample pool is NOT touched; call storage_sketch_load_samples()
 * (or storage_sketch_load_with_samples() to do both in one call)
 * to bring the PSRAM sample pool in sync. */
esp_err_t storage_sketch_load(const char *id_str);

/* Load the sample pool from /<id_str>/samples.bin into PSRAM via
 * amy_bridge_register_slot(). Safe to call repeatedly (each call
 * resets the slot table and re-registers all slots). */
esp_err_t storage_sketch_load_samples(const char *id_str);

/* Convenience: load patterns + chain + samples for `id_str`. */
esp_err_t storage_sketch_load_with_samples(const char *id_str);

/* Persist the current PSRAM sample pool to /<id_str>/samples.bin
 * (256-byte slot table + raw PCM, little-endian). Used internally
 * by storage_sketch_save_active(); rarely called directly. */
esp_err_t storage_sketch_save_samples(const char *id_str);

esp_err_t storage_sketch_create(void);
esp_err_t storage_sketch_save_active(void);
esp_err_t storage_sketch_delete(const char *id_str);

#ifdef __cplusplus
}
#endif

#endif
