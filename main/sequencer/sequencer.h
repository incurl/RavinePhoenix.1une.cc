/*
 * sequencer.h — 16-step sequencer with pattern chain.
 */
#ifndef PO33_SEQUENCER_H
#define PO33_SEQUENCER_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sequencer_init(void);

/* Transport */
void sequencer_play(void);
void sequencer_stop(void);
bool sequencer_is_playing(void);
void sequencer_set_bpm(uint16_t bpm);
void sequencer_cycle_bpm_preset(void);
void sequencer_set_pattern(uint8_t pattern);
void sequencer_chain_append(uint8_t pattern);  /* PO-33 chain build */
void sequencer_chain_clear(void);

/* F-014: remove all occurrences of `pattern` from the chain. After
 * this call, the chain plays as if pattern had never been appended.
 * Cheap O(N) sweep over the chain buffer. */
void sequencer_chain_remove(uint8_t pattern);

/* F-014: swap chain entries at indices i and j. Both must be in range
 * (< sequencer_chain_len()). No-op if i == j. */
void sequencer_chain_swap(uint8_t i, uint8_t j);

/* F-014: insert `pattern` at index `at` in the chain (shifts later
 * entries forward by one). `at` must be <= chain length; at==length
 * appends. Refuses if the chain is full. */
esp_err_t sequencer_chain_insert(uint8_t at, uint8_t pattern);

/* F-014: number of chain entries currently pointing at `pattern`. */
size_t sequencer_chain_count(uint8_t pattern);

/* Read-only chain introspection (UART + tests). */
uint8_t sequencer_chain_at(uint8_t index);  /* index < sequencer_chain_len() */
uint8_t sequencer_chain_len(void);         /* current chain length */

/* Active slot / active FX. Set by the SOUND / FX hold-+-number
 * dispatcher in ui/input.c, consumed by on_step() so that subsequent
 * triggers (sequencer or "press step without modifier after a SOUND
 * select") pick up the user's selection. Initial state is
 * "no slot selected" (0xFF) and "no effect" (PO33_FX_NONE). */
void     sequencer_set_active_slot(uint8_t slot);      /* 0..15 or 0xFF = none */
uint8_t  sequencer_get_active_slot(void);
void     sequencer_set_active_fx(uint8_t fx);          /* po33_fx_t value */
uint8_t  sequencer_get_active_fx(void);

/* Swing level (0..SWING_LEVELS-1). Set by the BPM-held + Knob A
 * dispatcher; consumed by on_step() to delay off-beat 16th notes.
 * See config.h SWING_LEVELS for the level count and the SWING
 * mapping formula in sequencer.c for the delay math. */
void     sequencer_set_swing(uint8_t level);           /* 0..SWING_LEVELS-1 */
uint8_t  sequencer_get_swing(void);
uint16_t sequencer_get_bpm(void);
uint8_t  sequencer_get_current_step(void);
uint8_t  sequencer_get_current_pattern(void);

/* F-012: wipe all 16 steps of the active pattern. Does NOT touch
 * the chain. The pattern index remains the same (s_pattern unchanged);
 * only the step bindings are reset to empty. */
void sequencer_clear_current_pattern(void);

/* F-031 jam-sync IN: advance the sequencer by one step. Called from
 * the sync listener task when SYNC_IN_GPIO receives a pulse. If the
 * sequencer is playing, this triggers the same per-step logic as the
 * local esp_timer tick (notes, FX, swing). No-op when stopped. */
void sequencer_tick(void);

/* F-031: toggle sync-IN-driven playback. When true, sequencer_play()
 * does NOT start the local esp_timer step clock; instead the listener
 * task (system/sync.c) drives sequencer_tick() per incoming pulse. */
void sequencer_set_sync_in_active(bool active);

/* F-019 PO33_FX_RETRIGGER_PATTERN: request that the active pattern
 * restart from step 0 on the next step tick. Posted (not immediate)
 * so the call from amy_bridge is safe from any context -- it just
 * sets a flag the sequencer's on_step() callback consumes. */
void sequencer_request_retrigger(void);

/* F-019 "save effect in pattern": set the per-step `effect` field
 * on every step of the active pattern that has a slot bound.
 * Steps without a slot (slot_id == 0xFF) are left untouched so empty
 * steps stay empty. p1/p2 are the FX parameter bytes from the
 * PO-33 punch-in table. fx == PO33_FX_NONE means "no effect". */
void sequencer_save_fx_to_pattern(uint8_t fx, uint8_t p1, uint8_t p2);

/* Pattern building helpers */
void sequencer_set_step_slot(uint8_t pattern, uint8_t step,
                             uint8_t slot, uint8_t note);

/* Called from audio task on every render block. */
void sequencer_tick(void);

/* Debug */
void sequencer_print_status(void);

#ifdef __cplusplus
}
#endif

#endif