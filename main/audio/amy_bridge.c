/*
 * amy_bridge.c — glue between PO-33 firmware concepts and AMY.
 *
 * Sample pool:
 *   One contiguous PSRAM block partitioned into 16 slots.
 *   Each slot is registered with AMY (see apply_fx for PCM vs synth path).
 *
 * Recording:
 *   AMY exposes the mic DMA as an internal buffer; we copy that into the
 *   active slot each block and finalize on stop.
 */
#include "amy_bridge.h"
#include "config.h"

#include "esp_log.h"
#include "esp_heap_caps.h"

#include <string.h>
#include <stdlib.h>

#include "amy.h"

static const char *TAG = "amy_bridge";

/* PO-33 preset numbers start above AMY's built-in patch range. */
#define PO33_PRESET_BASE   2000

static int16_t *s_pool       = NULL;
static size_t   s_pool_bytes = 0;

static struct {
    bool     in_use;
    bool     is_drum;
    uint32_t length_samples;
    uint32_t start;
    uint32_t end;
    uint32_t sample_rate_hz;
} s_slots[SLOT_COUNT];

static int8_t   s_rec_slot = -1;
static uint32_t s_rec_pos  = 0;
static int16_t *s_rec_block = NULL;

/* Master volume level (PO-33 F-022). Default 5 = max so the device
 * is loud on first power-on; the user lowers via "hold BPM + 1..5".
 * Stored as level (0..5), not as a multiplier, so callers can log
 * the level and we don't accumulate float-rounding errors. */
static uint8_t s_volume_level = 5;

/* Map level 0..5 -> velocity multiplier 0.0..1.0. The curve is
 * roughly 0/0.25/0.5/0.75/0.9/1.0 -- slightly compressed at the
 * top end to avoid the perceptual jump between level 4 and 5.
 * AMY has no public master-gain API, so the multiplier is applied
 * per-note inside play_note(). */
static float volume_multiplier(uint8_t level)
{
    switch (level) {
    case 0:  return 0.0f;
    case 1:  return 0.25f;
    case 2:  return 0.5f;
    case 3:  return 0.75f;
    case 4:  return 0.9f;
    default: return 1.0f;   /* 5 and any out-of-range = max */
    }
}

void amy_bridge_set_volume_level(uint8_t level)
{
    if (level > 5) level = 5;
    s_volume_level = level;
    ESP_LOGI(TAG, "volume level -> %u (mul=%.2f)",
             level, volume_multiplier(level));
}

/* ─── slot registry ─────────────────────────────────────────────── */
static size_t slot_offset_bytes(uint8_t slot)
{
    size_t off = 0;
    for (int i = 0; i < slot; i++) {
        off += s_slots[i].length_samples * sizeof(int16_t);
    }
    return off;
}

const int16_t *amy_bridge_slot_ptr(uint8_t slot)
{
    if (slot >= SLOT_COUNT || !s_slots[slot].in_use) return NULL;
    return s_pool + slot_offset_bytes(slot) / sizeof(int16_t);
}

size_t amy_bridge_slot_len_samples(uint8_t slot)
{
    if (slot >= SLOT_COUNT) return 0;
    return s_slots[slot].length_samples;
}

size_t amy_bridge_slot_max_bytes(uint8_t slot)
{
    if (slot >= SLOT_COUNT) return 0;
    return (slot < SLOT_DRUM_COUNT) ? SLOT_DRUM_MAX_BYTES
                                    : SLOT_MELODIC_MAX_BYTES;
}

esp_err_t amy_bridge_set_trim(uint8_t slot, uint32_t start, uint32_t end)
{
    if (slot >= SLOT_COUNT || !s_slots[slot].in_use) return ESP_ERR_INVALID_ARG;
    if (end > s_slots[slot].length_samples || start >= end) {
        return ESP_ERR_INVALID_ARG;
    }
    s_slots[slot].start = start;
    s_slots[slot].end   = end;
    return ESP_OK;
}

esp_err_t amy_bridge_register_slot(uint8_t slot, const int16_t *data,
                                   size_t num_samples, uint32_t sample_rate_hz,
                                   bool is_drum)
{
    if (slot >= SLOT_COUNT) return ESP_ERR_INVALID_ARG;
    if (num_samples == 0)   return ESP_ERR_INVALID_ARG;

    size_t room_bytes = s_pool_bytes;
    for (int i = 0; i < SLOT_COUNT; i++) {
        if (i == slot && s_slots[i].in_use) continue;
        room_bytes -= s_slots[i].length_samples * sizeof(int16_t);
    }
    size_t max_for_slot = amy_bridge_slot_max_bytes(slot);
    size_t want_bytes = num_samples * sizeof(int16_t);
    if (want_bytes > max_for_slot) want_bytes = max_for_slot;
    if (want_bytes > room_bytes)   want_bytes = room_bytes;
    size_t copy_samples = want_bytes / sizeof(int16_t);

    int16_t *dst = s_pool + slot_offset_bytes(slot) / sizeof(int16_t);
    if (data) memcpy(dst, data, copy_samples * sizeof(int16_t));
    else      memset(dst, 0, copy_samples * sizeof(int16_t));

    s_slots[slot].in_use         = true;
    s_slots[slot].is_drum        = is_drum;
    s_slots[slot].length_samples = (uint32_t)copy_samples;
    s_slots[slot].start          = 0;
    s_slots[slot].end            = (uint32_t)copy_samples;
    s_slots[slot].sample_rate_hz = (sample_rate_hz == 0) ? SAMPLE_RATE_HZ
                                                         : sample_rate_hz;

    ESP_LOGI(TAG, "Slot %u: %u samples (%u Hz, %s)",
             slot, (unsigned)copy_samples,
             (unsigned)s_slots[slot].sample_rate_hz,
             is_drum ? "drum" : "melodic");
    return ESP_OK;
}

/* ─── init / deinit ─────────────────────────────────────────────── */
esp_err_t amy_bridge_init(void)
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
    memset(s_pool, 0, s_pool_bytes);
    memset(s_slots, 0, sizeof(s_slots));

    s_rec_block = heap_caps_malloc(AMY_BLOCK_SIZE * sizeof(int16_t),
                                   MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!s_rec_block) {
        ESP_LOGE(TAG, "Recording scratch alloc failed");
        return ESP_ERR_NO_MEM;
    }

    amy_config_t cfg = amy_default_config();
    cfg.i2s_lrc    = I2S_OUT_LRCK_GPIO;
    cfg.i2s_dout   = I2S_OUT_DATA_GPIO;
    cfg.i2s_din    = I2S_IN_DATA_GPIO;
    cfg.i2s_bclk   = I2S_OUT_BCLK_GPIO;
    cfg.playback_device_id  = 0;
    cfg.capture_device_id   = 0;
    cfg.features.startup_bleep = 0;
    cfg.features.midi = AMY_MIDI_NONE;

    cfg.ram_caps_events = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
    cfg.ram_caps_oscs   = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
    cfg.ram_caps_delay  = MALLOC_CAP_SPIRAM   | MALLOC_CAP_8BIT;
    cfg.ram_caps_fbl    = MALLOC_CAP_SPIRAM   | MALLOC_CAP_8BIT;
    cfg.ram_caps_block  = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
    cfg.ram_caps_sysex  = MALLOC_CAP_SPIRAM   | MALLOC_CAP_8BIT;
    cfg.ram_caps_synth  = MALLOC_CAP_SPIRAM   | MALLOC_CAP_8BIT;

    cfg.platform.multicore   = true;
    cfg.platform.multithread = true;
    cfg.overload_threshold = 0.0f;
    cfg.overload_ms        = 0;

    amy_start(cfg);

    ESP_LOGI(TAG, "AMY running @ %d Hz, block %d frames",
             AMY_SAMPLE_RATE, AMY_BLOCK_SIZE);
    return ESP_OK;
}

void amy_bridge_deinit(void)
{
    amy_stop();
    if (s_pool) { heap_caps_free(s_pool); s_pool = NULL; }
    if (s_rec_block) { heap_caps_free(s_rec_block); s_rec_block = NULL; }
    s_pool_bytes = 0;
}

/* ─── recording ────────────────────────────────────────────────── */
esp_err_t amy_bridge_start_record(uint8_t slot)
{
    if (slot >= SLOT_COUNT) return ESP_ERR_INVALID_ARG;
    s_rec_slot = (int8_t)slot;
    s_rec_pos  = 0;
    ESP_LOGI(TAG, "Recording into slot %u", slot);
    return ESP_OK;
}

void amy_bridge_stop_record(void)
{
    if (s_rec_slot < 0) return;
    uint8_t slot = (uint8_t)s_rec_slot;
    int16_t *src = s_pool + slot_offset_bytes(slot) / sizeof(int16_t);
    amy_bridge_register_slot(slot, src, s_rec_pos,
                             SAMPLE_RATE_HZ,
                             slot < SLOT_DRUM_COUNT);
    ESP_LOGI(TAG, "Recorded %u samples into slot %u", s_rec_pos, slot);
    s_rec_slot = -1;
    s_rec_pos  = 0;
}

bool amy_bridge_is_recording(void)
{
    return s_rec_slot >= 0;
}

void amy_bridge_pump_capture(void)
{
    if (s_rec_slot < 0 || !s_rec_block) return;

    size_t frames = AMY_BLOCK_SIZE;
    if (amy_get_input_buffer(s_rec_block) != (int)frames) return;

    uint8_t slot = (uint8_t)s_rec_slot;
    size_t max_samples = amy_bridge_slot_max_bytes(slot) / sizeof(int16_t);
    size_t room = (s_rec_pos + frames > max_samples)
                      ? (max_samples - s_rec_pos)
                      : frames;
    if (room == 0) {
        amy_bridge_stop_record();
        return;
    }
    int16_t *dst = s_pool + slot_offset_bytes(slot) / sizeof(int16_t) + s_rec_pos;
    memcpy(dst, s_rec_block, room * sizeof(int16_t));
    s_rec_pos += (uint32_t)room;
}

/* ─── note trigger ─────────────────────────────────────────────── */
static void apply_fx(amy_event *e, po33_fx_t fx, uint8_t p1, uint8_t p2)
{
    switch (fx) {
    case PO33_FX_NONE:
    case PO33_FX_LOOP_16:
    case PO33_FX_LOOP_12:
    case PO33_FX_LOOP_SHORT:
    case PO33_FX_LOOP_SHORTER:
    case PO33_FX_RETRIGGER_PATTERN:
    case PO33_FX_68_QUANTIZE:
        break;
    case PO33_FX_OCTAVE_UP:
        e->midi_note = (uint8_t)(e->midi_note + 12); break;
    case PO33_FX_OCTAVE_DOWN:
        e->midi_note = (uint8_t)(e->midi_note > 12 ? e->midi_note - 12 : 0); break;
    case PO33_FX_UNISON:
        break;
    case PO33_FX_UNISON_LOW:
        e->midi_note = (uint8_t)(e->midi_note > 12 ? e->midi_note - 12 : 0); break;
    case PO33_FX_FILTER_SWEEP:
        e->filter_type     = FILTER_LPF;
        e->filter_freq     = 200 + p1 * 2;
        e->filter_resonance = 1.0f + p2 / 64.0f;
        break;
    case PO33_FX_BITCRUSH:
        e->dist_bits = (uint8_t)(4 + (15 - p1 / 18));
        e->dist_rate = (uint8_t)((p2 >> 1) + 1);
        e->dist_clip = 1;
        break;
    case PO33_FX_STUTTER_4:
    case PO33_FX_STUTTER_3:
    case PO33_FX_SCRATCH_FAST:
    case PO33_FX_REVERSE:
    default:
        break;
    }
}

esp_err_t amy_bridge_play_note(uint8_t slot, uint8_t midi_note,
                              uint8_t velocity,
                              po33_fx_t fx, uint8_t fx_p1, uint8_t fx_p2)
{
    if (slot >= SLOT_COUNT) return ESP_ERR_INVALID_ARG;
    bool use_sampler = s_slots[slot].in_use;

    amy_event e = amy_default_event();
    e.midi_note = midi_note;
    /* Apply master volume level (PO-33 F-022). AMY has no public
     * master gain, so we scale velocity per note. */
    e.velocity  = ((float)velocity / 127.0f) * volume_multiplier(s_volume_level);

    if (use_sampler) {
        e.wave = PCM;
        e.patch_number = PO33_PRESET_BASE + slot;
    } else {
        e.synth = 1;
        e.num_voices = (slot < SLOT_DRUM_COUNT) ? 1 : VOICE_COUNT;
        e.patch_number = 0;
    }

    apply_fx(&e, fx, fx_p1, fx_p2);
    amy_add_event(&e);

    if (e.synth) {
        amy_event off = amy_default_event();
        off.midi_note = 0;
        off.velocity  = 0;
        off.time      = amy_sysclock() + 150;
        off.osc       = e.osc;
        off.synth     = e.synth;
        amy_add_event(&off);
    }
    return ESP_OK;
}