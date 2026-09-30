/*
 * sequencer.c — 16-step sequencer driving AMY via amy_bridge.
 *
 * Uses esp_timer for stepping; can be upgraded to gptimer_handle_t +
 * gptimer_register_event_callbacks when target toolchain confirms
 * symbol availability.
 */
#include "sequencer.h"
#include "pattern.h"
#include "audio/amy_bridge.h"
#include "config.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>

static const char *TAG = "seq";

static volatile bool s_playing   = false;
static volatile uint8_t s_step   = 0;
static volatile uint8_t s_pattern = 0;
static volatile uint16_t s_bpm   = DEFAULT_BPM;

static esp_timer_handle_t s_step_timer = NULL;

esp_err_t sequencer_init(void)
{
    pattern_init_all();
    s_bpm = DEFAULT_BPM;
    s_step = 0;
    s_pattern = 0;

    const esp_timer_create_args_t cfg = {
        .name    = "seq_step",
        .callback = NULL,    /* set in sequencer_play */
        .arg     = NULL,
        .skip_unhandled_events = false,
    };
    esp_err_t err = esp_timer_create(&cfg, &s_step_timer);
    if (err != ESP_OK) return err;

    /* default demo pattern: kick on every quarter */
    for (int i = 0; i < STEPS_PER_PATTERN; i += 4) {
        sequencer_set_step_slot(0, i, 0, 60);
    }
    ESP_LOGI(TAG, "Sequencer ready.");
    return ESP_OK;
}

void sequencer_set_bpm(uint16_t bpm)
{
    if (bpm < MIN_BPM) bpm = MIN_BPM;
    if (bpm > MAX_BPM) bpm = MAX_BPM;
    s_bpm = bpm;
    if (s_playing) {
        /* restart timer at new rate */
        sequencer_play();
    }
}

void sequencer_set_pattern(uint8_t pattern)
{
    if (pattern >= PATTERN_COUNT) return;
    s_pattern = pattern;
    s_step = 0;
}

uint16_t sequencer_get_bpm(void)         { return s_bpm; }
uint8_t  sequencer_get_current_step(void){ return s_step; }
uint8_t  sequencer_get_current_pattern(void){ return s_pattern; }

void sequencer_set_step_slot(uint8_t pattern, uint8_t step,
                             uint8_t slot, uint8_t note)
{
    if (pattern >= PATTERN_COUNT || step >= STEPS_PER_PATTERN) return;
    g_patterns[pattern].steps[step].slot_id   = slot;
    g_patterns[pattern].steps[step].note      = note;
    g_patterns[pattern].steps[step].velocity  = 100;
    g_patterns[pattern].steps[step].plock_active = true;
}

static void on_step(void *arg)
{
    (void)arg;
    step_t s;
    pattern_get_step(s_pattern, s_step, &s);
    if (s.slot_id != 0xFF) {
        amy_bridge_play_note(s.slot_id, s.note, s.velocity,
                             (po33_fx_t)s.effect,
                             s.effect_p1, s.effect_p2);
    }
    s_step++;
    if (s_step >= STEPS_PER_PATTERN) {
        s_step = 0;
        if (g_chain_len > 0 && s_pattern == g_chain[g_chain_len - 1]) {
            static uint8_t chain_idx = 0;
            chain_idx = (chain_idx + 1) % g_chain_len;
            s_pattern = g_chain[chain_idx];
        }
    }
}

void sequencer_play(void)
{
    if (!s_step_timer) return;
    esp_timer_stop(s_step_timer);

    /* 16th note interval at current BPM:
     *   one beat = 60/BPM sec
     *   16th note = beat/4 = 15/BPM sec = 15000/BPM ms
     */
    uint64_t period_us = (1000000ULL * 15ULL) / s_bpm;
    if (period_us < 1000) period_us = 1000;

    /* Re-create timer if callback was NULL at init time. */
    /* (We use esp_timer_create once and just re-start; the callback
     * is set at creation. To keep things simple we create a fresh
     * timer with the right callback every time we (re)start.) */
    if (s_step_timer) {
        esp_timer_delete(s_step_timer);
        s_step_timer = NULL;
    }
    const esp_timer_create_args_t cfg = {
        .name = "seq_step",
        .callback = on_step,
        .arg = NULL,
        .skip_unhandled_events = true,
    };
    esp_timer_create(&cfg, &s_step_timer);
    esp_timer_start_periodic(s_step_timer, period_us);

    s_playing = true;
    s_step = 0;
    ESP_LOGI(TAG, "Play BPM=%u period_us=%llu", s_bpm, period_us);
}

void sequencer_stop(void)
{
    if (s_step_timer) {
        esp_timer_stop(s_step_timer);
    }
    s_playing = false;
    ESP_LOGI(TAG, "Stop.");
}

bool sequencer_is_playing(void) { return s_playing; }

void sequencer_tick(void)
{
    /* Called every render block. For now we let the timer fire on_step(). */
}

void sequencer_print_status(void)
{
    printf("seq: %s bpm=%u step=%u pattern=%u\n",
           s_playing ? "playing" : "stopped",
           s_bpm, s_step, s_pattern);
}