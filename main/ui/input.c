/*
 * input.c — button-event dispatcher.
 *
 * Single switch on btn_id. Long-press vs. tap is a secondary axis; the
 * decision of what *each press* means lives in the case body, not in
 * a giant table. We expect this file to grow as v2 UI features land
 * (write mode, sketch picker, tweak mode, effect picker, etc.).
 */
#include "input.h"
#include "config.h"
#include "esp_log.h"
#include "sequencer/sequencer.h"
#include "ui/buttons.h"
#include "ui/knobs.h"
#include "ui/leds.h"

static const char *TAG = "input";

/* Held-mode state. While `s_bpm_held` is true we re-read Knob A on
 * every input_drain() call and map 0..255 to [MIN_BPM, MAX_BPM],
 * updating the sequencer. We exit the mode on the next drain where
 * buttons_is_pressed(BTN_BPM) returns false. */
static bool s_bpm_held = false;
static uint8_t s_bpm_knob_last = 0;   /* last knob A reading, for delta detection */

static const char *btn_id_str(uint8_t id)
{
    /* Modifier buttons (0..6) and step buttons (7..22). */
    switch (id) {
    case BTN_SOUND:    return "SOUND";
    case BTN_PATTERN:  return "PATTERN";
    case BTN_BPM:      return "BPM";
    case BTN_REC:      return "REC";
    case BTN_FX:       return "FX";
    case BTN_PLAY:     return "PLAY";
    case BTN_WRITE:    return "WRITE";
    default:
        if (BTN_IS_STEP(id)) {
            /* BTN_STEP1..BTN_STEP16 in row-major order. */
            static __thread char buf[12];
            uint8_t step = (uint8_t)(id - BTN_STEP1 + 1);
            snprintf(buf, sizeof(buf), "STEP%u", (unsigned)step);
            return buf;
        }
        return "?";
    }
}

/* Map Knob A's 0..255 reading to a BPM in [MIN_BPM, MAX_BPM]. */
static uint16_t knob_a_to_bpm(uint8_t knob)
{
    uint32_t span = MAX_BPM - MIN_BPM;       /* 180 */
    return (uint16_t)(MIN_BPM + (span * knob + 127) / 255);
}

void input_drain(void)
{
    /* Drain every queued event. The queue is 64 deep and we run at
     * ~10 ms cadence, so a worst-case button burst during a brief
     * scan gap can still fit. If we ever grow past 64 we'd need a
     * back-pressure story; not a concern at v1. */
    for (;;) {
        button_event_t ev = buttons_pop();
        if (ev.btn_id == 0xFF) break;   /* queue empty sentinel */

        ESP_LOGI(TAG, "btn %s%s",
                 btn_id_str(ev.btn_id),
                 ev.long_press ? " (long)" : "");

        switch (ev.btn_id) {
        case BTN_PLAY:
            /* Tap to toggle transport. The PO-33's PLAY key is also
             * a tap-to-toggle; the PLAY LED mirrors the new state. */
            if (ev.long_press) break;  /* no long-press action */
            if (sequencer_is_playing()) {
                sequencer_stop();
                leds_set_play(false);
            } else {
                sequencer_play();
                leds_set_play(true);
            }
            break;

        case BTN_BPM:
            if (ev.long_press) {
                /* Long-press enters BPM-adjust mode. While the button
                 * stays held, Knob A scans the BPM range (see below).
                 * The first call snaps to the knob's current position
                 * so the user gets immediate feedback, then continues
                 * to track on each subsequent knob-tick. */
                s_bpm_held = true;
                s_bpm_knob_last = knobs_get_a();
                sequencer_set_bpm(knob_a_to_bpm(s_bpm_knob_last));
            } else {
                /* Tap cycles to the next preset level (F-021):
                 * Hip Hop (80) -> Disco (120) -> Techno (140) -> wrap.
                 * Cycling is independent of the current BPM value;
                 * see sequencer_cycle_bpm_preset(). */
                sequencer_cycle_bpm_preset();
            }
            break;

        case BTN_PATTERN:
            /* Tap = next pattern, long-press = previous pattern.
             * Modulo PATTERN_COUNT so wrap-around is the user's job
             * (matches the PO-33, where the chain is the only way to
             * jump past 15 without scrolling). */
            if (ev.long_press) {
                uint8_t cur = sequencer_get_current_pattern();
                sequencer_set_pattern((uint8_t)((cur ? cur : PATTERN_COUNT) - 1));
            } else {
                uint8_t cur = sequencer_get_current_pattern();
                sequencer_set_pattern((uint8_t)((cur + 1) % PATTERN_COUNT));
            }
            break;

        default:
            /* SOUND / REC / FX / WRITE / steps: handler lands later. */
            break;
        }
    }

    /* Held-mode polling. Runs at the end of every input_drain() call,
     * i.e. once per button-scan tick (~10 ms). If the BPM button was
     * long-pressed earlier and is still held, track Knob A and update
     * the sequencer. Exiting the mode is automatic on release. */
    if (s_bpm_held) {
        if (!buttons_is_pressed(BTN_BPM)) {
            s_bpm_held = false;
            ESP_LOGI(TAG, "BPM-adjust mode exit (BPM=%u)",
                     sequencer_get_bpm());
        } else {
            uint8_t k = knobs_get_a();
            int delta = (int)k - (int)s_bpm_knob_last;
            if (delta < 0) delta = -delta;
            /* KNOB_DEADZONE from config.h avoids jitter from ADC noise. */
            if (delta > KNOB_DEADZONE) {
                sequencer_set_bpm(knob_a_to_bpm(k));
                s_bpm_knob_last = k;
            }
        }
    }
}