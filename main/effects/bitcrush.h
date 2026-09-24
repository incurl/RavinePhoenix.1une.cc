/*
 * bitcrush.h — bit reduction + decimation.
 */
#ifndef PO33_BITCRUSH_H
#define PO33_BITCRUSH_H

#include <stdint.h>

int16_t bitcrush_process(int16_t in, uint8_t bits, uint8_t downsample);

#endif