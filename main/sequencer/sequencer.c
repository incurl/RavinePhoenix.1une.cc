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
#include "ui/input.h"
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

/* F-019 PO33_FX_RETRIGGER_PATTERN: set true by amy_bridge when a
 * retrigger-pattern note is played; on_step() consumes + clears it.
 * The sequencer resets to step 0 (and resets the chain cursor) on the
 * next step tick. */
static volatile bool s_retrigger_requested = false;

/* F-031 sync IN. When true, the local esp_timer step clock is
 * disabled -- the sync listener task drives sequencer_tick() on
 * each incoming pulse. Default false; the sync module sets it true
 * via sequencer_set_sync_in_active() if/when it implements that
 * mode toggle. */
static volatile bool s_sync_in_active = false;

/* Swing level (0..SWING_LEVELS-1). Set by BPM-held + Knob A; consumed
 * by on_step() to delay off-beat 16th notes. */
static volatile uint8_t s_swing = 0;

/* One-shot esp_timer used to play off-beat steps late when swing != 0.
 * NULL until sequencer_init() runs. */
static esp_timer_handle_t s_swing_timer = NULL;

/* The note we owe the swing timer. Set by on_step() before scheduling
 * the timer; read by the timer callback. Same volatile / atomic
 * argument as s_active_*. */
typedef struct {
    uint8_t     slot;
    uint8_t     note;
    uint8_t     velocity;
    po33_fx_t   fx;
} pending_note_t;
static volatile pending_note_t s_pending_note = { 0xFF, 0, 0, PO33_FX_NONE };
static volatile bool           s_pending      = false;

/* Forward decls. The swing timer fires on_swing_fire() to play a
 * pending note late. */
static void on_swing_fire(void *arg);

static esp_timer_handle_t s_step_timer = NULL;

/* Current 16th-note period in microseconds, set by sequencer_play() and
 * read by on_step() to compute swing delays. Volatile because the
 * input task can call sequencer_set_bpm() while the sequencer timer
 * is running. */
static volatile uint32_t s_period_us = 0;

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

    /* One-shot swing timer. Fires on_swing_fire() to play an off-beat
     * note late when swing != 0. We allocate it once; sequencer_play
     * arms it per-step, sequencer_stop() disarms. */
    const esp_timer_create_args_t swing_cfg = {
        .name    = "seq_swing",
        .callback = on_swing_fire,
        .arg     = NULL,
        .skip_unhandled_events = false,
    };
    err = esp_timer_create(&swing_cfg, &s_swing_timer);
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
    } else {
        /* Keep s_period_us in sync even when stopped, so a subsequent
         * sequencer_play() doesn't compute from a stale value. */
        uint32_t p = (1000000U * 15U) / s_bpm;
        if (p < 1000) p = 1000;
        s_period_us = p;
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

void sequencer_chain_append(uint8_t pattern)
{
    /* PO-33 manual: "hold pattern (⠛) + number(s) -> adds each number
     * to the chain; choosing a single pattern multiple times is
     * allowed". We append; refuse if the chain is full so we never
     * overflow the g_chain[] buffer. */
    if (pattern >= PATTERN_COUNT) return;
    if (g_chain_len >= PATTERN_CHAIN_MAX) {
        /* Silent no-op. The caller can pre-check with
         * sequencer_chain_len() if it cares. */
        return;
    }
    g_chain[g_chain_len++] = pattern;
}

void sequencer_chain_clear(void)
{
    g_chain_len = 0;
}

void sequencer_chain_remove(uint8_t pattern)
{
    /* F-014: compact the buffer in place. O(N) sweep, expected N
     * small (<= PATTERN_CHAIN_MAX = 128). */
    size_t w = 0;
    for (size_t r = 0; r < g_chain_len; r++) {
        if (g_chain[r] != pattern) {
            g_chain[w++] = g_chain[r];
        }
    }
    g_chain_len = (uint8_t)w;
    ESP_LOGI(TAG, "chain: removed pattern %u (len now %u)",
             (unsigned)pattern, (unsigned)g_chain_len);
}

size_t sequencer_chain_count(uint8_t pattern)
{
    size_t n = 0;
    for (size_t i = 0; i < g_chain_len; i++) {
        if (g_chain[i] == pattern) n++;
    }
    return n;
}

uint8_t sequencer_chain_at(uint8_t index)
{
    return (index < g_chain_len) ? g_chain[index] : 0xFF;
}

uint8_t sequencer_chain_len(void)
{
    return g_chain_len;
}

void sequencer_clear_current_pattern(void)
{
    /* F-012 (PO-33: "hold record + pattern to clear the active pattern").
     * Wipe the steps; leave the pattern index, name, chain, BPM, and
     * active slot alone. Caller (input.c) is responsible for the
     * long-press timing that gates this gesture. */
    pattern_clear(&g_patterns[s_pattern]);
    ESP_LOGI(TAG, "pattern %u cleared", (unsigned)s_pattern);
}

void sequencer_request_retrigger(void)
{
    /* F-019 PO33_FX_RETRIGGER_PATTERN. Set the flag; on_step()
     * consumes it on the next timer tick. Safe from any context. */
    s_retrigger_requested = true;
}

void sequencer_tick(void)
{
    /* F-031 sync IN. Run the same per-step logic as the esp_timer
     * callback. Safe to call from any context; the esp_timer one
     * won't fire while sync IN is driving the sequencer (the timer
     * is one-shot and we don't restart it on sync-driven steps). */
    on_step(NULL);
}

void sequencer_set_sync_in_active(bool active)
{
    s_sync_in_active = active;
    ESP_LOGI(TAG, "sync IN %s", active ? "ENABLED" : "DISABLED");
}

void sequencer_save_fx_to_pattern(uint8_t fx, uint8_t p1, uint8_t p2)
{
    /* F-019 "save effect in pattern". Set the per-step effect on every
     * step of the active pattern that has a slot bound. */
    int saved = 0;
    for (uint8_t s = 0; s < STEPS_PER_PATTERN; s++) {
        if (g_patterns[s_pattern].steps[s].slot_id != 0xFF) {
            g_patterns[s_pattern].steps[s].effect    = fx;
            g_patterns[s_pattern].steps[s].effect_p1 = p1;
            g_patterns[s_pattern].steps[s].effect_p2 = p2;
            saved++;
        }
    }
    ESP_LOGI(TAG, "saved fx=%u (p1=%u p2=%u) to %d steps of pattern %u",
             (unsigned)fx, (unsigned)p1, (unsigned)p2,
             saved, (unsigned)s_pattern);
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

/* Swing timer callback. Fires `swing_delay_us` after on_step() armed
 * it; plays the pending note that on_step() stashed. If the pending
 * flag is false (e.g. sequencer_stop() ran between arm and fire) we
 * no-op. */
static void on_swing_fire(void *arg)
{
    (void)arg;
    if (!s_pending) return;
    pending_note_t n;
    /* Read volatile struct once, then clear pending. Atomic on the
     * ESP32-S3 for a 4-byte aligned read. */
    n.slot     = s_pending_note.slot;
    n.note     = s_pending_note.note;
    n.velocity = s_pending_note.velocity;
    n.fx       = s_pending_note.fx;
    s_pending  = false;
    if (n.slot != 0xFF) {
        amy_bridge_play_note(n.slot, n.note, n.velocity, n.fx, 0, 0,
                             0, 0);
    }
}

static void on_step(void *arg)
{
    (void)arg;

    /* F-019 PO33_FX_RETRIGGER_PATTERN: a retrigger request was posted
     * between the last tick and this one. Reset the step counter and
     * the chain cursor so the pattern restarts from the top. The
     * `if (g_chain_len > 0 ...)` logic further down already handles
     * chain advancement from the new origin. */
    if (s_retrigger_requested) {
        s_retrigger_requested = false;
        s_step = 0;
    }

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

    /* Swing (PO-33 "change swing" via BPM-held + Knob A):
     *
     *   On-beat steps (even index in 0..15) play immediately.
     *   Off-beat steps (odd index) play delayed by:
     *       delay_us = period_us * swing_level * SWING_MAX_PERCENT
     *                  / 100 / (SWING_LEVELS - 1)
     *   At level 0, delay is 0 (straight timing).
     *   At level 7 with SWING_MAX_PERCENT=50, off-beat is delayed by
     *   half a 16th note (max swing = pure shuffle).
     *
     * Implementation: when an off-beat step is owed a swing delay,
     * stash the note in s_pending_note, arm a one-shot esp_timer, and
     * do NOT play it now. The timer callback plays it later. */
    bool is_offbeat = (s_step % 2) == 1;
    bool defer       = is_offbeat && (s_swing > 0) && (slot != 0xFF);

    /* Tweak-mode override: in Tone or Filter mode, the per-step note /
     * velocity / filter_cutoff / filter_resonance are replaced by the
     * current knob readings. This is the PO-33 "select tweak
     * parameter, turn knobs" idiom (F-016 / F-017). Trim mode is
     * per-slot (handled by tweak_apply_slot() in play_active_slot),
     * not per-step, so we don't apply it here. */
    if (slot != 0xFF) {
        tweak_apply_step(&s.note, &s.velocity,
                         &s.filter_cutoff, &s.filter_resonance);
    }

    if (!defer) {
        if (slot != 0xFF) {
            amy_bridge_play_note(slot, s.note, s.velocity,
                                 fx, s.effect_p1, s.effect_p2,
                                 s.filter_cutoff, s.filter_resonance);
        }
    } else {
        /* Cancel any previous swing note that hasn't fired yet (e.g.
         * the user pressed BPM long-press while a previous off-beat
         * was in flight). The new off-beat replaces it. */
        if (s_swing_timer) esp_timer_stop(s_swing_timer);
        s_pending_note.slot     = slot;
        s_pending_note.note     = s.note;
        s_pending_note.velocity = s.velocity;
        s_pending_note.fx       = fx;
        s_pending               = true;
        uint32_t delay_us = (s_period_us * (uint32_t)s_swing
                            * (uint32_t)SWING_MAX_PERCENT)
                            / (100U * (uint32_t)(SWING_LEVELS - 1));
        if (s_swing_timer && delay_us > 0) {
            esp_timer_start_once(s_swing_timer, delay_us);
        } else {
            /* period_us was 0 (timer not running) or swing math
             * collapsed to zero — play immediately. */
            s_pending = false;
            amy_bridge_play_note(slot, s.note, s.velocity,
                                 fx, s.effect_p1, s.effect_p2,
                                 s.filter_cutoff, s.filter_resonance);
        }
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
    /* Mirror into the volatile so on_step() can compute swing delays. */
    s_period_us = (uint32_t)period_us;

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
    if (!s_sync_in_active) {
        esp_timer_start_periodic(s_step_timer, period_us);
    } else {
        /* F-031: sync IN is driving the sequencer. The local timer
         * stays unstarted so it doesn't double-step. */
        ESP_LOGI(TAG, "Play (sync IN active) -- local timer disabled");
    }

    s_playing = true;
    s_step = 0;
    ESP_LOGI(TAG, "Play BPM=%u period_us=%llu", s_bpm, period_us);
}

void sequencer_stop(void)
{
    if (s_step_timer) {
        esp_timer_stop(s_step_timer);
    }
    if (s_swing_timer) {
        esp_timer_stop(s_swing_timer);
    }
    s_pending = false;
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

void sequencer_set_swing(uint8_t level)
{
    /* Clamp to [0, SWING_LEVELS-1]. Out-of-range values (incl. the
     * future "swing = some byte that didn't come from our dispatcher"
     * case) are coerced to 0 (no swing) so on_step() never sees a
     * bogus level. */
    if (level >= SWING_LEVELS) level = 0;
    s_swing = level;
}

uint8_t sequencer_get_swing(void) { return s_swing; }

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