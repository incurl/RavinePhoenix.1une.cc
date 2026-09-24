/*
 * audio_engine.c — polyphonic mixer with linear-interpolation resampling.
 *
 * Per block (128 frames ≈ 5.8 ms @ 22050 Hz):
 *   1. Read N mic samples from I2S (when recording).
 *   2. For each active voice: read at fractional cursor, apply FX, sum.
 *   3. Apply 1-pole LP filter (cutoff param).
 *   4. Soft-clip the mix.
 *   5. Push mono block to I2S TX (duplicated to stereo in i2s_driver).
 */
#include "audio_engine.h"
#include "voice.h"
#include "sample_manager.h"
#include "effects/effects.h"
#include "i2s_driver.h"
#include "config.h"
#include "esp_log.h"
#include <string.h>
#include <math.h>

static const char *TAG = "audio_engine";

static voice_t s_voices[VOICE_COUNT];
static int16_t  s_block[AUDIO_BLOCK_FRAMES];
static int16_t  s_mic_buf[AUDIO_BLOCK_FRAMES];

static uint32_t s_filter_z = 0;
static uint16_t s_master_cutoff = 200;   /* 0..255 */

/* Q24.8 fractional sample index. */
#define FRAC_BITS 8
#define FRAC_ONE  (1u << FRAC_BITS)

static inline int16_t sample_at(const int16_t *p, uint32_t len, uint32_t pos_fp)
{
    if (!p || len == 0) return 0;
    uint32_t i = pos_fp >> FRAC_BITS;
    if (i >= len - 1) return 0;
    uint32_t frac = pos_fp & (FRAC_ONE - 1);
    int32_t a = p[i];
    int32_t b = p[i + 1];
    int32_t s = a + (((b - a) * (int32_t)frac) >> FRAC_BITS);
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    return (int16_t)s;
}

esp_err_t audio_engine_init(void)
{
    for (int i = 0; i < VOICE_COUNT; i++) voice_init(&s_voices[i]);
    memset(s_block, 0, sizeof(s_block));
    ESP_LOGI(TAG, "Audio engine ready (%d voices).", VOICE_COUNT);
    return ESP_OK;
}

void audio_engine_deinit(void)
{
    for (int i = 0; i < VOICE_COUNT; i++) s_voices[i].state = VOICE_STATE_IDLE;
}

int audio_engine_alloc_voice(void)
{
    for (int i = 0; i < VOICE_COUNT; i++) {
        if (s_voices[i].state == VOICE_STATE_IDLE) return i;
    }
    /* steal oldest */
    int oldest = 0;
    int64_t oldest_t = INT64_MAX;
    for (int i = 0; i < VOICE_COUNT; i++) {
        if (s_voices[i].start_time_us < oldest_t) {
            oldest_t = s_voices[i].start_time_us;
            oldest = i;
        }
    }
    return oldest;
}

void audio_engine_trigger_voice(int voice_idx, uint8_t slot,
                                uint8_t note, uint8_t velocity,
                                uint16_t filter_cutoff,
                                uint8_t effect_id, uint8_t fx_p1, uint8_t fx_p2)
{
    if (voice_idx < 0 || voice_idx >= VOICE_COUNT) return;
    voice_trigger(&s_voices[voice_idx], slot, note, velocity,
                  filter_cutoff, (effect_id_t)effect_id, fx_p1, fx_p2);
}

static inline int16_t soft_clip(int32_t x)
{
    /* tanh approx */
    if (x > 30000) return 32767;
    if (x < -30000) return -32768;
    return (int16_t)x;
}

static inline int16_t one_pole_lp(int16_t in, uint32_t *z, uint16_t cutoff)
{
    /* cutoff: 0..255 → coefficient 0.05..1.0 */
    float a = 0.05f + (cutoff / 255.0f) * 0.95f;
    int32_t y = (int32_t)((float)(*z) * (1.0f - a) + (float)in * a);
    *z = (uint32_t)(y & 0xFFFFFFFF);
    return (int16_t)y;
}

void audio_engine_render_block(void)
{
    memset(s_block, 0, sizeof(s_block));

    /* 1. mic → recording (one block at a time) */
    if (sample_manager_is_recording()) {
        size_t got = audio_i2s_read(s_mic_buf, AUDIO_BLOCK_FRAMES);
        if (got > 0) {
            sample_manager_record_block(s_mic_buf, got);
        }
    }

    /* 2. mix voices */
    for (int v = 0; v < VOICE_COUNT; v++) {
        voice_t *vc = &s_voices[v];
        if (vc->state != VOICE_STATE_PLAYING) continue;
        if (!vc->src || vc->src_len_samples == 0) {
            vc->state = VOICE_STATE_IDLE;
            continue;
        }

        uint32_t end   = vc->end;
        uint32_t start = vc->start;
        if (end > vc->src_len_samples) end = vc->src_len_samples;
        if (start >= end) { vc->state = VOICE_STATE_IDLE; continue; }

        int16_t gain = (int16_t)((vc->velocity * 256) / 127);

        for (size_t i = 0; i < AUDIO_BLOCK_FRAMES; i++) {
            uint32_t cur = vc->pos_fp;
            if ((cur >> FRAC_BITS) >= end - 1) {
                if (vc->loop_end > vc->loop_start &&
                    vc->loop_end <= end) {
                    /* loop wrap */
                    uint32_t loop_off = cur - (start << FRAC_BITS);
                    uint32_t loop_len = (vc->loop_end - vc->loop_start)
                                        << FRAC_BITS;
                    cur = (vc->loop_start << FRAC_BITS) +
                          (loop_off % (loop_len ? loop_len : 1));
                    vc->pos_fp = cur;
                } else {
                    vc->state = VOICE_STATE_IDLE;
                    break;
                }
            }

            int16_t s = sample_at(vc->src + start,
                                  end - start,
                                  cur - (start << FRAC_BITS));

            /* Effect insert */
            s = effects_process(vc->effect, &vc->effect_state, s,
                                vc->effect_param_1, vc->effect_param_2);

            int32_t mixed = (int32_t)s_block[i] +
                            (((int32_t)s * gain) >> 8);
            if (mixed > 32767) mixed = 32767;
            if (mixed < -32768) mixed = -32768;
            s_block[i] = (int16_t)mixed;

            vc->pos_fp += vc->step_fp;
        }
    }

    /* 3. master filter + soft clip */
    for (size_t i = 0; i < AUDIO_BLOCK_FRAMES; i++) {
        int16_t f = one_pole_lp(s_block[i], &s_filter_z, s_master_cutoff);
        s_block[i] = soft_clip((int32_t)f);
    }

    /* 4. push to I2S TX */
    audio_i2s_write(s_block, AUDIO_BLOCK_FRAMES);
}