/*
 * i2s_driver.h — I2S0 TX (DAC) and I2S1 RX (mic) wrapper using v6.0 channel API.
 */
#ifndef PO33_I2S_DRIVER_H
#define PO33_I2S_DRIVER_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t audio_i2s_init(void);
void      audio_i2s_deinit(void);

/* Blocking-style write of `samples` (16-bit mono interleaved as stereo L=R)
 * into the I2S TX DMA. Returns bytes consumed. */
size_t audio_i2s_write(const int16_t *samples, size_t num_frames);

/* Read up to `max_frames` mono samples from the mic DMA into `out`. */
size_t audio_i2s_read(int16_t *out, size_t max_frames);

#ifdef __cplusplus
}
#endif

#endif /* PO33_I2S_DRIVER_H */