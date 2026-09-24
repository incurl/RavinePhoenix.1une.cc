/*
 * effects.h — 16 punch-in effect registry.
 */
#ifndef PO33_EFFECTS_H
#define PO33_EFFECTS_H

#include <stdint.h>

typedef enum {
    FX_NONE = 0,
    FX_LOOP_16,
    FX_LOOP_12,
    FX_LOOP_SHORT,
    FX_LOOP_SHORTER,
    FX_UNISON,
    FX_UNISON_LOW,
    FX_OCTAVE_UP,
    FX_OCTAVE_DOWN,
    FX_STUTTER_4,
    FX_STUTTER_3,
    FX_SCRATCH_FAST,
    FX_REVERSE,
    FX_RETRIGGER_PATTERN,
    FX_68_QUANTIZE,
    FX_FILTER_SWEEP,
    FX_BITCRUSH,
    FX_COUNT
} effect_id_t;

/* Per-effect scratch state (heap-allocated per voice by effects_init). */
typedef struct {
    int16_t *ring;     /* small PSRAM ring used by stutter/reverse */
    uint32_t ring_len;
    uint32_t ring_idx;
    uint16_t hold_count;
    uint16_t hold_target;
} effect_state_t;

void effects_init(void);
void effects_reset(effect_id_t id, effect_state_t *st);

/* Sample-by-sample DSP entrypoint. */
int16_t effects_process(effect_id_t id, effect_state_t *st,
                        int16_t in, uint8_t p1, uint8_t p2);

const char *effects_name(effect_id_t id);

#endif