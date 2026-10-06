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

/* Per-slot metadata accessors (used by storage.c to round-trip
 * samples.bin). Return 0 / false if the slot is empty. */
uint32_t amy_bridge_slot_sample_rate_hz(uint8_t slot);
uint32_t amy_bridge_slot_start_sample  (uint8_t slot);
uint32_t amy_bridge_slot_end_sample    (uint8_t slot);
bool     amy_bridge_slot_is_drum       (uint8_t slot);

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

/* F-023: copy slot `src` to slot `dst`. After this call, both slots
 * hold identical sample data. Both AMY presets are re-registered
 * so playback is correct. The src slot is preserved (not deleted);
 * this is a true duplicate, not a swap.
 *
 * Implementation: compact-then-extend. Memmoves the pool contents
 * to make room at the dst position, copies src's bytes into the
 * vacated dst region, then updates s_slots[].length_samples for
 * every slot whose shift has changed.
 *
 * No-op (and returns ESP_ERR_INVALID_STATE) when either slot is
 * currently being recorded into, since the I2S capture path
 * races the memmove. */
esp_err_t amy_bridge_copy_slot(uint8_t dst, uint8_t src);

/* Note trigger from sequencer.
 *
 * `step_index_0_to_15` enables the F-024 auto-slice behaviour for
 * recorded drum slots: when the slot is a drum slot with a recording
 * and the per-event FX is PO33_FX_NONE, the loopstart/loopend fields
 * of the AMY event are set to the Nth 1/16th slice of the slot's
 * trimmed region (N = step_index_0_to_15). Pass 0xFF to disable the
 * auto-slice (loopstart/loopend left at AMY's default of "play the
 * whole sample"). Melodic slots and empty slots ignore this value
 * either way. */
esp_err_t amy_bridge_play_note(uint8_t slot, uint8_t midi_note,
                              uint8_t velocity,
                              po33_fx_t fx, uint8_t fx_p1, uint8_t fx_p2,
                              uint8_t filter_cutoff, uint8_t filter_resonance,
                              uint8_t step_index_0_to_15);

/* F-005 auto-mapping (the real PO-33 behaviour). When a melodic slot is
 * played with the user's explicit note = 0 ("not set"), the pad/step
 * index alone determines the pitch. The PO-33 maps the 16 pads to a
 * chromatic scale one octave wide (C4..D#5) -- so pad 1 plays C4, pad
 * 2 plays C#4, pad 16 plays D#5. Drum slots return 0 (no auto-map;
 * the PO-33 does not auto-slice drums in v1 firmware).
 *
 * `step_index` is 0..15 (0-based). The return value is a MIDI note
 * number 60..75 (C4..D#5). Caller decides what to do with it --
 * usually: use it as `midi_note` when the user has not set an
 * explicit note.
 *
 * Pre-condition: step_index in [0, 15]. Returns 60 if out of range.
 * For drum slots (slot < SLOT_DRUM_COUNT), always returns 0.
 *
 * See DESIGN.md F-005 (auto-mapping). */
uint8_t amy_bridge_auto_note_for_step(uint8_t slot, uint8_t step_index);

/* F-024 / F-005 drum auto-slicing (the real PO-33 behaviour). When a
 * drum slot is played with the user not having selected a per-step
 * effect that overrides loop bounds (LOOP_16, LOOP_SHORT, etc.), the
 * pad/step index alone determines which 1/16th of the recording to
 * play. Pad 1 = first 1/16th (samples 0..len/16-1), pad 2 = second
 * 1/16th, ..., pad 16 = last 1/16th.
 *
 * Writes the slice bounds into *loopstart and *loopend. The bounds
 * are derived from the slot's length_samples and the current trim
 * (start, end). For empty slots or non-drum slots, returns false
 * and writes nothing. Caller should pass the result into the AMY
 * event's loopstart/loopend fields before apply_fx() -- FX that
 * touch loop bounds (LOOP_16, LOOP_SHORT, STUTTER_*) will clobber
 * the auto-slice, which is the desired behaviour (FX always wins).
 *
 * step_index is 0..15 (0-based). Out-of-range inputs are clamped to 0.
 *
 * See DESIGN.md F-024 (auto-slicing). */
bool amy_bridge_auto_slice_for_step(uint8_t slot, uint8_t step_index,
                                     uint32_t *loopstart, uint32_t *loopend);

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