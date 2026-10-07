/*
 * config.h — ESP32-S3 PO-33 K.O! pin map, sample-rate and slot caps
 *
 * Override any of these via menuconfig / -D flags if your board differs.
 */
#ifndef PO33_CONFIG_H
#define PO33_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

/* ─── Audio sample pool ──────────────────────────────────────── */
#define SAMPLE_RATE_HZ            44100
#define SAMPLE_BITS               16
#define SAMPLE_BYTES_PER_SAMPLE   (SAMPLE_BITS / 8)
#define SAMPLE_BYTES_PER_SECOND   (SAMPLE_RATE_HZ * SAMPLE_BYTES_PER_SAMPLE)

/** Total pool: 40 s mono @ 44100 Hz 16-bit = 3 528 000 B ≈ 3.37 MB PSRAM. */
#define SAMPLE_TOTAL_SECONDS      40
#define SAMPLE_POOL_SIZE_BYTES    (SAMPLE_TOTAL_SECONDS * SAMPLE_BYTES_PER_SECOND)

/** Slot count and per-slot caps. */
#define SLOT_COUNT                16
#define SLOT_DRUM_COUNT           8
#define SLOT_MELODIC_COUNT        8

#define SLOT_DRUM_MAX_SECONDS     2.0f
#define SLOT_MELODIC_MAX_SECONDS  3.0f
#define SLOT_DRUM_MAX_BYTES       ((uint32_t)(SLOT_DRUM_MAX_SECONDS * SAMPLE_BYTES_PER_SECOND))
#define SLOT_MELODIC_MAX_BYTES    ((uint32_t)(SLOT_MELODIC_MAX_SECONDS * SAMPLE_BYTES_PER_SECOND))

/* Polyphony (AMY synth voices per note) */
#define VOICE_COUNT               4

/* ─── I2S pins (external DAC + mic) ───────────────────────────── */
#define I2S_OUT_BCLK_GPIO         GPIO_NUM_9
#define I2S_OUT_LRCK_GPIO         GPIO_NUM_10
#define I2S_OUT_DATA_GPIO         GPIO_NUM_8

#define I2S_IN_BCLK_GPIO          GPIO_NUM_15
#define I2S_IN_LRCK_GPIO          GPIO_NUM_16
#define I2S_IN_DATA_GPIO          GPIO_NUM_17

#define I2S_OUT_PORT              I2S_NUM_0
#define I2S_IN_PORT               I2S_NUM_1

#define I2S_DMA_BUF_COUNT         8
#define I2S_DMA_BUF_LEN           256
#define I2S_SAMPLE_RATE           SAMPLE_RATE_HZ

/* ─── Button matrix ──────────────────────────────────────────── */
/*
 * Two distinct hardware groups:
 *
 *   (A) The 4x4 matrix below holds the 16 polymorphic *step* buttons
 *       (numbered 1-16). On the real PO-33 these same physical buttons
 *       also mean "sample slot 1-16" when SOUND is held, "pattern slot
 *       1-16" when PATTERN is held, "effect 1-15 (+16 = swing)" when FX
 *       is held, etc. We get the same polymorphism by using the same
 *       matrix: BTN_STEP1..BTN_STEP16 mean step in normal play, and
 *       step / slot / effect depending on which mode button is held.
 *
 *   (B) Eight dedicated (modifier) buttons are wired to individual
 *       GPIOs as momentary-to-GND switches with internal pull-ups.
 *       No matrix needed; we just poll them.
 *
 * See hardware/HARDWARE.md S11 "The PO-33's button/knob design
 * (verbatim)" and S12 "How our subset maps onto the PO-33" for the
 * canonical mapping.
 */

#define BTN_ROW_COUNT             4
#define BTN_COL_COUNT             4
#define BTN_ROW_PINS              { GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38 }
#define BTN_COL_PINS              { GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_39, GPIO_NUM_40 }

/* Dedicated (modifier) buttons - one GPIO each, momentary to GND.
 *
 * Layout mirrors the PO-33's modifier row + column:
 *
 *   top row  (left to right):  SOUND, PATTERN, BPM
 *   right column (top to bottom, under Knob B):
 *                              REC, FX, PLAY, WRITE
 *
 * Each PO-33 BPM-handle / PATTERN-/+ pair collapses to a single button.
 * The PO-33's "WRITE" key replaces our previous generic "FUNC" key,
 * reflecting the actual PO-33 use (WRITE enters/exits write mode).
 * GPIO 45 (formerly PAT_DN) is freed for v2 expansion
 * (e.g. octave +/- or screen brightness).
 */
#define BTN_SOUND_GPIO            GPIO_NUM_11      /* top row, leftmost   */
#define BTN_PATTERN_GPIO          GPIO_NUM_44      /* top row, middle     */
#define BTN_BPM_GPIO              GPIO_NUM_13      /* top row, rightmost  */
#define BTN_REC_GPIO              GPIO_NUM_41      /* right column, top   */
#define BTN_FX_GPIO               GPIO_NUM_12      /* right column, 2nd   */
#define BTN_PLAY_GPIO             GPIO_NUM_42      /* right column, 3rd   */
#define BTN_WRITE_GPIO            GPIO_NUM_43      /* right column, bottom */

#define BTN_GPIO_COUNT             7   /* number of dedicated modifier buttons */

/**
 * Logical button IDs.
 *
 *   IDs 0..6   : 7 dedicated modifier buttons (REC, PLAY, SOUND, WRITE,
 *                FX, BPM, PATTERN). Order mirrors the PO-33's modifier
 *                row + right column.
 *   IDs 7..22  : 16 step buttons on the 4x4 matrix (STEP1..STEP16)
 *
 * Total: 23 logical buttons + 2 analog knobs.
 *
 *   The 4x4 step matrix uses only 8 GPIOs (4 row + 4 col), so the
 *   7 dedicated modifier buttons + the matrix's 8 GPIOs + 2 ADC
 *   channels = 17 GPIOs total - well within the ESP32-S3's budget.
 *   See ui/knobs.c for the knob driver and hardware/HARDWARE.md S4.11
 *   for the wiring.
 */
enum {
    /* Dedicated modifier buttons (GPIO inputs).
     * Order matches the chosen physical layout (top row + right
     * column under Knob B). */
    BTN_SOUND    = 0,    /* was BTN_FUNC; PO-33's "S" modifier    */
    BTN_PATTERN  = 1,    /* was BTN_PAT_UP; PO-33's pattern mod   */
    BTN_BPM      = 2,    /* was BTN_BPM_UP; PO-33's BPM handle   */
    BTN_REC      = 3,    /* top of right column; PO-33's record  */
    BTN_FX       = 4,    /* PO-33's "FX" modifier                 */
    BTN_PLAY     = 5,    /* PO-33's play                         */
    BTN_WRITE    = 6,    /* bottom of right column; PO-33's write */

    /* Step / polymorphic buttons (4x4 matrix). */
    BTN_STEP1    = 7,
    BTN_STEP2    = 8,
    BTN_STEP3    = 9,
    BTN_STEP4    = 10,
    BTN_STEP5    = 11,
    BTN_STEP6    = 12,
    BTN_STEP7    = 13,
    BTN_STEP8    = 14,
    BTN_STEP9    = 15,
    BTN_STEP10 = 16,
    BTN_STEP11 = 17,
    BTN_STEP12 = 18,
    BTN_STEP13 = 19,
    BTN_STEP14 = 20,
    BTN_STEP15 = 21,
    BTN_STEP16 = 22,

    BTN_COUNT    = 23
};

#define BTN_DEBOUNCE_MS           30
#define BTN_LONG_PRESS_MS         600
#define BTN_IDLE_SLEEP_MS         (5 * 60 * 1000)   /* 5 min idle → deep sleep */

/**
 * Returns true if the given button ID is one of the 16 step buttons
 * (i.e. on the 4x4 matrix, in the polymorphic range).
 */
#define BTN_IS_STEP(id) ((id) >= BTN_STEP1 && (id) <= BTN_STEP16)

/* ─── 2.4" TFT (ILI9341, SPI) ────────────────────────────────── */
#define TFT_SPI_HOST              SPI2_HOST
#define TFT_SPI_CLK_HZ            (40 * 1000 * 1000)
#define TFT_MOSI_GPIO             GPIO_NUM_6
#define TFT_SCK_GPIO              GPIO_NUM_7
#define TFT_CS_GPIO               GPIO_NUM_5
#define TFT_DC_GPIO               GPIO_NUM_4
#define TFT_RST_GPIO              GPIO_NUM_48
#define TFT_BL_GPIO               GPIO_NUM_47

#define TFT_WIDTH                 240
#define TFT_HEIGHT                320

#define TFT_BL_PWM_TIMER          LEDC_TIMER_0
#define TFT_BL_PWM_CHANNEL        LEDC_CHANNEL_0
#define TFT_BL_PWM_HZ             5000
#define TFT_BL_DEFAULT_DUTY       70  /* percent */

/* ─── Jam Sync GPIO ──────────────────────────────────────────── */
#define SYNC_OUT_GPIO             GPIO_NUM_18
/* SYNC_IN (jam-sync input, F-031) was originally GPIO 19. GPIO 19
 * is the ESP32-S3 USB-OTG D− line and is reserved for the future
 * USB-MIDI feature (§14 — `USB-MIDI`). Reassigned to GPIO 25
 * (RTC-domain; capable of waking the chip from deep sleep on a
 * sync pulse, which GPIO 19 cannot). The GPIO is still configured
 * as input with internal pull-up by `sync_init()`; no listener is
 * wired yet (F-031 ❌ missing). */
#define SYNC_IN_GPIO              GPIO_NUM_19

/* ─── Status LEDs (optional) ─────────────────────────────────── */
#define LED_REC_GPIO              GPIO_NUM_21
#define LED_PLAY_GPIO             GPIO_NUM_14

/* ─── Battery monitor ────────────────────────────────────────── */
#define BATTERY_ADC_CHANNEL       ADC_CHANNEL_3   /* GPIO4 in v6.0 ADC1; remap as needed */
#define BATTERY_ADC_ATTEN         ADC_ATTEN_DB_12
#define BATTERY_DIVIDER_RATIO     2.0f            /* R1=R2 */

/* ─── Knobs (PO-33's "Knob A" and "Knob B") ─────────────────────── */
/*
 * Each knob is a 10 kΩ linear potentiometer wired as a voltage
 * divider between 3.3 V and GND, with the wiper on the ADC pin.
 *
 *   PO-33 names them "Knob A" and "Knob B" (no semantic name on the
 *   device; what they do depends on the active tweak mode). We use the
 *   same names.
 *
 *   Knob A → ADC1_CH1  on GPIO 2   (was GPIO 20 / ADC1_CH9; see note)
 *   Knob B → ADC1_CH5  on GPIO 46
 *
 *   Knob A was reassigned from GPIO 20 to GPIO 2 to free the USB-OTG
 *   D+ line (GPIO 20) for the future USB-MIDI feature (§14). GPIO 2 is
 *   the only free ADC1 channel on the N16R8 module not already used by
 *   I2S / TFT / boot strapping. (An SD card interface was considered
 *   but declined — see HARDWARE.md §8.4 for the GPIO-budget analysis.)
 *
 *   ⚠️ GPIO 2 is a strapping pin: it must be floating or held HIGH at
 *   boot, otherwise the chip enters download-boot (esptool) mode. The
 *   Knob A voltage divider in the BOM (10 kΩ to 3V3, wiper to ADC) is
 *   high-impedance and the ADC's internal pull-up keeps GPIO 2 high
 *   unless the knob is turned fully counter-clockwise. In practice the
 *   boot-time pull-up wins. If users report "device stuck in download
 *   mode", instruct them to rotate Knob A away from the minimum stop
 *   before power-cycling.
 *
 *   Knob B (GPIO 46 / ADC1_CH5) is non-strapping and confirmed
 *   ADC-capable on the ESP32-S3-WROOM-1-N16R8 module.
 */
#define KNOB_A_GPIO               GPIO_NUM_2
#define KNOB_A_ADC_CHANNEL        ADC_CHANNEL_1
#define KNOB_B_GPIO               GPIO_NUM_46
#define KNOB_B_ADC_CHANNEL        ADC_CHANNEL_5
#define KNOB_ADC_ATTEN            ADC_ATTEN_DB_12
#define KNOB_SAMPLES              8     /* samples averaged per read */
#define KNOB_DEADZONE             4     /* ignore deltas below this in 0..255 units */

/* ─── Rotary encoder (left side of board) ────────────────────── */
/* Alps EC11E, 24 detents, vertical mount. Phase A and phase B are
 * consumed by ESP32-S3 PCNT unit 0; the click-switch is a polled
 * GPIO debounced inside encoder_tick(). The encoder is REQUIRED at
 * build (no runtime fallback). */
#define ENCODER_A_GPIO            GPIO_NUM_20      /* phase A; was GPIO_NUM_22 (invalid for S3 in v6.1) */
#define ENCODER_B_GPIO            GPIO_NUM_26      /* phase B; was GPIO_NUM_23 (invalid for S3 in v6.1) */
#define ENCODER_CLICK_GPIO        GPIO_NUM_45      /* click-switch; was GPIO_NUM_24 in old pin map (invalid for S3 in v6.1) */

/* ─── Sequencer defaults ─────────────────────────────────────── */
#define DEFAULT_BPM               120
#define MIN_BPM                   60
#define MAX_BPM                   240
#define STEPS_PER_PATTERN         16
#define PATTERN_COUNT             16
#define PATTERN_CHAIN_MAX         128

/* PO-33 BPM preset levels (F-021). Tapping BTN_BPM repeatedly cycles
 * through them in this order; long-pressing BTN_BPM enters BPM-adjust
 * mode where Knob A adjusts swing and Knob B fine-tunes BPM. */
#define BPM_PRESET_HIP_HOP        80
#define BPM_PRESET_DISCO          120
#define BPM_PRESET_TECHNO         140
#define BPM_PRESETS               { BPM_PRESET_HIP_HOP, \
                                    BPM_PRESET_DISCO,    \
                                    BPM_PRESET_TECHNO }
#define BPM_PRESET_COUNT          3

/* Swing (PO-33 "change swing" combo: hold BPM + Knob A). 8 discrete
 * levels (0=no swing, 7=max). SWING_MAX_PERCENT is the maximum
 * percentage of a 16th-note duration that an off-beat step can be
 * delayed by at swing level 7. */
#define SWING_LEVELS              8
#define SWING_MAX_PERCENT         50

/* ─── Sketch (v2 multi-storage) naming convention ──────────────── */
/*
 * When the v2 multi-sketch storage system lands (see docs/DESIGN.md
 * S11), all new identifiers in storage.h / sequencer.h / ui/menu.h
 * MUST use the `sketch` prefix, NOT `project`. Example:
 *
 *   storage_sketch_save_active()   // not storage_project_save_active()
 *   sketch_meta_t                  // not project_meta_t
 *   SKETCHES_MAX                   // not PROJECTS_MAX
 *   sketches.lst                   // not project.lst
 *
 * The user-facing word is "sketch" (PO-33 calls it a "song", Korg
 * Electribe calls it a "Pattern Set", Ableton calls it a "Live Set").
 * See docs/DESIGN.md S11 vocabulary callout for the full rationale.
 *
 * v1 firmware code does NOT yet contain sketch_/project_ identifiers,
 * so there is nothing to rename today. The convention is documented
 * here so the v2 implementer doesn't accidentally introduce the old
 * `project` prefix.
 */
#define SKETCHES_MAX              16   /* hard cap for UI picker; see S11.8 */
#define SKETCH_NAME_MAX           24    /* bytes, NUL-terminated */

/* ─── UART shell ─────────────────────────────────────────────── */
#define SHELL_UART_BAUD           115200

#endif /* PO33_CONFIG_H */