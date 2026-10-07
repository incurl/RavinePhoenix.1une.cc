/*
 * storage.c — LittleFS partition for sketches, plus the NVS-backed
 * "which sketch is active right now" pointer.
 *
 * v6.0 note: LittleFS is a component (joltwallet/littlefs) in
 * idf_component.yml, not an in-tree driver.
 *
 * On-flash layout (v2 — see docs/DESIGN.md §11.2):
 *
 *     /sketches/sketches.lst                  # "<id>\t<name>\n" per line
 *     /sketches/<id>/
 *       meta.bin                              # 24-byte name (NUL-padded)
 *       samples.bin                           # 256-B slot table + raw PCM
 *       patterns.bin                          # 16 * sizeof(pattern_t)
 *       chain.bin                             # 1-byte len + entries
 *
 * Active-sketch pointer lives in NVS under the "sketches" namespace,
 * key "active_id". storage_load_all() reads it on boot; save_all()
 * and sketch_save_active() update it.
 *
 * ADR-0004: at most one sketch's sample pool is in PSRAM at a time.
 */
#include "storage.h"
#include "config.h"
#include "sequencer/pattern.h"
#include "audio/amy_bridge.h"
#include "sketch/sketch_name.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

/* samples.bin on-flash format
 *
 *   ┌──────────────────────────────────────────────────────────┐
 *   │ slot_table[16]: 16 × 16-byte slot records (256 B total)  │
 *   ├──────────────────────────────────────────────────────────┤
 *   │ pcm[]: raw int16 little-endian, each slot's samples back  │
 *   │        to back in slot order. Empty slots contribute 0 B. │
 *   └──────────────────────────────────────────────────────────┘
 *
 * struct slot_rec (16 bytes, packed, little-endian):
 *
 *   u32 length_samples     0 = empty (no PCM follows for this slot)
 *   u32 sample_rate_hz
 *   u32 start_sample       trim start (sample index into this slot)
 *   u32 end_sample         trim end (sample index, exclusive)
 *
 * This is the *only* place the on-disk sample format is defined;
 * keep storage_load_all(), storage_sketch_load_samples(), and
 * storage_sketch_save_samples() in lockstep.
 */
#define SLOT_REC_SIZE_BYTES     16
#define SLOT_TABLE_SIZE_BYTES   (SLOT_COUNT * SLOT_REC_SIZE_BYTES)  /* 256 */

#define SKETCHES_BASE "/sketches"

/* NVS namespace + key holding the 4-hex ID of the last-active
 * sketch. storage_load_all() reads it on boot; save_all() and
 * sketch_save_active() update it. ADR-0004. These macros must come
 * before storage_factory_reset() (which uses them) -- they're
 * placed up here rather than next to the helpers below so the
 * function order can stay "factory_reset, NVS, helpers". */
#define NVS_NS_SKETCHES   "sketches"
#define NVS_KEY_ACTIVE_ID "active_id"

/* Default sketch ID used on first-ever boot (before any sketch has
 * been saved). */
#define DEFAULT_SKETCH_ID "0000"

/* Length of the on-flash Docker-style jazz sketch name. Aliases
 * config.h's SKETCH_NAME_MAX (24); sketches.lst stores the same
 * bytes the picker displays, so wire-format and display length are
 * equal. */
#define SKETCH_META_LEN SKETCH_NAME_MAX

/* Forward decls for the helpers defined later in this file. They
 * are called from storage_save_all() / storage_load_all() (above),
 * so we declare them up here to avoid an implicit-declaration
 * warning. */
static int          next_free_id(void);
static esp_err_t    write_index(uint8_t count,
                                 char (*ids)[SKETCH_ID_LEN + 1],
                                 char (*names)[SKETCH_META_LEN]);
static esp_err_t    read_index_full(uint8_t *out_count,
                                    char (*out_ids)[SKETCH_ID_LEN + 1],
                                    char (*out_names)[SKETCH_META_LEN]);

static const char *TAG = "storage";

static bool s_sketches_mounted = false;

esp_err_t storage_init(void)
{
    /* Samples live entirely in PSRAM; only sketches are persisted. */
    esp_vfs_littlefs_conf_t sketches_conf = {
        .base_path          = SKETCHES_BASE,
        .partition_label    = "sketches",
        .format_if_mount_failed = true,
        .dont_mount         = false,
    };
    ESP_ERROR_CHECK(esp_vfs_littlefs_register(&sketches_conf));
    s_sketches_mounted = true;

    ESP_LOGI(TAG, "LittleFS mounted at %s", SKETCHES_BASE);
    return ESP_OK;
}

esp_err_t storage_mount(void)
{
    /* esp_littlefs_create() in v6.0 already mounts; this is a no-op
     * placeholder for symmetry / future hot-remount logic. */
    return ESP_OK;
}

esp_err_t storage_factory_reset(void)
{
    /* F-037: wipe the sketches partition and re-initialise in-RAM
     * state to defaults. Destructive -- caller is responsible for
     * gating the gesture (3-second hold of REC + BPM, see main.c /
     * ui/input.c) and printing a warning before invoking.
     *
     * 1. Format the LittleFS partition (deletes every file under
     *    /sketches/ including all saved patterns, the chain, and
     *    every sketch directory).
     * 2. Re-init the in-RAM pattern/chain state via pattern_init_all().
     *    Samples live in PSRAM only (storage_init docs) and are NOT
     *    wiped -- the user keeps their recordings. This matches the
     *    PO-33 factory-reset semantics ("erase everything in the
     *    song chain; samples are unchanged").
     * 3. The sketch Picker UI may be open; storage_factory_reset()
     *    doesn't touch it. Caller should close the picker if open
     *    before invoking, or accept the stale-active-state UI. */
    if (!s_sketches_mounted) {
        ESP_LOGE(TAG, "factory_reset: not initialised");
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t e = esp_littlefs_format("sketches");
    if (e != ESP_OK) {
        ESP_LOGE(TAG, "factory_reset: format failed %s", esp_err_to_name(e));
        return e;
    }
    pattern_init_all();

    /* Clear the NVS "active sketch" pointer so the next boot starts
     * fresh on the default sketch 0000 (whose folder doesn't yet
     * exist; storage_load_all() handles that by returning ESP_OK
     * with an empty RAM state). */
    nvs_handle_t h;
    if (nvs_open(NVS_NS_SKETCHES, NVS_READWRITE, &h) == ESP_OK) {
        nvs_erase_key(h, NVS_KEY_ACTIVE_ID);
        nvs_commit(h);
        nvs_close(h);
        ESP_LOGW(TAG, "factory_reset: NVS active_id erased");
    }

    ESP_LOGW(TAG, "factory_reset: sketches partition formatted; "
                  "in-RAM patterns/chain reset to defaults");
    return ESP_OK;
}

/* ─── Active sketch identity (NVS) ─────────────────────────────── */

/* True iff `id` is exactly 4 lowercase hex characters. */
static bool is_valid_sketch_id(const char *id)
{
    if (!id) return false;
    for (int i = 0; i < SKETCH_ID_LEN; i++) {
        char c = id[i];
        bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        if (!ok) return false;
    }
    return id[SKETCH_ID_LEN] == '\0';
}

esp_err_t storage_get_active_sketch_id(char *out_id)
{
    if (!out_id) return ESP_ERR_INVALID_ARG;
    out_id[0] = '\0';

    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS_SKETCHES, NVS_READONLY, &h);
    if (err != ESP_OK) {
        /* No NVS entry (fresh flash, or namespace never written):
         * fall back to the default sketch. */
        strcpy(out_id, DEFAULT_SKETCH_ID);
        return ESP_OK;
    }
    size_t len = SKETCH_ID_LEN + 1;
    err = nvs_get_str(h, NVS_KEY_ACTIVE_ID, out_id, &len);
    nvs_close(h);
    if (err != ESP_OK || !is_valid_sketch_id(out_id)) {
        strcpy(out_id, DEFAULT_SKETCH_ID);
        return ESP_OK;
    }
    return ESP_OK;
}

esp_err_t storage_set_active_sketch_id(const char *id)
{
    if (!is_valid_sketch_id(id)) return ESP_ERR_INVALID_ARG;

    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS_SKETCHES, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open(sketches, RW) failed: %s", esp_err_to_name(err));
        return err;
    }
    err = nvs_set_str(h, NVS_KEY_ACTIVE_ID, id);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str(active_id, %s) failed: %s",
                 id, esp_err_to_name(err));
    }
    return err;
}

/* ─── Per-pattern helpers (active sketch folder) ───────────────── */

/* Compute the absolute path of a file inside a sketch's folder.
 * `out` must be at least 80 bytes. */
static void sketch_path(char *out, size_t out_size,
                        const char *id, const char *leaf)
{
    snprintf(out, out_size, "%s/%s/%s", SKETCHES_BASE, id, leaf);
}

/* Best-effort mkdir for the sketch folder. LittleFS's mkdir returns
 * success if the directory already exists, so we just log on
 * failure and let the open() below complain if needed. */
static void ensure_sketch_dir(const char *id)
{
    char dir[64];
    snprintf(dir, sizeof(dir), "%s/%s", SKETCHES_BASE, id);
    if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
        ESP_LOGW(TAG, "mkdir(%s) failed: errno=%d", dir, errno);
    }
}

esp_err_t storage_save_pattern(uint8_t idx)
{
    if (idx >= PATTERN_COUNT) return ESP_ERR_INVALID_ARG;
    char id[SKETCH_ID_LEN + 1];
    esp_err_t e = storage_get_active_sketch_id(id);
    if (e != ESP_OK) return e;

    char path[80];
    sketch_path(path, sizeof(path), id, "patterns.bin");
    ensure_sketch_dir(id);

    /* Read-modify-write the full patterns.bin. The sketch's
     * patterns.bin is tiny (~50 KB) so this is cheap and avoids a
     * "patterns.bin.tmp" dance for a single-pattern edit. For full
     * sketch save use storage_save_all() which writes the whole
     * file atomically. */
    pattern_t buf[PATTERN_COUNT];
    FILE *fr = fopen(path, "rb");
    if (fr) {
        if (fread(buf, sizeof(pattern_t), PATTERN_COUNT, fr) != PATTERN_COUNT) {
            /* short read -- treat as fresh file */
        }
        fclose(fr);
    } else {
        memset(buf, 0, sizeof(buf));
    }
    memcpy(&buf[idx], &g_patterns[idx], sizeof(pattern_t));

    char tmp[96];
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    FILE *fw = fopen(tmp, "wb");
    if (!fw) return ESP_FAIL;
    size_t w = fwrite(buf, sizeof(pattern_t), PATTERN_COUNT, fw);
    fclose(fw);
    if (w != PATTERN_COUNT) { remove(tmp); return ESP_FAIL; }
    if (rename(tmp, path) != 0) {
        remove(path);
        if (rename(tmp, path) != 0) { remove(tmp); return ESP_FAIL; }
    }
    return ESP_OK;
}

esp_err_t storage_load_pattern(uint8_t idx)
{
    if (idx >= PATTERN_COUNT) return ESP_ERR_INVALID_ARG;
    char id[SKETCH_ID_LEN + 1];
    storage_get_active_sketch_id(id);

    char path[80];
    sketch_path(path, sizeof(path), id, "patterns.bin");
    FILE *f = fopen(path, "rb");
    if (!f) return ESP_ERR_NOT_FOUND;
    pattern_t buf[PATTERN_COUNT];
    size_t r = fread(buf, sizeof(pattern_t), PATTERN_COUNT, f);
    fclose(f);
    if (r != PATTERN_COUNT) return ESP_FAIL;
    g_patterns[idx] = buf[idx];
    return ESP_OK;
}
/* ─── samples.bin I/O ───────────────────────────────────────────── */

esp_err_t storage_sketch_save_samples(const char *id_str)
{
    if (!is_valid_sketch_id(id_str)) return ESP_ERR_INVALID_ARG;
    ensure_sketch_dir(id_str);

    /* Build the slot table from the live amy_bridge state. We
     * peek at the slot metadata via the public accessors rather
     * than touching s_slots[] directly (it's file-local to
     * amy_bridge.c). The amy_bridge exposes start/end as trim
     * percentages (0..100). */
    uint8_t  table[SLOT_TABLE_SIZE_BYTES];
    memset(table, 0, sizeof(table));
    uint32_t total_pcm_bytes = 0;

    for (uint8_t s = 0; s < SLOT_COUNT; s++) {
        size_t n_samples = amy_bridge_slot_len_samples(s);
        uint8_t *rec = &table[s * SLOT_REC_SIZE_BYTES];

        if (n_samples == 0) {
            /* length_samples stays 0 -> "empty slot" sentinel. */
            continue;
        }

        /* u32 length_samples (LE) */
        uint32_t len = (uint32_t)n_samples;
        memcpy(rec + 0, &len, 4);

        /* u32 sample_rate_hz (LE) -- amy_bridge stores it per-slot;
         * we round-trip through the live PSRAM pool's metadata. */
        uint32_t sr = amy_bridge_slot_sample_rate_hz(s);
        if (sr == 0) sr = SAMPLE_RATE_HZ;
        memcpy(rec + 4, &sr, 4);

        /* u32 start_sample, end_sample (LE) -- trim bounds in
         * sample positions. amy_bridge_set_trim() takes the same
         * units. */
        uint32_t ss = amy_bridge_slot_start_sample(s);
        uint32_t es = amy_bridge_slot_end_sample(s);
        if (ss > len) ss = len;
        if (es > len) es = len;
        if (ss >= es) { ss = 0; es = len; }
        memcpy(rec + 8,  &ss, 4);
        memcpy(rec + 12, &es, 4);

        total_pcm_bytes += len * sizeof(int16_t);
    }

    /* Write slot table + PCM to a temp file, then rename atomically. */
    char path[80], tmp[96];
    sketch_path(path, sizeof(path), id_str, "samples.bin");
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    FILE *f = fopen(tmp, "wb");
    if (!f) {
        ESP_LOGE(TAG, "save_samples: fopen(%s) failed", tmp);
        return ESP_FAIL;
    }
    if (fwrite(table, 1, sizeof(table), f) != sizeof(table)) {
        fclose(f); remove(tmp);
        ESP_LOGE(TAG, "save_samples: table write short");
        return ESP_FAIL;
    }
    /* Append each non-empty slot's PCM. amy_bridge_slot_ptr() gives
     * a pointer directly into the PSRAM pool -- exactly what we
     * want to flush to flash. */
    for (uint8_t s = 0; s < SLOT_COUNT; s++) {
        const int16_t *p = amy_bridge_slot_ptr(s);
        size_t n_samples = amy_bridge_slot_len_samples(s);
        if (!p || n_samples == 0) continue;
        size_t want = n_samples * sizeof(int16_t);
        if (fwrite(p, 1, want, f) != want) {
            fclose(f); remove(tmp);
            ESP_LOGE(TAG, "save_samples: slot %u PCM write short", s);
            return ESP_FAIL;
        }
    }
    fclose(f);
    if (rename(tmp, path) != 0) {
        if (errno != ENOENT) remove(path);  /* best effort */
        if (rename(tmp, path) != 0) {
            remove(tmp);
            ESP_LOGE(TAG, "save_samples: rename failed: errno=%d", errno);
            return ESP_FAIL;
        }
    }

    ESP_LOGI(TAG, "save_samples[%s]: %u B PCM + 256 B table",
             id_str, (unsigned)total_pcm_bytes);
    return ESP_OK;
}

esp_err_t storage_sketch_load_samples(const char *id_str)
{
    if (!is_valid_sketch_id(id_str)) return ESP_ERR_INVALID_ARG;

    char path[80];
    sketch_path(path, sizeof(path), id_str, "samples.bin");
    FILE *f = fopen(path, "rb");
    if (!f) {
        /* No samples on disk for this sketch -- not an error; the
         * sketch simply has no samples yet. PSRAM pool stays empty. */
        return ESP_OK;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < (long)SLOT_TABLE_SIZE_BYTES) {
        ESP_LOGW(TAG, "load_samples[%s]: file too small (%ld B)", id_str, sz);
        fclose(f);
        return ESP_FAIL;
    }
    /* Slurp the entire file. 256 B table + up to SAMPLE_POOL_SIZE_BYTES
     * (3.37 MB) PCM -- fits comfortably in normal internal heap for
     * a one-shot boot-time load. */
    uint8_t *file_buf = malloc((size_t)sz);
    if (!file_buf) {
        fclose(f);
        ESP_LOGE(TAG, "load_samples[%s]: malloc(%ld) failed", id_str, sz);
        return ESP_ERR_NO_MEM;
    }
    if (fread(file_buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(file_buf);
        fclose(f);
        ESP_LOGE(TAG, "load_samples[%s]: short read", id_str);
        return ESP_FAIL;
    }
    fclose(f);

    /* Walk the slot table; for each in-use slot, copy its PCM slice
     * into a small intermediate and hand it to amy_bridge_register_slot().
     * The helper memcpy()s into s_pool anyway and we'd need to keep
     * file_buf alive until every slot is registered -- safer to copy
     * through a tiny intermediate freed immediately after the call. */
    esp_err_t result = ESP_OK;
    size_t pcm_offset = SLOT_TABLE_SIZE_BYTES;
    for (uint8_t s = 0; s < SLOT_COUNT && result == ESP_OK; s++) {
        const uint8_t *rec = &file_buf[s * SLOT_REC_SIZE_BYTES];
        uint32_t len = 0;
        memcpy(&len, rec + 0, 4);
        if (len == 0) continue;  /* empty slot sentinel */
        uint32_t sr = 0;
        memcpy(&sr, rec + 4, 4);
        uint32_t ss = 0, es = 0;
        memcpy(&ss, rec + 8, 4);
        memcpy(&es, rec + 12, 4);
        /* is_drum derived from slot index: drum slots are 0..7.
         * ADR-0002 keeps the drum/melodic split by index. */
        bool is_drum = (s < SLOT_DRUM_COUNT);

        if (pcm_offset + (size_t)len * sizeof(int16_t) > (size_t)sz) {
            ESP_LOGE(TAG, "load_samples[%s]: truncated at slot %u", id_str, s);
            result = ESP_FAIL;
            break;
        }
        size_t bytes = (size_t)len * sizeof(int16_t);
        int16_t *tmp = malloc(bytes);
        if (!tmp) {
            result = ESP_ERR_NO_MEM;
            break;
        }
        memcpy(tmp, &file_buf[pcm_offset], bytes);
        result = amy_bridge_register_slot(s, tmp, (size_t)len, sr, is_drum);
        free(tmp);
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "load_samples[%s]: register_slot(%u) failed",
                     id_str, s);
            break;
        }
        /* Re-apply trim (register_slot resets to start=0, end=len).
         * Clamp to the new length defensively. */
        if (ss > len) ss = len;
        if (es > len) es = len;
        if (ss >= es) { ss = 0; es = len; }
        amy_bridge_set_trim(s, ss, es);
        pcm_offset += bytes;
    }
    free(file_buf);
    return result;
}

esp_err_t storage_sketch_load_with_samples(const char *id_str)
{
    esp_err_t e = storage_sketch_load(id_str);
    if (e != ESP_OK) return e;
    return storage_sketch_load_samples(id_str);
}

/* ─── Save all / load all (sketch-scoped, ADR-0004) ─────────────── */

esp_err_t storage_save_all(void)
{
    /* If NVS has no active_id (first-ever boot before any sketch
     * has been saved), fall back to the default sketch 0000 so
     * this save has somewhere to land. */
    char id[SKETCH_ID_LEN + 1];
    esp_err_t e = storage_get_active_sketch_id(id);
    if (e != ESP_OK) return e;

    /* If the sketch folder doesn't exist yet (e.g. factory-reset
     * since the last boot), create it and add an entry to
     * sketches.lst so the sketch is real on disk. */
    char dir[64];
    snprintf(dir, sizeof(dir), "%s/%s", SKETCHES_BASE, id);
    bool need_index_entry = (access(dir, F_OK) != 0);
    ensure_sketch_dir(id);

    /* 1. Patterns: one atomic write of the full 16-pattern blob. */
    char ppath[80], ptmp[96];
    sketch_path(ppath, sizeof(ppath), id, "patterns.bin");
    snprintf(ptmp, sizeof(ptmp), "%s.tmp", ppath);
    FILE *fp = fopen(ptmp, "wb");
    if (!fp) return ESP_FAIL;
    size_t pw = fwrite(g_patterns, sizeof(pattern_t), PATTERN_COUNT, fp);
    fclose(fp);
    if (pw != PATTERN_COUNT) { remove(ptmp); return ESP_FAIL; }
    if (rename(ptmp, ppath) != 0) {
        remove(ppath);
        if (rename(ptmp, ppath) != 0) { remove(ptmp); return ESP_FAIL; }
    }

    /* 2. Chain: 1 byte len + entries. */
    char chpath[80], chtmp[96];
    sketch_path(chpath, sizeof(chpath), id, "chain.bin");
    snprintf(chtmp, sizeof(chtmp), "%s.tmp", chpath);
    FILE *fc = fopen(chtmp, "wb");
    if (!fc) return ESP_FAIL;
    uint8_t len = (g_chain_len > PATTERN_CHAIN_MAX) ? PATTERN_CHAIN_MAX : g_chain_len;
    fwrite(&len, 1, 1, fc);
    if (len > 0) fwrite(g_chain, 1, len, fc);
    fclose(fc);
    if (rename(chtmp, chpath) != 0) {
        remove(chpath);
        if (rename(chtmp, chpath) != 0) { remove(chtmp); return ESP_FAIL; }
    }

    /* 3. Samples: pool → flash. */
    e = storage_sketch_save_samples(id);
    if (e != ESP_OK) {
        ESP_LOGE(TAG, "save_all: save_samples failed: %s", esp_err_to_name(e));
        return e;
    }

    /* 4. If we just materialised the default sketch, add an entry
     *    to sketches.lst so the picker can find it. */
    if (need_index_entry) {
        char names[SKETCHES_MAX][SKETCH_META_LEN];
        char ids  [SKETCHES_MAX][SKETCH_ID_LEN + 1];
        memset(names, 0, sizeof(names));
        memset(ids,   0, sizeof(ids));
        memcpy(ids[0], id, SKETCH_ID_LEN + 1);
        /* Auto-name via the Docker-style jazz generator. No
         * collision check yet (this is the first sketch). */
        const char *taken[1] = { names[0] };
        if (!sketch_name_next_free(names[0], SKETCH_META_LEN, taken, 0)) {
            strncpy(names[0], "default", SKETCH_META_LEN - 1);
            names[0][SKETCH_META_LEN - 1] = '\0';
        }
        esp_err_t we = write_index(1, ids, names);
        if (we != ESP_OK) {
            ESP_LOGW(TAG, "save_all: write_index failed: %s", esp_err_to_name(we));
        }
    }

    ESP_LOGI(TAG, "save_all[%s]: patterns + chain + samples saved", id);
    return ESP_OK;
}

esp_err_t storage_load_all(void)
{
    /* Boot-time load: read NVS active_id, load patterns + chain +
     * samples for that sketch. If the folder doesn't exist (first
     * boot, factory reset), returns ESP_OK with RAM empty -- same
     * behaviour as a brand-new device. */
    char id[SKETCH_ID_LEN + 1];
    esp_err_t e = storage_get_active_sketch_id(id);
    if (e != ESP_OK) return e;

    char dir[64];
    snprintf(dir, sizeof(dir), "%s/%s", SKETCHES_BASE, id);
    if (access(dir, F_OK) != 0) {
        ESP_LOGI(TAG, "load_all: sketch %s folder absent; starting empty", id);
        return ESP_OK;
    }

    e = storage_sketch_load(id);
    if (e != ESP_OK) {
        ESP_LOGE(TAG, "load_all: storage_sketch_load(%s) failed: %s",
                 id, esp_err_to_name(e));
        return e;
    }
    e = storage_sketch_load_samples(id);
    if (e != ESP_OK) {
        ESP_LOGE(TAG, "load_all: storage_sketch_load_samples(%s) failed: %s",
                 id, esp_err_to_name(e));
        return e;
    }
    ESP_LOGI(TAG, "load_all[%s]: patterns + chain + samples restored", id);
    return ESP_OK;
}

/* ─── Multi-sketch (v2) full implementation ───────────────────────── */

/* Layout (per docs/DESIGN.md §11.2):
 *
 *   /sketches/                       mount root (LittleFS)
 *     sketches.lst                   master index, "<id>\t<name>\n"
 *     <id>/
 *       meta.bin                     fixed 24-byte name (NUL-padded)
 *       samples.bin                  256-B slot table + raw PCM
 *                                     (see SLOT_REC_SIZE_BYTES)
 *       patterns.bin                 16 * sizeof(pattern_t) raw
 *       chain.bin                    1 byte (len) + len bytes (entries)
 *
 * atomic-write is via tmpfile + rename(), so power-loss mid-write
 * leaves either the old file or the new one (never a half-written
 * file). */

/* The on-flash sketches.lst format stores the Docker-style jazz
 * name (e.g. "bebop_coltrane") in this slot. The display length
 * and the wire-format length are the same. The #define itself
 * lives at the top of the file (see above) so storage_save_all()
 * can use it. */

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
                (*p >= 'A' && *p <= 'F'))) {
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
                (*p >= 'A' && *p <= 'F'))) {
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
    if (fread(g_patterns, sizeof(pattern_t), PATTERN_COUNT, fp) != PATTERN_COUNT) { fclose(fp); return ESP_FAIL; }
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

/* "sketch new" / "sketch save" semantics: allocate the next free
 * 4-hex ID, snapshot the entire active state (patterns + chain +
 * samples + meta) into a fresh sketch folder, append the new entry
 * to sketches.lst, and bump NVS active_id so the next boot lands
 * here. The previous active sketch is left untouched on flash. */
esp_err_t storage_sketch_save_active(void)
{
    int id = next_free_id();
    char id_str[12];
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

    /* Samples (256-B slot table + raw PCM, ADR-0004). */
    esp_err_t se = storage_sketch_save_samples(id_str);
    if (se != ESP_OK) {
        ESP_LOGE(TAG, "sketch_save_active[%s]: save_samples failed: %s",
                 id_str, esp_err_to_name(se));
        return se;
    }

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

    esp_err_t we = write_index(cur_count, ids_all, names_all);
    if (we != ESP_OK) return we;

    /* Switch the active pointer to the freshly-created sketch so the
     * next boot lands here. ADR-0004. */
    esp_err_t ae = storage_set_active_sketch_id(id_str);
    if (ae != ESP_OK) {
        ESP_LOGW(TAG, "sketch_save_active[%s]: set_active_id failed: %s",
                 id_str, esp_err_to_name(ae));
    }
    ESP_LOGI(TAG, "sketch_save_active: new sketch %s saved (active now)",
             id_str);
    return ESP_OK;
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
     * chain.bin + meta.bin + samples.bin are individually unlinked.) */
    char path[80];
    sketch_path(path, sizeof(path), id_str, "patterns.bin"); remove(path);
    sketch_path(path, sizeof(path), id_str, "chain.bin");    remove(path);
    sketch_path(path, sizeof(path), id_str, "meta.bin");      remove(path);
    sketch_path(path, sizeof(path), id_str, "samples.bin");  remove(path);
    char dir_path[64];
    snprintf(dir_path, sizeof(dir_path), "/sketches/%s", id_str);
    rmdir(dir_path);

    /* If we just deleted the active sketch, clear the NVS pointer
     * so the next boot doesn't try to load a vanished folder. The
     * device will fall back to the default sketch 0000. */
    char active_id[SKETCH_ID_LEN + 1];
    if (storage_get_active_sketch_id(active_id) == ESP_OK &&
        strcmp(active_id, id_str) == 0) {
        /* Clear by erasing the namespace key directly. nvs_erase_key
         * is the right verb for "set to absent"; storage_get_active_sketch_id
         * will return "0000" the next time it's read. */
        nvs_handle_t h;
        if (nvs_open(NVS_NS_SKETCHES, NVS_READWRITE, &h) == ESP_OK) {
            nvs_erase_key(h, NVS_KEY_ACTIVE_ID);
            nvs_commit(h);
            nvs_close(h);
        }
    }

    return ESP_OK;
}
