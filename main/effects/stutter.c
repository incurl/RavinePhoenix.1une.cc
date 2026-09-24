/*
 * stutter.c — repeats a small window of the input stream.
 *
 *   grains = number of repeats per hold (e.g. 4 for stutter-4).
 *   p1     = window size in ms (16..512).
 */
#include "stutter.h"
#include "config.h"
#include "esp_heap_caps.h"
#include <string.h>

int16_t stutter_process(effect_state_t *st, int16_t in,
                        uint16_t grains, uint8_t p1)
{
    if (!st) return in;

    uint32_t win_ms = 16 + (p1 * 4);   /* 16..1038 ms */
    uint32_t win_samples = (win_ms * SAMPLE_RATE_HZ) / 1000;
    if (win_samples < 64) win_samples = 64;
    if (win_samples > 8192) win_samples = 8192;

    if (st->ring_len != win_samples) {
        if (st->ring) heap_caps_free(st->ring);
        st->ring = heap_caps_malloc(win_samples * sizeof(int16_t),
                                    MALLOC_CAP_SPIRAM);
        if (!st->ring) { st->ring_len = 0; return in; }
        memset(st->ring, 0, win_samples * sizeof(int16_t));
        st->ring_len = win_samples;
        st->ring_idx = 0;
        st->hold_count = 0;
        st->hold_target = grains;
    }

    /* Capture input into ring. */
    st->ring[st->ring_idx] = in;
    st->ring_idx++;
    if (st->ring_idx >= st->ring_len) {
        st->ring_idx = 0;
        st->hold_count++;
    }

    /* Output the captured window (slightly delayed). */
    uint32_t out_idx = st->ring_idx;
    int16_t out = st->ring[out_idx];
    (void)grains;
    return out;
}