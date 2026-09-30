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

/* "Active" state set by the SOUND / FX hold-+-number dispatcher in
 * ui/input.c. Consumed by on_step() and by step-tap-with-no-modifier
 * in the dispatcher. Volatile because they're written from the input
 * task (button_scan_task) and read from the audio task (sequencer
 * timer / render-task). On the ESP32-S3 these are aligned single
 * bytes, so atomic loads/stores are guaranteed. */
static volatile uint8_t s_active_slot = 0xFF;   /* 0xFF = none selected */
static volatile uint8_t s_active_fx   = PO33_FX_NONE;

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

void sequencer_cycle_bpm_preset(void)
{
    /* Tap-to-cycle: advance to next preset in sequence regardless of
     * the current BPM. The user gets to Hip Hop by 3 taps from Techno
     * even if they're at, e.g., 95 BPM (knob-adjust territory).
     *
     * We remember the current preset slot in a static so the cycle is
     * deterministic across calls. If the user has used Knob A to land
     * on an off-preset value, the next tap returns to the slot *after*
     * the last preset slot we delivered, not the nearest preset to the
     * current value. This is the PO-33 behaviour. */
    static uint8_t s_preset_idx = 1;   /* start on Disco == DEFAULT_BPM */
    static const uint16_t s_presets[BPM_PRESET_COUNT] = BPM_PRESETS;

    s_preset_idx = (uint8_t)((s_preset_idx + 1) % BPM_PRESET_COUNT);
    sequencer_set_bpm(s_presets[s_preset_idx]);
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

    /* Slot resolution: the per-step slot wins if set, otherwise fall
     * back to whatever SOUND-hold-+-number most recently selected.
     * Either way, 0xFF means "no slot" -> silent step. */
    uint8_t slot = (s.slot_id != 0xFF) ? s.slot_id : s_active_slot;

    /* Effect resolution: per-step wins if non-NONE, else fall back to
     * the FX-hold-+-number selection. PO33_FX_NONE = no effect. */
    po33_fx_t fx = (s.effect != PO33_FX_NONE)
                       ? (po33_fx_t)s.effect
                       : (po33_fx_t)s_active_fx;

    if (slot != 0xFF) {
        amy_bridge_play_note(slot, s.note, s.velocity,
                             fx, s.effect_p1, s.effect_p2);
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

void sequencer_set_active_slot(uint8_t slot)
{
    /* Out-of-range inputs are coerced to "none" rather than rejected.
     * The dispatcher always passes 0..15 (from a step press); UART or
     * future callers could pass 0xFF to mean "clear selection". */
    if (slot >= SLOT_COUNT && slot != 0xFF) slot = 0xFF;
    s_active_slot = slot;
}

uint8_t sequencer_get_active_slot(void) { return s_active_slot; }

void sequencer_set_active_fx(uint8_t fx)
{
    /* Clamp into the enum range. Anything outside PO33_FX_COUNT (incl.
     * the future swing slot) is coerced to PO33_FX_NONE so a stale
     * caller can't poison on_step(). */
    if (fx >= PO33_FX_COUNT) fx = PO33_FX_NONE;
    s_active_fx = (uint8_t)fx;
}

uint8_t sequencer_get_active_fx(void) { return s_active_fx; }

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