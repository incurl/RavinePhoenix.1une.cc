/*
 * test_knobs.c — sanity checks for the knob driver.
 *
 * The hardware tests can't actually exercise the ADCs in unit-test
 * context (no pots wired up), but we can verify:
 *   - the GPIO assignments don't collide with any other subsystem
 *   - the knob config macros are internally consistent
 */
#include "unity.h"
#include "config.h"

TEST_CASE("knob GPIOs are non-strapping", "[knobs]")
{
    /* GPIO 0..3 are strapping pins on the ESP32-S3 (sampled at reset
     * to set boot mode and JTAG source). Using them as ADC inputs
     * after boot is technically possible but discouraged. */
    TEST_ASSERT_NOT_EQUAL(0, KNOB_A_GPIO);
    TEST_ASSERT_NOT_EQUAL(0, KNOB_B_GPIO);
    TEST_ASSERT_NOT_EQUAL(1, KNOB_A_GPIO);
    TEST_ASSERT_NOT_EQUAL(1, KNOB_B_GPIO);
    TEST_ASSERT_NOT_EQUAL(2, KNOB_A_GPIO);
    TEST_ASSERT_NOT_EQUAL(2, KNOB_B_GPIO);
    TEST_ASSERT_NOT_EQUAL(3, KNOB_A_GPIO);
    TEST_ASSERT_NOT_EQUAL(3, KNOB_B_GPIO);
}

TEST_CASE("knob GPIOs are distinct from each other", "[knobs]")
{
    TEST_ASSERT_NOT_EQUAL(KNOB_A_GPIO, KNOB_B_GPIO);
    TEST_ASSERT_NOT_EQUAL(KNOB_A_ADC_CHANNEL, KNOB_B_ADC_CHANNEL);
}

TEST_CASE("knob GPIOs don't collide with other subsystems", "[knobs]")
{
    const gpio_num_t used_by_others[] = {
        I2S_OUT_BCLK_GPIO, I2S_OUT_LRCK_GPIO, I2S_OUT_DATA_GPIO,
        I2S_IN_BCLK_GPIO,  I2S_IN_LRCK_GPIO,  I2S_IN_DATA_GPIO,
        TFT_MOSI_GPIO, TFT_SCK_GPIO, TFT_CS_GPIO, TFT_DC_GPIO,
        TFT_RST_GPIO, TFT_BL_GPIO,
        LED_REC_GPIO, LED_PLAY_GPIO,
        SYNC_OUT_GPIO, SYNC_IN_GPIO,
        BTN_ROW_PINS[0], BTN_ROW_PINS[1], BTN_ROW_PINS[2], BTN_ROW_PINS[3],
        BTN_COL_PINS[0], BTN_COL_PINS[1], BTN_COL_PINS[2], BTN_COL_PINS[3],
        BTN_REC_GPIO, BTN_PLAY_GPIO, BTN_FUNC_GPIO, BTN_FX_GPIO,
        BTN_BPM_UP_GPIO, BTN_BPM_DN_GPIO,
        BTN_PAT_UP_GPIO, BTN_PAT_DN_GPIO,
    };
    const gpio_num_t mine[] = { KNOB_A_GPIO, KNOB_B_GPIO };
    for (size_t i = 0; i < sizeof(mine) / sizeof(mine[0]); i++) {
        for (size_t j = 0; j < sizeof(used_by_others) / sizeof(used_by_others[0]); j++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(mine[i], used_by_others[j],
                "GPIO collision between knob and another subsystem");
        }
    }
}

TEST_CASE("knob smoothing constants are sane", "[knobs]")
{
    /* KNOB_SAMPLES must be at least 1 (no point otherwise) and should
     * be small enough that 8 samples * 2 knobs at the button-scan rate
     * (100 Hz) doesn't drown the CPU. */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(1, KNOB_SAMPLES);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(32, KNOB_SAMPLES);
    /* KNOB_DEADZONE must be small enough that the knob still feels
     * responsive: with 0..255 scale, a dead-zone of 4 is ~1.5 %. */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(1, KNOB_DEADZONE);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(16, KNOB_DEADZONE);
}