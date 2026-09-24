/*
 * voice.c — voice initialization + trigger helpers.
 */
#include "voice.h"
#include "sample_manager.h"
#include "config.h"
#include "esp_timer.h"
#include <math.h>

void voice_init(voice_t *v)
{
    memset(v, 0, sizeof(*v));
    v->state = VOICE_STATE_IDLE;
}

void voice_trigger(voice_t *v, uint8_t slot, uint8_t note, uint8_t velocity,
                   uint16_t filter_cutoff,
                   effect_id_t fx, uint8_t fx_p1, uint8_t fx_p2)
{
    const int16_t *src = sample_manager_slot_ptr(slot);
    uint32_t       len = sample_manager_slot_len_samples(slot);
    if (!src || len == 0) {
        v->state = VOICE_STATE_IDLE;
        return;
    }

    v->src            = src;
    v->src_len_samples = len;
    v->start          = 0;
    v->end            = len;
    v->loop_start     = 0;
    v->loop_end       = 0;

    /* MIDI note → frequency ratio → Q24.8 step */
    /* Base: note 60 (middle C) plays at 1.0× sample rate. */
    int semi = (int)note - 60;
    double ratio = pow(2.0, semi / 12.0);
    if (ratio < 0.0625) ratio = 0.0625;
    if (ratio > 16.0)   ratio = 16.0;
    v->step_fp  = (uint32_t)(ratio * (double)(1u << 8));
    v->pos_fp   = 0;

    v->velocity       = (velocity == 0) ? 100 : velocity;
    v->filter_cutoff  = filter_cutoff;

    v->effect          = fx;
    v->effect_param_1  = fx_p1;
    v->effect_param_2  = fx_p2;
    effects_reset((uint8_t)fx, &v->effect_state);

    v->start_time_us  = esp_timer_get_time();
    v->state          = VOICE_STATE_PLAYING;
}