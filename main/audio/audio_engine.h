/*
 * audio_engine.h — mixer + voice allocator + per-block renderer.
 */
#ifndef PO33_AUDIO_ENGINE_H
#define PO33_AUDIO_ENGINE_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t audio_engine_init(void);
void      audio_engine_deinit(void);

/* Called from the audio task to render and push one block. */
void audio_engine_render_block(void);

/* Voice allocation: returns voice index or -1 if all busy. */
int audio_engine_alloc_voice(void);

/* Trigger a slot on a voice. */
void audio_engine_trigger_voice(int voice_idx, uint8_t slot,
                                uint8_t note, uint8_t velocity,
                                uint16_t filter_cutoff,
                                uint8_t effect_id, uint8_t fx_p1, uint8_t fx_p2);

/* Number of frames rendered per block. */
#define AUDIO_BLOCK_FRAMES  128

#ifdef __cplusplus
}
#endif

#endif