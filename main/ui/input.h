/*
 * input.h — button-event dispatcher.
 *
 * Drains the FreeRTOS button queue (populated by buttons_tick()) and
 * routes each event to the matching subsystem handler (transport,
 * pattern selection, BPM, REC, FX, WRITE, step toggles, etc.).
 *
 * Designed to be called from button_scan_task at ~10 ms cadence. The
 * function drains *all* queued events per call so a button-burst
 * during a brief scan gap never loses input.
 */
#ifndef PO33_INPUT_H
#define PO33_INPUT_H

#include "ui/buttons.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Drain every event currently in the button queue. No-op if empty. */
void input_drain(void);

/* Tweak-mode state machine (PO-33 F-015 / F-016/017/018). Cycle order:
 * Tone -> Filter -> Trim -> Tone. Tweak mode is "always on" (no
 * NONE state) per the PO-33 manual wording; the sequencer reads the
 * current mode via tweak_get_mode() and applies knob-axis bindings
 * (Knob A/B) when triggering notes.
 *
 * Trim has no per-step data field -- trim is per-slot. For Trim mode,
 * the caller (play_active_slot) should call tweak_apply_slot()
 * which routes Knob A/B into amy_bridge_set_trim() on the active
 * slot. For in-pattern tweaking (sequencer on_step), Trim is a
 * no-op because we can't per-step trim; the per-step voice will
 * still play. */
typedef enum {
    TWEAK_TONE   = 0,
    TWEAK_FILTER,
    TWEAK_TRIM,
    TWEAK_MODE_COUNT,
} tweak_mode_t;

tweak_mode_t tweak_get_mode(void);

/* Apply the current tweak-mode binding (Tone or Filter) to a
 * per-step note in place. No-op for Trim mode (which has no per-step
 * representation). Knob A maps to: Tone -> note (MIDI 0..127),
 * Filter -> filter_cutoff (0..255). Knob B maps to: Tone ->
 * velocity (0..127), Filter -> filter_resonance (0..255).
 *
 * Both filter outputs are per-step fields (step_t.filter_cutoff,
 * step_t.filter_resonance), populated by the caller before invoking
 * amy_bridge_play_note(). The audio layer maps them onto the AMY
 * filter (filter_freq / filter_resonance) in amy_bridge.c. */
void tweak_apply_step(uint8_t *note, uint8_t *velocity,
                      uint8_t *filter_cutoff, uint8_t *filter_resonance);

/* Apply the current tweak-mode binding (Trim) to the active slot.
 * No-op for Tone and Filter modes (the caller can call this
 * unconditionally; for Tone/Filter the function is a no-op).
 * Knob A -> trim start, Knob B -> trim end. Calls
 * amy_bridge_set_trim() with the new bounds. */
void tweak_apply_slot(uint8_t slot);

/* Write-mode state (PO-33 F-009). Toggle on WRITE tap when the
 * picker is inactive. While in write mode, tapping a step button
 * binds (or clears, on toggle) the active slot for that step. The
 * picker is read-only, so write mode and the picker cannot both be
 * active -- tapping WRITE while in the picker exits the picker (existing
 * behaviour). */
bool   write_mode_is_active(void);
void   write_mode_enter(void);
void   write_mode_exit(void);

/* Apply write-mode to the step at the given 1-based step index
 * (1..16) using the currently-active slot (sequencer_get_active_slot).
 * If no slot is active, this is a no-op. The assignment follows the
 * PO-33's "press step again to remove" toggle semantics (F-010).
 *
 * is_long: false = tap = toggle; true = long-press = clear (F-011).
 * If no slot is active OR no step is at this index, returns false
 * (the caller can decide whether to fall through). */
bool   write_mode_apply_step(uint8_t step_1_to_16, bool is_long);

#ifdef __cplusplus
}
#endif

#endif /* PO33_INPUT_H */