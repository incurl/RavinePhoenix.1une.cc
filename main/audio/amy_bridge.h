/*
 * amy_bridge.h — thin wrapper over the AMY synthesizer library.
 *
 * Responsibilities:
 *   - Boot AMY with our pin map and config.
 *   - Register our 16 sample slots as AMY PCM presets (PSRAM-backed).
 *   - Translate PO-33 punch-in effects → AMY primitives.
 *   - Provide note-trigger, sample-record and capture-buffer APIs to the rest
 *     of the firmware (sequencer, buttons, storage).
 *
 *   All "sound" lives here. The rest of the firmware (sequencer/pattern.c,
 *   ui/buttons.c, storage/storage.c, ui/display.c, system/*) is unchanged
 *   from the pre-AMY plan.
 */
#ifndef PO33_AMY_BRIDGE_H
#define PO33_AMY_BRIDGE_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Effect IDs we expose to the rest of the firmware. Order is the PO-33
 * punch-in order (see README "Effects" table). */
typedef enum {
    PO33_FX_NONE = 0,
    PO33_FX_LOOP_16,
    PO33_FX_LOOP_12,
    PO33_FX_LOOP_SHORT,
    PO33_FX_LOOP_SHORTER,
    PO33_FX_UNISON,
    PO33_FX_UNISON_LOW,
    PO33_FX_OCTAVE_UP,
    PO33_FX_OCTAVE_DOWN,
    PO33_FX_STUTTER_4,
    PO33_FX_STUTTER_3,
    PO33_FX_SCRATCH_FAST,
    PO33_FX_REVERSE,
    PO33_FX_RETRIGGER_PATTERN,
    PO33_FX_68_QUANTIZE,
    PO33_FX_FILTER_SWEEP,
    PO33_FX_BITCRUSH,
    PO33_FX_COUNT
} po33_fx_t;

esp_err_t amy_bridge_init(void);
void      amy_bridge_deinit(void);

/* Slot management (0..15). Each slot is backed by a PSRAM buffer that's
 * registered with AMY as an in-memory PCM preset. */
esp_err_t amy_bridge_register_slot(uint8_t slot, const int16_t *data,
                                   size_t num_samples, uint32_t sample_rate_hz,
                                   bool is_drum);
const    int16_t *amy_bridge_slot_ptr(uint8_t slot);
size_t   amy_bridge_slot_len_samples(uint8_t slot);
size_t   amy_bridge_slot_max_bytes(uint8_t slot);
bool     amy_bridge_slot_has_sample(uint8_t slot);  /* true iff slot has a recording (>0 samples) */
esp_err_t amy_bridge_set_trim(uint8_t slot, uint32_t start, uint32_t end);

/* Recording (mic → PSRAM). `rec_buf` should point at the active slot's PSRAM
 * region; amy_bridge pumps AMY's input buffer into it one block at a time. */
esp_err_t amy_bridge_start_record(uint8_t slot);
void      amy_bridge_stop_record(void);
bool      amy_bridge_is_recording(void);
void      amy_bridge_pump_capture(void);    /* call from audio task */

/* F-026: clear a slot's recording. Marks the slot as not-in-use,
 * unregisters the preset from AMY (so playback falls back to ROM
 * preset 0 again), and zeroes the sample data in s_pool. Safe to
 * call on a slot that was never recorded (no-op). The slot is then
 * available for re-recording. */
void      amy_bridge_clear_slot(uint8_t slot);

/* Note trigger from sequencer. */
esp_err_t amy_bridge_play_note(uint8_t slot, uint8_t midi_note,
                              uint8_t velocity,
                              po33_fx_t fx, uint8_t fx_p1, uint8_t fx_p2,
                              uint8_t filter_cutoff, uint8_t filter_resonance);

/* Master volume level (PO-33 F-022: hold BPM + 1..5 sets the level).
 * Level 0 (silent) is also accepted. Internally scales the velocity
 * applied to subsequent play_note() calls. Levels map to multipliers:
 *   level 0 -> 0.0   (silent)
 *   level 1 -> 0.25
 *   level 2 -> 0.5
 *   level 3 -> 0.75
 *   level 4 -> 0.9
 *   level 5 -> 1.0  (max)
 * AMY does not expose a master gain in its public API, so we apply
 * the multiplier per-note. This means very-short notes might be
 * affected by the level change between trigger and playback; in
 * practice the 10 ms tick is fast enough that this is inaudible. */
void amy_bridge_set_volume_level(uint8_t level);
uint8_t amy_bridge_get_volume_level(void);  /* current level 0..5 */

#ifdef __cplusplus
}
#endif

#endif /* PO33_AMY_BRIDGE_H */