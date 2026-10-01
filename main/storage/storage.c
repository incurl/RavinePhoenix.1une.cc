/*
 * storage.c — LittleFS partition for sketches.
 *
 * v6.0 note: LittleFS is a component (joltwallet/littlefs) in
 * idf_component.yml, not an in-tree driver.
 */
#include "storage.h"
#include "config.h"
#include "sequencer/pattern.h"
#include "audio/amy_bridge.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static const char *TAG = "storage";

#define SKETCHES_BASE "/sketches"

static esp_littlefs_handle_t s_sketches_fs = NULL;

esp_err_t storage_init(void)
{
    /* Samples live entirely in PSRAM; only sketches are persisted. */
    esp_vfs_littlefs_conf_t sketches_conf = {
        .base_path          = SKETCHES_BASE,
        .partition_label    = "sketches",
        .format_if_mount_failed = true,
        .dont_mount         = false,
    };
    ESP_ERROR_CHECK(esp_littlefs_create(&sketches_conf, &s_sketches_fs));

    ESP_LOGI(TAG, "LittleFS mounted at %s", SKETCHES_BASE);
    return ESP_OK;
}

esp_err_t storage_mount(void)
{
    /* esp_littlefs_create() in v6.0 already mounts; this is a no-op
     * placeholder for symmetry / future hot-remount logic. */
    return ESP_OK;
}

esp_err_t storage_save_pattern(uint8_t idx)
{
    if (idx >= PATTERN_COUNT) return ESP_ERR_INVALID_ARG;
    char path[64];
    snprintf(path, sizeof(path), "%s/p%u.bin", SKETCHES_BASE, idx);
    FILE *f = fopen(path, "wb");
    if (!f) return ESP_FAIL;
    size_t w = fwrite(&g_patterns[idx], 1, sizeof(pattern_t), f);
    fclose(f);
    return (w == sizeof(pattern_t)) ? ESP_OK : ESP_FAIL;
}

esp_err_t storage_load_pattern(uint8_t idx)
{
    if (idx >= PATTERN_COUNT) return ESP_ERR_INVALID_ARG;
    char path[64];
    snprintf(path, sizeof(path), "%s/p%u.bin", SKETCHES_BASE, idx);
    FILE *f = fopen(path, "rb");
    if (!f) return ESP_ERR_NOT_FOUND;
    size_t r = fread(&g_patterns[idx], 1, sizeof(pattern_t), f);
    fclose(f);
    return (r == sizeof(pattern_t)) ? ESP_OK : ESP_FAIL;
}

esp_err_t storage_save_all(void)
{
    ESP_LOGI(TAG, "Saving %d step patterns...", PATTERN_COUNT);
    for (int i = 0; i < PATTERN_COUNT; i++) {
        esp_err_t e = storage_save_pattern((uint8_t)i);
        if (e != ESP_OK) {
            ESP_LOGE(TAG, "save pattern %d failed", i);
            return e;
        }
    }
    /* Sample pool save: one raw int16 file per slot. */
    for (int s = 0; s < SLOT_COUNT; s++) {
        const int16_t *p = amy_bridge_slot_ptr((uint8_t)s);
        size_t n = amy_bridge_slot_len_samples((uint8_t)s);
        if (!p || n == 0) continue;

        char path[64];
        snprintf(path, sizeof(path), "%s/s%u.bin",
                 SKETCHES_BASE, (unsigned)s);
        FILE *f = fopen(path, "wb");
        if (!f) return ESP_FAIL;
        fwrite(p, sizeof(int16_t), n, f);
        fclose(f);
    }
    return ESP_OK;
}

esp_err_t storage_load_all(void)
{
    ESP_LOGI(TAG, "Loading step patterns + samples...");
    for (int i = 0; i < PATTERN_COUNT; i++) {
        storage_load_pattern((uint8_t)i);
    }
    for (int s = 0; s < SLOT_COUNT; s++) {
        char path[64];
        snprintf(path, sizeof(path), "%s/s%d.bin",
                 SKETCHES_BASE, s);
        FILE *f = fopen(path, "rb");
        if (!f) continue;
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (sz <= 0) { fclose(f); continue; }
        int16_t *tmp = malloc(sz);
        if (!tmp) { fclose(f); return ESP_ERR_NO_MEM; }
        fread(tmp, 1, sz, f);
        fclose(f);
        amy_bridge_register_slot((uint8_t)s, tmp, sz / sizeof(int16_t),
                                 SAMPLE_RATE_HZ,
                                 s < SLOT_DRUM_COUNT);
        free(tmp);
    }
    return ESP_OK;
}
/* ─── Multi-sketch (v2) APIs ─────────────────────────────────────── */

/* The v2 sketch layout (per docs/DESIGN.md §11.2): a master index
 * /sketches.lst, one line per sketch: "<4-hex-id>\t<name>\n".
 * Per-sketch folder: /<id>/ with p0.bin..p15.bin + s0.bin..s15.bin
 * (same content as the v1 flat layout).
 *
 * For Commit 2, storage_sketch_list() reads the index and returns
 * the IDs (and count). If sketches.lst is missing, we synthesise a
 * single entry so the picker always has at least one sketch visible.
 * The full create/save/delete/load path lands in Commit 3. */
esp_err_t storage_sketch_list(uint8_t *out_count,
                              char (*out_ids)[SKETCH_ID_LEN + 1])
{
    if (!out_count || !out_ids) return ESP_ERR_INVALID_ARG;
    *out_count = 0;

    FILE *f = fopen("/sketches/sketches.lst", "r");
    if (!f) {
        /* Synthesise a default sketch so the picker is never empty. */
        strcpy(out_ids[0], "0001");
        *out_count = 1;
        return ESP_OK;
    }
    char line[64];
    uint8_t n = 0;
    while (n < SKETCHES_MAX && fgets(line, sizeof(line), f)) {
        /* Skip comments / blank lines. */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') continue;
        /* Read up to 4 hex digits into the ID slot. */
        char *p = line;
        uint8_t i = 0;
        while (i < SKETCH_ID_LEN && *p &&
               ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') ||
                (*p >= 'A' && *p <= 'F'))) {
            out_ids[n][i++] = *p;
        }
        out_ids[n][i] = '\0';
        if (i == SKETCH_ID_LEN) {
            n++;
        }
        /* If we couldn't read a full 4-hex ID, skip the entry (corrupt
         * index line). */
    }
    fclose(f);
    *out_count = n;
    if (n == 0) {
        /* Empty or corrupt index: synthesise a default. */
        strcpy(out_ids[0], "0001");
        *out_count = 1;
    }
    return ESP_OK;
}

/* Stubs for Commit 2; full implementation lands in Commit 3. */
esp_err_t storage_sketch_load(const char *id_str)
{
    (void)id_str;
    return ESP_ERR_NOT_SUPPORTED;
}
esp_err_t storage_sketch_create(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}
esp_err_t storage_sketch_save_active(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}
esp_err_t storage_sketch_delete(const char *id_str)
{
    (void)id_str;
    return ESP_ERR_NOT_SUPPORTED;
}
