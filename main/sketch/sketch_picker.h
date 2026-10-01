/*
 * sketch_picker.h — sketch picker state machine.
 *
 * Triggered by holding BTN_WRITE for >=600 ms (PO-33 "enter write
 * mode" verb repurposed for picker entry on the v2 firmware). The
 * picker shows the current sketch list on the TFT and lets the user
 * scroll + select:
 *
 *   - Turn encoder (1 detent)  -> scroll highlight by 1
 *   - Click encoder            -> load + exit
 *   - Press step 1..16         -> load + exit (direct jump)
 *   - Tap WRITE                -> exit without loading
 *
 * The picker is a single page of 16. If fewer than 16 sketches exist
 * the unused steps are blank.
 *
 * Lifecycle: storage_sketch_list() on enter; storage_sketch_load()
 * on select. The load is currently a stub (Commit 2 keeps it as
 * ESP_OK acknowledgement; the real load lands in Commit 3).
 */
#ifndef PO33_SKETCH_PICKER_H
#define PO33_SKETCH_PICKER_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns true if the picker is currently active (entered via
 * WRITE long-press, not yet exited). */
bool sketch_picker_is_active(void);

/* Enter the picker. Loads the current sketch list from
 * storage_sketch_list(). Idempotent: calling while already active
 * is a no-op. */
esp_err_t sketch_picker_enter(void);

/* Exit the picker without loading. Idempotent. */
void sketch_picker_exit(void);

/* Per-tick handler called from input_drain() after the modifier
 * dispatcher. Reads the encoder + click-switch via the encoder
 * driver and routes them through the picker state machine. */
void sketch_picker_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* PO33_SKETCH_PICKER_H */
