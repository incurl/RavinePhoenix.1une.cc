/*
 * voice.h — polyphonic voice state.
 *
 * Voices read directly from the PSRAM sample pool (zero-copy). The audio
 * mixer pulls `VOICE_COUNT` voices per output frame, applies pitch via
 * linear-interpolation resampling, optionally runs an effect, and sums.
 */
#ifndef PO33_VOICE_H
#define PO33_VOICE_H

#include <stdint.h>
#include <stdbool.h>
#include "effects/effects.h"

typedef enum {
    VOICE_STATE_IDLE = 0,
    VOICE_STATE_PLAYING,
    VOICE_STATE_RECORDING,
} voice_state_t;

typedef struct {
    voice_state_t state;

    /* Source pointer into PSRAM sample pool. */
    const int16_t *src;
    uint32_t       src_len_samples;   /* mono frames */
    uint32_t       start;             /* trim start */
    uint32_t       end;               /* trim end (exclusive) */
    uint32_t       loop_start;        /* for drum loops */
    uint32_t       loop_end;

    /* Playback cursor (fixed-point, accumulator = pos * 256 for Q24.8). */
    uint32_t pos_fp;                  /* Q24.8: integer part = sample index */
    uint32_t step_fp;                 /* Q24.8 increment per output frame */
    uint16_t pitch_semi;              /* chromatic shift in semitones (0..127) */
    uint16_t velocity;                /* 0..127 */

    /* Effect state */
    effect_id_t     effect;
    effect_state_t  effect_state;
    uint8_t         effect_param_1;
    uint8_t         effect_param_2;

    /* Per-voice filter cutoff (0..255) */
    uint16_t filter_cutoff;

    int64_t start_time_us;            /* for retrigger timing */
} voice_t;

void voice_init(voice_t *v);
void voice_trigger(voice_t *v, uint8_t slot, uint8_t note, uint8_t velocity,
                   uint16_t filter_cutoff,
                   effect_id_t fx, uint8_t fx_p1, uint8_t fx_p2);

#endif