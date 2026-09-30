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
#include "ui/leds.h"

static const char *TAG = "input";

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
            /* Tap = +1 BPM, long-press = -1 BPM. The set_bpm() call
             * clamps to [MIN_BPM, MAX_BPM] so we don't have to. */
            if (ev.long_press) {
                sequencer_set_bpm((uint16_t)(sequencer_get_bpm() - 1));
            } else {
                sequencer_set_bpm((uint16_t)(sequencer_get_bpm() + 1));
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
}