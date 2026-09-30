/*
 * input.c — button-event dispatcher.
 *
 * Single switch on btn_id. Long-press vs. tap is a secondary axis; the
 * decision of what *each press* means lives in the case body, not in
 * a giant table. We expect this file to grow as v2 UI features land
 * (write mode, sketch picker, tweak mode, effect picker, etc.).
 *
 * Currently only logs each event. The transport-level wiring (PLAY,
 * BPM, PATTERN) lands in a follow-up commit so this commit can stay
 * focused on "stop silently filling the queue".
 */
#include "input.h"
#include "config.h"
#include "esp_log.h"

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
        /* Real handler routing lands in the next commit. */
    }
}