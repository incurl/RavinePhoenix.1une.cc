/*
 * reverse.c — captures a window then plays it backwards.
 */
#include "reverse.h"
#include "config.h"
#include "esp_heap_caps.h"
#include <string.h>

int16_t reverse_process(effect_state_t *st, int16_t in, uint8_t p1)
{
    if (!st) return in;
    uint32_t win_ms = 32 + (p1 * 8);   /* 32..2072 ms */
    uint32_t win_samples = (win_ms * SAMPLE_RATE_HZ) / 1000;
    if (win_samples < 64) win_samples = 64;
    if (win_samples > 16384) win_samples = 16384;

    if (st->ring_len != win_samples) {
        if (st->ring) heap_caps_free(st->ring);
        st->ring = heap_caps_malloc(win_samples * sizeof(int16_t),
                                    MALLOC_CAP_SPIRAM);
        if (!st->ring) { st->ring_len = 0; return in; }
        memset(st->ring, 0, win_samples * sizeof(int16_t));
        st->ring_len = win_samples;
        st->ring_idx = 0;
    }

    /* Push input, then read backwards. */
    st->ring[st->ring_idx] = in;
    st->ring_idx++;
    if (st->ring_idx >= st->ring_len) st->ring_idx = 0;

    uint32_t back_idx = (st->ring_idx == 0)
                            ? (st->ring_len - 1)
                            : (st->ring_idx - 1);
    return st->ring[back_idx];
}