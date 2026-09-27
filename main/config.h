/*
 * config.h — ESP32-S3 PO-33 K.O! pin map, sample-rate and slot caps
 *
 * Override any of these via menuconfig / -D flags if your board differs.
 */
#ifndef PO33_CONFIG_H
#define PO33_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

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

/* Dedicated (modifier) buttons - one GPIO each, momentary to GND. */
#define BTN_REC_GPIO              GPIO_NUM_41
#define BTN_PLAY_GPIO             GPIO_NUM_42
#define BTN_FUNC_GPIO             GPIO_NUM_11
#define BTN_FX_GPIO               GPIO_NUM_12
#define BTN_BPM_UP_GPIO           GPIO_NUM_13
#define BTN_BPM_DN_GPIO           GPIO_NUM_43
#define BTN_PAT_UP_GPIO           GPIO_NUM_44
#define BTN_PAT_DN_GPIO           GPIO_NUM_45

#define BTN_GPIO_COUNT             8   /* number of dedicated modifier buttons */

/**
 * Logical button IDs.
 *
 *   IDs 0..7   : 8 dedicated modifier buttons (REC, PLAY, FUNC, FX, BPM±, PAT±)
 *   IDs 8..23  : 16 step buttons on the 4x4 matrix (STEP1..STEP16)
 *
 * Total: 24 logical buttons. (The real PO-33 has 16 + 2 knobs; we
 * replace the knobs with extra modifier buttons.)
 */
enum {
    /* Dedicated modifier buttons (GPIO inputs). */
    BTN_REC    = 0,
    BTN_PLAY   = 1,
    BTN_FUNC   = 2,
    BTN_FX     = 3,
    BTN_BPM_UP = 4,
    BTN_BPM_DN = 5,
    BTN_PAT_UP = 6,
    BTN_PAT_DN = 7,

    /* Step / polymorphic buttons (4x4 matrix). */
    BTN_STEP1  = 8,
    BTN_STEP2  = 9,
    BTN_STEP3  = 10,
    BTN_STEP4  = 11,
    BTN_STEP5  = 12,
    BTN_STEP6  = 13,
    BTN_STEP7  = 14,
    BTN_STEP8  = 15,
    BTN_STEP9  = 16,
    BTN_STEP10 = 17,
    BTN_STEP11 = 18,
    BTN_STEP12 = 19,
    BTN_STEP13 = 20,
    BTN_STEP14 = 21,
    BTN_STEP15 = 22,
    BTN_STEP16 = 23,

    BTN_COUNT  = 24
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
#define SYNC_IN_GPIO              GPIO_NUM_19

/* ─── Status LEDs (optional) ─────────────────────────────────── */
#define LED_REC_GPIO              GPIO_NUM_21
#define LED_PLAY_GPIO             GPIO_NUM_14

/* ─── Battery monitor ────────────────────────────────────────── */
#define BATTERY_ADC_CHANNEL       ADC_CHANNEL_3   /* GPIO4 in v6.0 ADC1; remap as needed */
#define BATTERY_ADC_ATTEN         ADC_ATTEN_DB_12
#define BATTERY_DIVIDER_RATIO     2.0f            /* R1=R2 */

/* ─── Sequencer defaults ─────────────────────────────────────── */
#define DEFAULT_BPM               120
#define MIN_BPM                   60
#define MAX_BPM                   240
#define STEPS_PER_PATTERN         16
#define PATTERN_COUNT             16
#define PATTERN_CHAIN_MAX         128

/* ─── UART shell ─────────────────────────────────────────────── */
#define SHELL_UART_BAUD           115200

#endif /* PO33_CONFIG_H */