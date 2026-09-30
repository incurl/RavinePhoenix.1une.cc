/*
 * input.c — button-event dispatcher.
 *
 * Single switch on btn_id. Long-press vs. tap is a secondary axis; the
 * decision of what *each press* means lives in the case body, not in
 * a giant table. We expect this file to grow as v2 UI features land
 * (write mode, sketch picker, tweak mode, effect picker, etc.).
 *
 * Two held-mode mechanisms live here:
 *
 *   (1) `s_bpm_held` — set by BTN_BPM long-press. While held, Knob A
 *       scans BPM. Exits when the button is released. Implemented as
 *       end-of-drain polling against buttons_is_pressed().
 *
 *   (2) The `s_modifiers[]` table — a list of "modifier + number" pairs
 *       modelled on the PO-33's hold-modifier-and-press-a-number idiom.
 *       When a step event arrives while any registered modifier is held,
 *       the corresponding callback fires with the 1..16 step number.
 *       Currently PATTERN is registered; SOUND and FX land as separate
 *       commits in the same shape (see F-006 / F-019 in docs/DESIGN.md).
 */
#include "input.h"
#include "config.h"
#include "esp_log.h"
#include "sequencer/sequencer.h"
#include "ui/buttons.h"
#include "ui/knobs.h"
#include "ui/leds.h"

static const char *TAG = "input";

/* ─── Held-mode state ──────────────────────────────────────────── */

/* (1) BPM-adjust mode — see F-020 / F-021. */
static bool s_bpm_held = false;
static uint8_t s_bpm_knob_last = 0;   /* last knob A reading, for delta detection */

/* (2) Modifier + step dispatcher. Each entry says: "while btn_id is
 * held, route matrix step 1..16 presses to on_step(step_1_to_16)".
 * Step numbers are 1-based (PO-33 convention); callbacks subtract 1
 * before indexing into a 0-based array. */
typedef void (*modifier_step_cb_t)(uint8_t step_1_to_16);

typedef struct {
    uint8_t           btn_id;
    modifier_step_cb_t on_step;
} modifier_binding_t;

static void pattern_on_step(uint8_t step_1_to_16);

static const modifier_binding_t s_modifiers[] = {
    /* PO-33 ground truth (https://github.com/lode/PO-33 README):
     *   "select pattern — hold pattern (⠛) + number"
     *   "change patterns — hold pattern (⠛) + number(s)"
     * Tap and long-press of PATTERN alone are no-ops; the only PO-33
     * behaviour is "hold + number". The chain-build variant lands
     * alongside F-013 (still ❌ missing). */
    { BTN_PATTERN, pattern_on_step },
};

#define MODIFIER_COUNT (sizeof(s_modifiers) / sizeof(s_modifiers[0]))

/* ─── Helpers ──────────────────────────────────────────────────── */

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
static uint16_t knob_a_to_bpm(uint8_t knob_v)
{
    return (uint16_t)(MIN_BPM +
                     ((MAX_BPM - MIN_BPM) * (uint32_t)knob_v + 127) / 255);
}

/* ─── Modifier + step callbacks ────────────────────────────────── */

static void pattern_on_step(uint8_t step_1_to_16)
{
    /* PO-33: "hold PATTERN + number" loads that pattern directly.
     * We map step 1..16 -> pattern 0..15. */
    sequencer_set_pattern((uint8_t)(step_1_to_16 - 1));
}

/* Find the first modifier currently being held, if any. Returns NULL
 * when no modifier is held. */
static const modifier_binding_t *active_modifier(void)
{
    for (size_t i = 0; i < MODIFIER_COUNT; i++) {
        if (buttons_is_pressed(s_modifiers[i].btn_id)) {
            return &s_modifiers[i];
        }
    }
    return NULL;
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

        /* Modifier + step routing (PO-33 "hold + number" idiom).
         * Runs *before* the per-btn switch because we want step
         * presses to be routed even when their default step handler
         * would also fire. Currently no step handler exists, but the
         * ordering keeps the door open for future "step tap in
         * play-mode" handlers to live downstream and inspect
         * active_modifier() themselves if they need to. */
        if (BTN_IS_STEP(ev.btn_id) && !ev.long_press) {
            const modifier_binding_t *m = active_modifier();
            if (m) {
                uint8_t step_1_to_16 =
                    (uint8_t)(ev.btn_id - BTN_STEP1 + 1);
                ESP_LOGI(TAG, "%s + STEP%u",
                         btn_id_str(m->btn_id),
                         (unsigned)step_1_to_16);
                m->on_step(step_1_to_16);
                continue;   /* do not fall through to default handler */
            }
        }

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
            /* No-op. Per PO-33 ground truth (lode/PO-33 README),
             * PATTERN alone does nothing — only "hold + number"
             * selects a pattern, handled above by the modifier
             * dispatcher. Tap-to-cycle was an earlier invention
             * of mine, removed in this commit. */
            break;

        default:
            /* SOUND / REC / FX / WRITE: handlers land in future
             * commits. Step buttons fall through here unless a
             * modifier was held (handled above). */
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