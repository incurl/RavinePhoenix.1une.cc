/*
 * effects.c — effect registry + dispatch.
 *
 * Loops (16/12/short/shorter) are handled in the mixer by adjusting
 * loop_end on the voice rather than per-sample DSP — they're free here.
 */
#include "effects.h"
#include "stutter.h"
#include "reverse.h"
#include "bitcrush.h"
#include "esp_heap_caps.h"
#include <string.h>
#include <stdlib.h>

static const char *s_names[FX_COUNT] = {
    [FX_NONE]               = "none",
    [FX_LOOP_16]            = "loop16",
    [FX_LOOP_12]            = "loop12",
    [FX_LOOP_SHORT]         = "loopS",
    [FX_LOOP_SHORTER]       = "loopSS",
    [FX_UNISON]             = "unison",
    [FX_UNISON_LOW]         = "unison-",
    [FX_OCTAVE_UP]          = "+oct",
    [FX_OCTAVE_DOWN]        = "-oct",
    [FX_STUTTER_4]          = "stut4",
    [FX_STUTTER_3]          = "stut3",
    [FX_SCRATCH_FAST]       = "scratch",
    [FX_REVERSE]            = "reverse",
    [FX_RETRIGGER_PATTERN]  = "retrig",
    [FX_68_QUANTIZE]        = "6/8",
    [FX_FILTER_SWEEP]       = "filter",
    [FX_BITCRUSH]           = "crush",
};

void effects_init(void)
{
    /* Nothing to do globally; per-voice state is allocated lazily. */
}

void effects_reset(effect_id_t id, effect_state_t *st)
{
    (void)id;
    if (!st) return;
    if (st->ring) {
        heap_caps_free(st->ring);
        st->ring = NULL;
    }
    st->ring_len   = 0;
    st->ring_idx   = 0;
    st->hold_count = 0;
    st->hold_target = 0;
}

const char *effects_name(effect_id_t id)
{
    if (id >= FX_COUNT) return "?";
    return s_names[id];
}

static int16_t fx_octave_up(int16_t s)   { return (int16_t)(s << 1); }
static int16_t fx_octave_down(int16_t s) { return (int16_t)(s >> 1); }
static int16_t fx_unison(int16_t s)      { return s; }              /* mixer duplicates */
static int16_t fx_unison_low(int16_t s)  { return fx_octave_down(s); }

static int16_t fx_filter_sweep(int16_t s, uint8_t p1)
{
    /* Lightweight: scale magnitude by sweep amount. */
    int32_t v = s;
    v = (v * (100 + (p1 / 4))) / 100;
    if (v > 32767) v = 32767;
    if (v < -32768) v = -32768;
    return (int16_t)v;
}

int16_t effects_process(effect_id_t id, effect_state_t *st,
                        int16_t in, uint8_t p1, uint8_t p2)
{
    switch (id) {
    case FX_NONE:
    case FX_LOOP_16:
    case FX_LOOP_12:
    case FX_LOOP_SHORT:
    case FX_LOOP_SHORTER:
    case FX_RETRIGGER_PATTERN:
    case FX_68_QUANTIZE:
        return in;

    case FX_OCTAVE_UP:   return fx_octave_up(in);
    case FX_OCTAVE_DOWN: return fx_octave_down(in);
    case FX_UNISON:      return fx_unison(in);
    case FX_UNISON_LOW:  return fx_unison_low(in);
    case FX_FILTER_SWEEP:return fx_filter_sweep(in, p1);

    case FX_STUTTER_4:   return stutter_process(st, in, 4,  p1);
    case FX_STUTTER_3:   return stutter_process(st, in, 3,  p1);
    case FX_SCRATCH_FAST:return stutter_process(st, in, 16, p1);
    case FX_REVERSE:     return reverse_process(st, in, p1);
    case FX_BITCRUSH:    return bitcrush_process(in, p1, p2);

    default:             return in;
    }
}