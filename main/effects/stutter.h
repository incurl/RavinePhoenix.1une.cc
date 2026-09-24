/*
 * stutter.h — granular repeat / scratch effect.
 */
#ifndef PO33_STUTTER_H
#define PO33_STUTTER_H

#include "effects.h"
#include <stdint.h>

int16_t stutter_process(effect_state_t *st, int16_t in,
                        uint16_t grains, uint8_t p1);

#endif