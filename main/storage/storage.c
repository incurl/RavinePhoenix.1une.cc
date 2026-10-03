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
#include "sketch/sketch_name.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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

/* ─── Multi-sketch (v2) full implementation ───────────────────────── */

/* Layout (per docs/DESIGN.md §11.2, simplified for v2):
 *
 *   /sketches.lst               master file -- "<id>\t<name>\n" per line
 *   /<id>/meta.bin              fixed 16-byte name (NUL-padded)
 *   /<id>/patterns.bin          16 * sizeof(pattern_t) raw
 *   /<id>/chain.bin             1 byte (len) + len bytes (entries)
 *
 * No JSON, no atomic write to a temp file (we trust LittleFS power-
 * safe semantics for v2; the docs note this as a future hardening).
 */

#define SKETCH_META_LEN SKETCH_NAME_MAX   /* alias to config.h's
                                            * SKETCH_NAME_MAX (24). The
                                            * on-flash sketches.lst
                                            * format stores the
                                            * Docker-style jazz name
                                            * (e.g. "bebop_coltrane")
                                            * in this slot. The
                                            * display length and the
                                            * wire-format length are
                                            * the same now. */

/* Allocate the next 4-hex ID by scanning sketches.lst for max(id)+1. */
static int next_free_id(void)
{
    FILE *f = fopen("/sketches/sketches.lst", "r");
    if (!f) return 1;   /* first sketch ever */
    char buf[512];
    int max = -1;
    while (fgets(buf, sizeof(buf), f)) {
        char *p = buf;
        if (*p == '#' || *p == '\n' || *p == '\0') continue;
        int v = 0;
        int n = 0;
        while (n < SKETCH_ID_LEN && *p &&
               ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') ||
                (*p >= 'A' && *p <= 'F'))) {
            v = (v << 4) | ((*p >= '0' && *p <= '9') ? *p - '0' :
                              (*p >= 'a' && *p <= 'f') ? *p - 'a' + 10 :
                              *p - 'A' + 10);
            p++; n++;
        }
        if (n == SKETCH_ID_LEN && v > max) max = v;
    }
    fclose(f);
    return max < 0 ? 1 : max + 1;
}

/* Re-read the sketches.lst index. Public API. */
esp_err_t storage_sketch_list(uint8_t *out_count,
                              char (*out_ids)[SKETCH_ID_LEN + 1])
{
    if (!out_count || !out_ids) return ESP_ERR_INVALID_ARG;
    *out_count = 0;
    FILE *f = fopen("/sketches/sketches.lst", "r");
    if (!f) {
        strcpy(out_ids[0], "0001");
        *out_count = 1;
        return ESP_OK;
    }
    char line[64];
    uint8_t n = 0;
    while (n < SKETCHES_MAX && fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') continue;
        char *p = line;
        uint8_t i = 0;
        while (i < SKETCH_ID_LEN && *p &&
               ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') ||
                (*p >= 'A' && *A' <= 'F'))) {
            out_ids[n][i++] = *p;
        }
        out_ids[n][i] = '\0';
        if (i == SKETCH_ID_LEN) n++;
    }
    fclose(f);
    *out_count = n;
    if (n == 0) {
        strcpy(out_ids[0], "0001");
        *out_count = 1;
    }
    return ESP_OK;
}

/* Write one sketch entry to sketches.lst. The caller passes the
 * current full list; we rewrite atomically by writing to a temp
 * file and renaming over. */
static esp_err_t write_index(uint8_t count,
                              char (*ids)[SKETCH_ID_LEN + 1],
                              char (*names)[SKETCH_META_LEN])
{
    FILE *f = fopen("/sketches/sketches.lst.tmp", "w");
    if (!f) return ESP_FAIL;
    fprintf(f, "# PO-33 K.O! sketch index (auto-generated)\n");
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\t%.*s\n", ids[i], SKETCH_META_LEN, names[i]);
    }
    fclose(f);
    /* LittleFS supports rename; this is the power-safe atomic swap. */
    if (rename("/sketches/sketches.lst.tmp",
               "/sketches/sketches.lst") != 0) {
        /* rename() failed (target may not exist yet on first call) */
        if (remove("/sketches/sketches.lst") != 0 && errno != ENOENT) {
            /* ignore -- the destination may simply not exist yet */
        }
        if (rename("/sketches/sketches.lst.tmp",
                   "/sketches/sketches.lst") != 0) {
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

/* Read all sketches' IDs + names from the index. Caller-provided
 * buffer for IDs + names. */
static esp_err_t read_index_full(uint8_t *out_count,
                                 char (*out_ids)[SKETCH_ID_LEN + 1],
                                 char (*out_names)[SKETCH_META_LEN])
{
    *out_count = 0;
    FILE *f = fopen("/sketches/sketches.lst", "r");
    if (!f) return ESP_OK;
    char line[64];
    uint8_t n = 0;
    while (n < SKETCHES_MAX && fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') continue;
        char *p = line;
        uint8_t i = 0;
        while (i < SKETCH_ID_LEN && *p &&
               ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') ||
                (*p >= 'A' && *A' <= 'F'))) {
            out_ids[n][i++] = *p++;
        }
        out_ids[n][i] = '\0';
        /* Skip past the tab + name. */
        if (*p == '\t') p++;
        for (uint8_t j = 0; j < SKETCH_META_LEN; j++) {
            out_names[n][j] = (*p && *p != '\n' && *p != '\r') ? *p++ : 0;
        }
        if (i == SKETCH_ID_LEN) n++;
    }
    fclose(f);
    *out_count = n;
    return ESP_OK;
}

/* Read the patterns.bin + chain.bin for sketch `id_str` into the
 * active g_patterns[] + g_chain[]. */
esp_err_t storage_sketch_load(const char *id_str)
{
    if (!id_str) return ESP_ERR_INVALID_ARG;
    char patterns_path[64];
    char chain_path[64];
    snprintf(patterns_path, sizeof(patterns_path), "/sketches/%s/patterns.bin", id_str);
    snprintf(chain_path,    sizeof(chain_path),    "/sketches/%s/chain.bin",    id_str);

    /* Patterns: 16 * sizeof(pattern_t). */
    FILE *fp = fopen(patterns_path, "rb");
    if (!fp) return ESP_ERR_NOT_FOUND;
    if (fread(g_patterns, sizeof(pattern_t), PATTERN_COUNT, f) != PATTERN_COUNT) { fclose(fp); return ESP_FAIL; }
    fclose(fp);

    /* Chain: 1 byte length + N entries. */
    FILE *fc = fopen(chain_path, "rb");
    if (fc) {
        uint8_t len = 0;
        if (fread(&len, 1, 1, fc) == 1) {
            if (len > PATTERN_CHAIN_MAX) len = PATTERN_CHAIN_MAX;
            g_chain_len = len;
            if (len > 0) fread(g_chain, 1, len, fc);
        }
        fclose(fc);
    } else {
        g_chain_len = 0;
    }
    return ESP_OK;
}

/* Write current g_patterns[] + g_chain[] to the sketch folder for
 * `id_str`. */
esp_err_t storage_sketch_save_active(void)
{
    /* Allocate an ID if none exists yet. For v2 simplicity we save
     * to the lowest-empty slot, OR create a new one if the index
     * doesn't already have an "active" pointer. Simplest: the caller
     * picks the ID (we don't track "active sketch" yet). For the
     * simplest v2 behaviour, save to the most-recently-created sketch
     * OR create one. Since we don't track "most recent", we just
     * create a new sketch each time save is called. The user
     * manages which sketch via sketch new/load via UART.
     */
    int id = next_free_id();
    char id_str[SKETCH_ID_LEN + 1];
    snprintf(id_str, sizeof(id_str), "%04x", id);

    char dir_path[64], patterns_path[80], chain_path[80], meta_path[80];
    snprintf(dir_path,      sizeof(dir_path),      "/sketches/%s", id_str);
    snprintf(patterns_path, sizeof(patterns_path), "/sketches/%s/patterns.bin", id_str);
    snprintf(chain_path,    sizeof(chain_path),    "/sketches/%s/chain.bin",    id_str);
    snprintf(meta_path,      sizeof(meta_path),      "/sketches/%s/meta.bin",      id_str);

    /* Ensure the folder exists. LittleFS mkdir returns success if the
     * directory already exists. */
    mkdir(dir_path, 0755);

    /* Patterns. */
    FILE *fp = fopen(patterns_path, "wb");
    if (!fp) return ESP_FAIL;
    if (fwrite(g_patterns, sizeof(pattern_t), PATTERN_COUNT, fp) != PATTERN_COUNT) {
        fclose(fp); return ESP_FAIL;
    }
    fclose(fp);

    /* Chain. */
    FILE *fc = fopen(chain_path, "wb");
    if (!fc) return ESP_FAIL;
    uint8_t len = g_chain_len > PATTERN_CHAIN_MAX ? PATTERN_CHAIN_MAX : g_chain_len;
    fwrite(&len, 1, 1, fc);
    if (len > 0) fwrite(g_chain, 1, len, fc);
    fclose(fc);

    /* Meta: auto-name via the Docker-style jazz generator.
     * See main/sketch/sketch_name.h for the format. We collect the
     * existing names as `taken` so the generator picks the
     * lexicographically smallest free canonical name (and falls back
     * to "_N" suffixes only if every canonical name is in use). */
    char name[SKETCH_META_LEN];
    uint8_t cur_count = 0;
    char ids[SKETCHES_MAX][SKETCH_ID_LEN + 1];
    char names[SKETCHES_MAX][SKETCH_META_LEN];
    read_index_full(&cur_count, ids, names);

    /* Build a flat array of C-string pointers into `names` so
     * sketch_name_next_free() can iterate. */
    const char *taken[SKETCHES_MAX];
    for (uint8_t i = 0; i < cur_count; i++) taken[i] = names[i];
    /* Sketch_name.h requires the array to end at `taken_count`, so
     * any unused slots after `cur_count` are simply past the count. */

    if (!sketch_name_next_free(name, sizeof(name), taken, cur_count)) {
        /* Namespace exhausted (would need >19,008 sketches). */
        ESP_LOGE(TAG, "sketch-name namespace exhausted");
        return ESP_ERR_NO_MEM;
    }

    FILE *fm = fopen(meta_path, "wb");
    if (fm) {
        fwrite(name, 1, SKETCH_META_LEN, fm);
        fclose(fm);
    }

    /* Append to sketches.lst. */
    char ids_all[SKETCHES_MAX + 1][SKETCH_ID_LEN + 1];
    char names_all[SKETCHES_MAX + 1][SKETCH_META_LEN];
    for (int i = 0; i < cur_count; i++) {
        memcpy(ids_all[i], ids[i], SKETCH_ID_LEN + 1);
        memcpy(names_all[i], names[i], SKETCH_META_LEN);
    }
    memcpy(ids_all[cur_count],   id_str, SKETCH_ID_LEN + 1);
    memcpy(names_all[cur_count], name,   SKETCH_META_LEN);
    cur_count++;

    return write_index(cur_count, ids_all, names_all);
}

esp_err_t storage_sketch_create(void)
{
    /* Same as save_active, but uses the next ID without forcing the
     * caller to know it. */
    return storage_sketch_save_active();
}

esp_err_t storage_sketch_delete(const char *id_str)
{
    if (!id_str) return ESP_ERR_INVALID_ARG;

    /* Read the current index, skip the deleted one, write back. */
    uint8_t count = 0;
    char ids[SKETCHES_MAX + 1][SKETCH_ID_LEN + 1];
    char names[SKETCHES_MAX + 1][SKETCH_META_LEN];
    esp_err_t err = read_index_full(&count, ids, names);
    if (err != ESP_OK) return err;

    uint8_t new_count = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(ids[i], id_str) == 0) continue;
        memcpy(ids[new_count],   ids[i],   SKETCH_ID_LEN + 1);
        memcpy(names[new_count], names[i], SKETCH_META_LEN);
        new_count++;
    }

    err = write_index(new_count, ids, names);
    if (err != ESP_OK) return err;

    /* Best-effort delete of the sketch folder. (We don't recursively
     * rm since vfs doesn't expose opendirf easily; patterns.bin +
     * chain.bin + meta.bin are individually unlinked.) */
    char path[80];
    snprintf(path, sizeof(path), "/sketches/%s/patterns.bin", id_str); remove(path);
    snprintf(path, sizeof(path), "/sketches/%s/chain.bin",    id_str); remove(path);
    snprintf(path, sizeof(path), "/sketches/%s/meta.bin",      id_str); remove(path);
    char dir_path[64];
    snprintf(dir_path, sizeof(dir_path), "/sketches/%s", id_str);
    rmdir(dir_path);

    return ESP_OK;
}
