/*
 * storage.c — LittleFS partitions for samples + patterns.
 *
 * v6.0 note: LittleFS is a component (joltwallet/littlefs) in
 * idf_component.yml, not an in-tree driver.
 */
#include "storage.h"
#include "config.h"
#include "sequencer/pattern.h"
#include "audio/sample_manager.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static const char *TAG = "storage";

#define SAMPLES_BASE  "/samples"
#define PATTERNS_BASE "/patterns"

static esp_littlefs_handle_t s_samples_fs  = NULL;
static esp_littlefs_handle_t s_patterns_fs = NULL;

esp_err_t storage_init(void)
{
    /* Init partitions — partitions.csv declares the labels. */
    esp_vfs_littlefs_conf_t samples_conf = {
        .base_path          = SAMPLES_BASE,
        .partition_label    = "samples",
        .format_if_mount_failed = true,
        .dont_mount         = false,
    };
    ESP_ERROR_CHECK(esp_littlefs_create(&samples_conf, &s_samples_fs));

    esp_vfs_littlefs_conf_t patterns_conf = {
        .base_path          = PATTERNS_BASE,
        .partition_label    = "patterns",
        .format_if_mount_failed = true,
        .dont_mount         = false,
    };
    ESP_ERROR_CHECK(esp_littlefs_create(&patterns_conf, &s_patterns_fs));

    ESP_LOGI(TAG, "LittleFS mounted at %s and %s", SAMPLES_BASE, PATTERNS_BASE);
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
    snprintf(path, sizeof(path), "%s/p%u.bin", PATTERNS_BASE, idx);
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
    snprintf(path, sizeof(path), "%s/p%u.bin", PATTERNS_BASE, idx);
    FILE *f = fopen(path, "rb");
    if (!f) return ESP_ERR_NOT_FOUND;
    size_t r = fread(&g_patterns[idx], 1, sizeof(pattern_t), f);
    fclose(f);
    return (r == sizeof(pattern_t)) ? ESP_OK : ESP_FAIL;
}

esp_err_t storage_save_all(void)
{
    ESP_LOGI(TAG, "Saving %d patterns...", PATTERN_COUNT);
    for (int i = 0; i < PATTERN_COUNT; i++) {
        esp_err_t e = storage_save_pattern((uint8_t)i);
        if (e != ESP_OK) {
            ESP_LOGE(TAG, "save pattern %d failed", i);
            return e;
        }
    }
    /* Sample pool save (one big file). */
    const int16_t *p = sample_manager_slot_ptr(0);  /* v1: dump contiguous pool */
    /* Total bytes used by all slots. */
    uint32_t total = 0;
    for (int i = 0; i < SLOT_COUNT; i++) {
        total += sample_manager_slot_len_samples((uint8_t)i) *
                 SAMPLE_BYTES_PER_SAMPLE;
    }
    if (total == 0) {
        ESP_LOGI(TAG, "Sample pool empty, skipping");
        return ESP_OK;
    }
    FILE *f = fopen(SAMPLES_BASE "/pool.bin", "wb");
    if (!f) return ESP_FAIL;
    fwrite(p, 1, total, f);
    fclose(f);
    ESP_LOGI(TAG, "Saved pool (%u bytes)", (unsigned)total);
    return ESP_OK;
}

esp_err_t storage_load_all(void)
{
    ESP_LOGI(TAG, "Loading patterns + samples...");
    for (int i = 0; i < PATTERN_COUNT; i++) {
        storage_load_pattern((uint8_t)i);
    }
    FILE *f = fopen(SAMPLES_BASE "/pool.bin", "rb");
    if (!f) {
        ESP_LOGW(TAG, "No sample pool file yet");
        return ESP_OK;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return ESP_OK; }

    /* Naive: dump into slot 0 only for v1. */
    int16_t *tmp = malloc(sz);
    if (!tmp) { fclose(f); return ESP_ERR_NO_MEM; }
    fread(tmp, 1, sz, f);
    fclose(f);
    sample_manager_write(0, tmp, sz / SAMPLE_BYTES_PER_SAMPLE);
    free(tmp);
    return ESP_OK;
}