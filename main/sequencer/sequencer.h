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