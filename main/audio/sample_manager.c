/*
 * sample_manager.c — single 2.6 MB PSRAM pool, 16 slots.
 */
#include "sample_manager.h"
#include "config.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "audio/i2s_driver.h"
#include <string.h>

static const char *TAG = "sample_mgr";

static int16_t       *s_pool       = NULL;
static uint32_t       s_pool_bytes = 0;
static uint32_t       s_pool_used  = 0;

static sample_slot_info_t s_slots[SLOT_COUNT] = {0};

static int8_t   s_rec_slot = -1;
static uint32_t s_rec_pos  = 0;

esp_err_t sample_manager_init(void)
{
    ESP_LOGI(TAG, "Allocating %.2f MB PSRAM pool for samples",
             (double)SAMPLE_POOL_SIZE_BYTES / (1024.0 * 1024.0));

    s_pool = heap_caps_malloc(SAMPLE_POOL_SIZE_BYTES, MALLOC_CAP_SPIRAM);
    if (!s_pool) {
        ESP_LOGE(TAG, "PSRAM alloc FAILED — need ~%u bytes free in SPIRAM",
                 (unsigned)SAMPLE_POOL_SIZE_BYTES);
        return ESP_ERR_NO_MEM;
    }
    s_pool_bytes = SAMPLE_POOL_SIZE_BYTES;
    s_pool_used  = 0;
    memset(s_pool, 0, s_pool_bytes);

    for (int i = 0; i < SLOT_COUNT; i++) {
        s_slots[i].is_drum = (i < SLOT_DRUM_COUNT);
        s_slots[i].in_use  = false;
        s_slots[i].length_samples = 0;
        s_slots[i].start = 0;
        s_slots[i].end   = 0;
    }

    ESP_LOGI(TAG, "Pool ready (%u bytes).", (unsigned)s_pool_bytes);
    return ESP_OK;
}

void sample_manager_deinit(void)
{
    if (s_pool) {
        heap_caps_free(s_pool);
        s_pool = NULL;
    }
    s_pool_bytes = 0;
    s_pool_used  = 0;
}

uint32_t sample_manager_pool_free_bytes(void)
{
    return s_pool_bytes - s_pool_used;
}

uint32_t sample_manager_slot_max_bytes(uint8_t slot)
{
    if (slot >= SLOT_COUNT) return 0;
    return s_slots[slot].is_drum ? SLOT_DRUM_MAX_BYTES
                                 : SLOT_MELODIC_MAX_BYTES;
}

const sample_slot_info_t *sample_manager_slot_info(uint8_t slot)
{
    if (slot >= SLOT_COUNT) return NULL;
    return &s_slots[slot];
}

const int16_t *sample_manager_slot_ptr(uint8_t slot)
{
    if (slot >= SLOT_COUNT || !s_slots[slot].in_use) return NULL;
    /* Slots are packed sequentially in the pool; for v1 we give the
     * mixer a direct pointer into the pool offset. */
    uint32_t offset = 0;
    for (int i = 0; i < slot; i++) {
        offset += s_slots[i].length_samples;
    }
    return s_pool + offset;
}

uint32_t sample_manager_slot_len_samples(uint8_t slot)
{
    if (slot >= SLOT_COUNT) return 0;
    return s_slots[slot].length_samples;
}

size_t sample_manager_write(uint8_t slot, const int16_t *src, size_t num_samples)
{
    if (slot >= SLOT_COUNT || !src || num_samples == 0) return 0;
    if (s_slots[slot].in_use) {
        /* overwrite mode: release existing region */
        s_pool_used -= s_slots[slot].length_samples;
        s_slots[slot].in_use = false;
    }
    uint32_t max_samples = sample_manager_slot_max_bytes(slot) /
                           SAMPLE_BYTES_PER_SAMPLE;
    if (num_samples > max_samples) num_samples = max_samples;
    if (sample_manager_pool_free_bytes() < num_samples * SAMPLE_BYTES_PER_SAMPLE) {
        ESP_LOGW(TAG, "Pool exhausted");
        return 0;
    }

    uint32_t offset = 0;
    for (int i = 0; i < slot; i++) {
        offset += s_slots[i].length_samples;
    }
    memcpy(s_pool + offset, src, num_samples * SAMPLE_BYTES_PER_SAMPLE);

    s_slots[slot].in_use         = true;
    s_slots[slot].length_samples = (uint32_t)num_samples;
    s_slots[slot].start          = 0;
    s_slots[slot].end            = (uint32_t)num_samples;
    s_pool_used += (uint32_t)num_samples * SAMPLE_BYTES_PER_SAMPLE;

    return num_samples;
}

esp_err_t sample_manager_start_record(uint8_t slot)
{
    if (slot >= SLOT_COUNT) return ESP_ERR_INVALID_ARG;
    s_rec_slot = (int8_t)slot;
    s_rec_pos  = 0;
    ESP_LOGI(TAG, "Recording into slot %u", slot);
    return ESP_OK;
}

void sample_manager_stop_record(void)
{
    if (s_rec_slot < 0) return;

    /* Finalize the recorded region. */
    uint8_t slot = (uint8_t)s_rec_slot;
    s_slots[slot].in_use         = true;
    s_slots[slot].length_samples = s_rec_pos;
    s_slots[slot].start          = 0;
    s_slots[slot].end            = s_rec_pos;

    ESP_LOGI(TAG, "Recorded %u samples into slot %u", s_rec_pos, slot);
    s_rec_slot = -1;
    s_rec_pos  = 0;
}

bool sample_manager_is_recording(void)
{
    return s_rec_slot >= 0;
}

/* Called from the audio mixer to capture one block into the active slot. */
void sample_manager_record_block(const int16_t *mic_samples, size_t num)
{
    if (s_rec_slot < 0 || !s_pool || !mic_samples || num == 0) return;

    uint8_t slot = (uint8_t)s_rec_slot;
    uint32_t max_samples = sample_manager_slot_max_bytes(slot) /
                           SAMPLE_BYTES_PER_SAMPLE;
    uint32_t room = (s_rec_pos + num > max_samples)
                        ? (max_samples - s_rec_pos)
                        : (uint32_t)num;
    if (room == 0) {
        sample_manager_stop_record();
        return;
    }

    uint32_t offset = 0;
    for (int i = 0; i < slot; i++) {
        offset += s_slots[i].length_samples;
    }
    memcpy(s_pool + offset + s_rec_pos, mic_samples,
           room * SAMPLE_BYTES_PER_SAMPLE);
    s_rec_pos += room;
}

esp_err_t sample_manager_set_trim(uint8_t slot, uint32_t start, uint32_t end)
{
    if (slot >= SLOT_COUNT || !s_slots[slot].in_use) return ESP_ERR_INVALID_ARG;
    if (end > s_slots[slot].length_samples || start >= end) {
        return ESP_ERR_INVALID_ARG;
    }
    s_slots[slot].start = start;
    s_slots[slot].end   = end;
    return ESP_OK;
}

esp_err_t sample_manager_slice_auto(uint8_t slot, uint8_t parts,
                                    uint32_t *out_starts)
{
    if (slot >= SLOT_COUNT || !s_slots[slot].in_use || parts == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    uint32_t len = s_slots[slot].length_samples;
    for (uint8_t i = 0; i < parts; i++) {
        out_starts[i] = (len * i) / parts;
    }
    return ESP_OK;
}