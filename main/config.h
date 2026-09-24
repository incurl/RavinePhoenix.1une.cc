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

/* ─── 4×4 button matrix (16 keys) ────────────────────────────── */
#define BTN_ROW_COUNT             4
#define BTN_COL_COUNT             4
#define BTN_ROW_PINS              { GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_37, GPIO_NUM_38 }
#define BTN_COL_PINS              { GPIO_NUM_33, GPIO_NUM_34, GPIO_NUM_39, GPIO_NUM_40 }

/** Logical button IDs (0..15). */
enum {
    BTN_REC   = 0,
    BTN_PLAY  = 1,
    BTN_STEP1 = 2,
    BTN_STEP2 = 3,
    BTN_STEP3 = 4,
    BTN_STEP4 = 5,
    BTN_STEP5 = 6,
    BTN_STEP6 = 7,
    BTN_STEP7 = 8,
    BTN_STEP8 = 9,
    BTN_FUNC  = 10,
    BTN_FX    = 11,
    BTN_BPM_UP = 12,
    BTN_BPM_DN = 13,
    BTN_PAT_UP = 14,
    BTN_PAT_DN = 15,
};

#define BTN_DEBOUNCE_MS           30
#define BTN_LONG_PRESS_MS         600
#define BTN_IDLE_SLEEP_MS         (5 * 60 * 1000)   /* 5 min idle → deep sleep */

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