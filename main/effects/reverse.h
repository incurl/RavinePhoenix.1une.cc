/*
 * reverse.h — captures a small window and plays it backwards.
 */
#ifndef PO33_REVERSE_H
#define PO33_REVERSE_H

#include "effects.h"
#include <stdint.h>

int16_t reverse_process(effect_state_t *st, int16_t in, uint8_t p1);

#endif