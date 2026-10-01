/*
 * sketch_picker.c — sketch picker state machine.
 *
 * The picker is a 16-slot matrix + encoder UI for selecting which
 * sketch to load. State:
 *
 *   s_active      true/false (between enter and exit).
 *   s_count       number of sketches loaded from storage_sketch_list().
 *   s_ids[i]      the 4-hex ID of sketch slot 0..15 (NUL-terminated).
 *   s_highlight    0..15, the current selection.
 *
 * The picker handles:
 *   - Encoder delta (via sketch_picker_tick() called from the
 *     input_drain() post-drain block) -> move highlight.
 *   - Encoder click (consume)           -> load + exit.
 *   - Step 1..16 press (via sketch_picker_on_step() called from the
 *     input_drain() modifier branch) -> load + exit.
 *   - WRITE tap (consumed by input.c)   -> exit.
 *
 * The picker is intentionally a thin layer; storage operations are
 * delegated to storage_sketch_list() + storage_sketch_load() (the
 * latter is currently a stub; full impl lands in Commit 3).
 */
#include "sketch_picker.h"
#include "storage/storage.h"
#include "ui/encoder.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "sketch_pick";

static bool     s_active    = false;
static uint8_t  s_count     = 0;
static char     s_ids[SKETCHES_MAX][SKETCH_ID_LEN + 1];
static int8_t   s_highlight = 0;   /* 0..15, defaults to first entry */

bool sketch_picker_is_active(void) { return s_active; }

esp_err_t sketch_picker_enter(void)
{
    if (s_active) return ESP_OK;   /* idempotent */
    s_count = 0;
    memset(s_ids, 0, sizeof(s_ids));
    esp_err_t err = storage_sketch_list(&s_count, s_ids);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "storage_sketch_list failed: %s", esp_err_to_name(err));
        return err;
    }
    if (s_count > SKETCHES_MAX) s_count = SKETCHES_MAX;
    s_highlight = 0;
    ESP_LOGI(TAG, "picker entered with %d sketches", s_count);
    s_active = true;
    return ESP_OK;
}

void sketch_picker_exit(void)
{
    if (!s_active) return;          /* idempotent */
    ESP_LOGI(TAG, "picker exited (highlight=%d, s_count=%u)",
             (int)s_highlight, (unsigned)s_count);
    s_active = false;
}

/* Pick a sketch by 1-based step index (1..16) and load it.
 * Returns true if the picker should exit as a result (always, since
 * selecting a sketch is a load+exit action). */
bool sketch_picker_on_step(uint8_t step_1_to_16)
{
    if (!s_active) return false;
    if (step_1_to_16 < 1 || step_1_to_16 > SKETCHES_MAX) return false;
    uint8_t idx = (uint8_t)(step_1_to_16 - 1);
    if (idx >= s_count) {
        /* The slot is empty (sketch N doesn't exist). Don't exit; let
         * the user pick another. */
        ESP_LOGW(TAG, "step %u has no sketch (only %u exist)",
                 (unsigned)step_1_to_16, (unsigned)s_count);
        return false;
    }
    ESP_LOGI(TAG, "load sketch[%u] = %s", (unsigned)idx, s_ids[idx]);
    /* Commit 3 will replace this stub with the real load. For now we
     * just acknowledge and exit; the user's selection is logged so
     * they can verify the binding works. */
    storage_sketch_load(s_ids[idx]);   /* currently ESP_ERR_NOT_SUPPORTED */
    sketch_picker_exit();
    return true;
}

/* Called from input_drain() post-drain block (same cadence as
 * s_bpm_held polling). Reads the encoder delta + click and routes
 * them through the picker state machine. */
void sketch_picker_tick(void)
{
    if (!s_active) return;

    /* Encoder delta moves the highlight (modulo s_count). */
    int8_t d = encoder_get_delta();
    if (d != 0 && s_count > 0) {
        int new_h = (int)s_highlight + d;
        /* Wrap into [0, s_count). Modulo that handles negatives via
         * the C convention (% can be negative); correct by adding
         * s_count before % and subtracting if >= s_count. */
        new_h = ((new_h % (int)s_count) + (int)s_count) % (int)s_count;
        s_highlight = (int8_t)new_h;
        ESP_LOGI(TAG, "highlight = %u (id=%s)",
                 (unsigned)s_highlight, s_ids[s_highlight]);
    }

    /* Encoder click: load + exit (mirrors scroll-at-clicked-step). */
    if (encoder_was_clicked() && s_count > 0) {
        ESP_LOGI(TAG, "click load highlight=%u (id=%s)",
                 (unsigned)s_highlight, s_ids[s_highlight]);
        storage_sketch_load(s_ids[s_highlight]);
        sketch_picker_exit();
    }
}
