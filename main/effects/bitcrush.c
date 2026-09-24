/*
 * bitcrush.c — bit depth reduction + sample-and-hold decimation.
 */
#include "bitcrush.h"
#include <stdint.h>

static int16_t s_hold = 0;
static uint8_t s_hold_n = 0;

int16_t bitcrush_process(int16_t in, uint8_t bits, uint8_t downsample)
{
    /* bits: 1..16 (effective). Reduce bit depth. */
    if (bits == 0 || bits > 16) return in;
    uint8_t shift = 16 - bits;
    int16_t crushed = (int16_t)(((uint16_t)in >> shift) << shift);

    /* downsample: hold previous sample for N frames. */
    if (downsample > 0) {
        if (s_hold_n == 0) {
            s_hold = crushed;
            s_hold_n = downsample;
        }
        s_hold_n--;
        return s_hold;
    }
    return crushed;
}