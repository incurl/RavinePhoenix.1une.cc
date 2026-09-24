/*
 * sample_manager.h — PSRAM-backed sample slot manager.
 *
 *   Single contiguous 60 s mono pool in PSRAM.
 *   16 slots: 0..7 = drums (≤3 s), 8..15 = melodic (≤4.5 s).
 *   Trimming and slicing supported.
 */
#ifndef PO33_SAMPLE_MANAGER_H
#define PO33_SAMPLE_MANAGER_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool     in_use;
    bool     is_drum;
    uint32_t length_samples;   /* mono frames in slot */
    uint32_t start;            /* trim start (samples) */
    uint32_t end;              /* trim end   (samples) */
} sample_slot_info_t;

esp_err_t sample_manager_init(void);
void     sample_manager_deinit(void);

uint32_t sample_manager_pool_free_bytes(void);
const    sample_slot_info_t *sample_manager_slot_info(uint8_t slot);
uint32_t sample_manager_slot_max_bytes(uint8_t slot);

/* Recording (non-blocking — call from audio task or a timer). */
esp_err_t sample_manager_start_record(uint8_t slot);
void      sample_manager_stop_record(void);
bool      sample_manager_is_recording(void);

/* Trimming / slicing */
esp_err_t sample_manager_set_trim(uint8_t slot, uint32_t start, uint32_t end);
esp_err_t sample_manager_slice_auto(uint8_t slot, uint8_t parts, uint32_t *out_starts);

/* Direct read pointer (for the audio mixer — zero copy). */
const int16_t *sample_manager_slot_ptr(uint8_t slot);
uint32_t       sample_manager_slot_len_samples(uint8_t slot);

/* Writes from external source (mic, file, etc.). Returns bytes written. */
size_t sample_manager_write(uint8_t slot, const int16_t *src, size_t num_samples);

#ifdef __cplusplus
}
#endif

#endif /* PO33_SAMPLE_MANAGER_H */