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
 *   (1) `s_bpm_held` — set by BTN_BPM long-press. While held, the BPM
 *       row maps:
 *           Knob A  -> swing    (8 discrete levels, 0..7)
 *           Knob B  -> fine BPM (continuous, [MIN_BPM..MAX_BPM])
 *       Per PO-33 ground truth (lode/PO-33 README): "change swing" is
 *       BPM + Knob A; "change tempo (fine tuned)" is BPM + Knob B.
 *       Both knobs are polled at the end of every drain; changes
 *       past KNOB_DEADZONE apply immediately. Exits when the button
 *       is released.
 *
 *   (2) The `s_modifiers[]` table — a list of "modifier + number" pairs
 *       modelled on the PO-33's hold-modifier-and-press-a-number idiom.
 *       When a step event arrives while any registered modifier is held,
 *       the corresponding callback fires with the 1..16 step number.
 *       Currently PATTERN, SOUND, FX are registered (see F-007 / F-006
 *       / F-019 in docs/DESIGN.md).
 */
#include "input.h"
#include "config.h"
#include "esp_log.h"
#include "audio/amy_bridge.h"
#include "sequencer/sequencer.h"
#include "ui/buttons.h"
#include "ui/knobs.h"
#include "ui/leds.h"

static const char *TAG = "input";

/* ─── Held-mode state ──────────────────────────────────────────── */

/* (1) BPM-adjust mode — see F-020 / F-021.
 *
 * Knob A adjusts swing (discrete 0..7); Knob B fine-tunes BPM
 * continuously. Each knob has its own "last reading" so delta
 * detection doesn't conflate the two axes. */
static bool     s_bpm_held         = false;
static uint8_t  s_bpm_knob_a_last  = 0;
static uint8_t  s_bpm_knob_b_last  = 0;

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

    /* PO-33: "select sound — hold sound (S) + number"
     *        "play a sound — [select sound] press number"
     * SOUND + step sets the active slot. The PLAY half fires in the
     * no-modifier branch of input_drain(). */
    { BTN_SOUND, sound_on_step },

    /* PO-33: "hold fx (FX) + number (1-15) — add & save effect in pattern"
     *        "[write mode] [play] hold fx (FX) + 16 — clear effect in pattern"
     *
     * Step 1..15 maps to PO33_FX_LOOP_16..PO33_FX_FILTER_SWEEP. Step 16
     * is PO-33 entry "no effect" (PO33_FX_NONE); the "clear effect in
     * pattern" combo behind it lives in write mode which is queued.
     * Swing is NOT a step press — it's BPM-held + Knob A (see the
     * s_bpm_held polling below). */
    { BTN_FX, fx_on_step },
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

/* Map Knob A's 0..255 reading to a discrete swing level in
 * [0..SWING_LEVELS-1]. We bucket the knob range into SWING_LEVELS
 * equal slices (32 knob-units per level at SWING_LEVELS=8).
 * The PO-33 community uses 8 swing levels (0=no swing, 7=max). */
static uint8_t knob_a_to_swing(uint8_t knob_v)
{
    /* (knob_v * SWING_LEVELS + 128) / 256 would also work and would
     * be more precise for the midpoint, but a simple slice is what
     * the PO-33's own detented knob feels like. */
    return (uint8_t)((uint32_t)knob_v * SWING_LEVELS / 256);
}

/* Map Knob B's 0..255 reading to a continuous BPM in [MIN_BPM,
 * MAX_BPM]. Same rounding trick used by the previous knob_a_to_bpm
 * helper so 0 -> MIN_BPM and 255 -> MAX_BPM land exactly. */
static uint16_t knob_b_to_bpm(uint8_t knob_v)
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

static void sound_on_step(uint8_t step_1_to_16)
{
    /* PO-33 strict two-step flow (lode/PO-33 manual):
     *   "select sound — hold sound (S) + number"
     *   "play a sound — [select sound] press number"
     * We only implement the SELECT step here. The PLAY step fires
     * below in input_drain() when a step press arrives WITHOUT any
     * modifier held and the active slot is non-0xFF.
     *
     * Note: docs/DESIGN.md F-006 'Layman' line ("and that slot's sound
     * plays once") reads as one-press ergonomics, but the PO-33 manual
     * is unambiguous about two presses. We honour the manual. */
    sequencer_set_active_slot((uint8_t)(step_1_to_16 - 1));
}

static void fx_on_step(uint8_t step_1_to_16)
{
    /* PO-33 (lode/PO-33 manual):
     *   "hold fx (FX) + number (1-15) — add & save effect in pattern"
     *   "[write mode] [play] hold fx (FX) + 16 — clear effect in pattern"
     *
     * Step 1..15 maps to PO33_FX_LOOP_16 .. PO33_FX_FILTER_SWEEP (the
     * first 15 enum values, in order). Step 16 is the PO-33 manual's
     * entry "no effect" (PO33_FX_NONE) — the 16th row of the manual's
     * effects list reads "16. no effect". We set PO33_FX_NONE on the
     * active FX; the PO-33 hardware treats this as "clear the punch-in
     * for the next note" (the write-mode half of the combo lands later).
     *
     * Selecting a new FX replaces, not stacks. The stored value is then
     * consumed by sequencer_on_step() (via s_active_fx) for every note
     * triggered until something else overwrites it.
     *
     * Note: swing is *not* this combo. Per the PO-33 manual, "change
     * swing" is BPM-held + Knob A, handled by the s_bpm_held polling
     * below. Earlier commits mapped FX + step 16 to swing; that was a
     * fabrication — the manual is clear that swing is a knob-twist. */
    if (step_1_to_16 >= 1 && step_1_to_16 <= 15) {
        uint8_t fx = (uint8_t)(PO33_FX_LOOP_16 + (step_1_to_16 - 1));
        sequencer_set_active_fx(fx);
    } else {
        /* step 16 = "no effect". Clear active FX. */
        sequencer_set_active_fx((uint8_t)PO33_FX_NONE);
    }
}

/* Trigger the currently selected slot. Called from input_drain() when
 * a step event arrives with no modifier held. If no slot is selected
 * yet (0xFF), this is a no-op. Uses midi_note = 60 (middle C) and
 * velocity = 100 — same defaults sequencer_set_step_slot() uses. */
static void play_active_slot(void)
{
    uint8_t slot = sequencer_get_active_slot();
    if (slot == 0xFF) return;
    amy_bridge_play_note(slot, 60, 100,
                         (po33_fx_t)sequencer_get_active_fx(), 0, 0);
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
         * would also fire. */
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
            /* No modifier held + step pressed: PO-33 second-press
             * play-the-selected-sound. No-op if nothing is selected. */
            play_active_slot();
            continue;
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
                s_bpm_knob_a_last = knobs_get_a();
                s_bpm_knob_b_last = knobs_get_b();
                /* Snap both axes to the current knob positions for
                 * immediate user feedback. Subsequent drain polls
                 * update only on actual knob movement past the
                 * deadzone. */
                sequencer_set_swing(knob_a_to_swing(s_bpm_knob_a_last));
                sequencer_set_bpm(knob_b_to_bpm(s_bpm_knob_b_last));
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

        case BTN_SOUND:
        case BTN_FX:
            /* No-op as a bare button. Their hold-+-NUMBER dispatch
             * lives in the s_modifiers[] table above. */
            break;

        default:
            /* REC / WRITE: handlers land in future commits. */
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
            ESP_LOGI(TAG, "BPM mode exit (BPM=%u swing=%u)",
                     sequencer_get_bpm(), sequencer_get_swing());
        } else {
            /* Knob A -> swing (8 discrete levels). Knob B -> BPM
             * (continuous fine tempo). Each axis has its own deadzone
             * + last-reading for delta detection. */
            uint8_t ka = knobs_get_a();
            int da = (int)ka - (int)s_bpm_knob_a_last;
            if (da < 0) da = -da;
            if (da > KNOB_DEADZONE) {
                sequencer_set_swing(knob_a_to_swing(ka));
                s_bpm_knob_a_last = ka;
            }
            uint8_t kb = knobs_get_b();
            int db = (int)kb - (int)s_bpm_knob_b_last;
            if (db < 0) db = -db;
            if (db > KNOB_DEADZONE) {
                sequencer_set_bpm(knob_b_to_bpm(kb));
                s_bpm_knob_b_last = kb;
            }
        }
    }
}