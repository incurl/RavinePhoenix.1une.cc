/*
 * test_buttons.c — verify button-ID layout after the 16-step expansion.
 *
 * The hardware layout is:
 *   - 8 dedicated modifier buttons (REC, PLAY, FUNC, FX, BPM±, PAT±)
 *     on dedicated GPIOs. They occupy BTN IDs 0..7.
 *   - 16 polymorphic step buttons on a 4x4 matrix. They occupy
 *     BTN IDs 8..23 (BTN_STEP1..BTN_STEP16).
 *
 * These tests verify the enum, the BTN_IS_STEP macro, and the matrix
 * row-major mapping that buttons.c uses.
 */
#include "unity.h"
#include "config.h"

TEST_CASE("button enum has 24 IDs", "[buttons]")
{
    TEST_ASSERT_EQUAL(0, BTN_REC);
    TEST_ASSERT_EQUAL(1, BTN_PLAY);
    TEST_ASSERT_EQUAL(2, BTN_FUNC);
    TEST_ASSERT_EQUAL(3, BTN_FX);
    TEST_ASSERT_EQUAL(4, BTN_BPM_UP);
    TEST_ASSERT_EQUAL(5, BTN_BPM_DN);
    TEST_ASSERT_EQUAL(6, BTN_PAT_UP);
    TEST_ASSERT_EQUAL(7, BTN_PAT_DN);
    TEST_ASSERT_EQUAL(8, BTN_STEP1);
    TEST_ASSERT_EQUAL(23, BTN_STEP16);
    TEST_ASSERT_EQUAL(24, BTN_COUNT);
}

TEST_CASE("BTN_IS_STEP macro is correct", "[buttons]")
{
    /* All 8 modifier buttons are NOT steps. */
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_REC));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_PLAY));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_FUNC));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_FX));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_BPM_UP));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_BPM_DN));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_PAT_UP));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_PAT_DN));

    /* All 16 step buttons ARE steps. */
    for (int i = BTN_STEP1; i <= BTN_STEP16; i++) {
        TEST_ASSERT_TRUE_MESSAGE(BTN_IS_STEP(i),
            "expected step button");
    }

    /* Boundary check. */
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_COUNT));
    TEST_ASSERT_FALSE(BTN_IS_STEP(255));
}

TEST_CASE("button counts add up", "[buttons]")
{
    /* 16 matrix positions + 8 dedicated GPIOs = 24 logical buttons. */
    TEST_ASSERT_EQUAL(24, BTN_COUNT);
    TEST_ASSERT_EQUAL(4,  BTN_ROW_COUNT);
    TEST_ASSERT_EQUAL(4,  BTN_COL_COUNT);
    TEST_ASSERT_EQUAL(16, BTN_ROW_COUNT * BTN_COL_COUNT);
    TEST_ASSERT_EQUAL(8,  BTN_GPIO_COUNT);
    TEST_ASSERT_EQUAL(24, BTN_ROW_COUNT * BTN_COL_COUNT + BTN_GPIO_COUNT);
}

TEST_CASE("GPIO assignments don't collide with I2S / TFT / etc.", "[buttons]")
{
    /* We rely on GPIO 11, 12, 13, 41, 42, 43, 44, 45 for modifier buttons.
     * Verify they don't overlap with any other subsystem. */
    const gpio_num_t used_by_others[] = {
        I2S_OUT_BCLK_GPIO, I2S_OUT_LRCK_GPIO, I2S_OUT_DATA_GPIO,
        I2S_IN_BCLK_GPIO,  I2S_IN_LRCK_GPIO,  I2S_IN_DATA_GPIO,
        TFT_MOSI_GPIO, TFT_SCK_GPIO, TFT_CS_GPIO, TFT_DC_GPIO,
        TFT_RST_GPIO, TFT_BL_GPIO,
        LED_REC_GPIO, LED_PLAY_GPIO,
        SYNC_OUT_GPIO, SYNC_IN_GPIO,
        BTN_ROW_PINS[0], BTN_ROW_PINS[1], BTN_ROW_PINS[2], BTN_ROW_PINS[3],
        BTN_COL_PINS[0], BTN_COL_PINS[1], BTN_COL_PINS[2], BTN_COL_PINS[3],
    };
    const gpio_num_t mine[] = {
        BTN_REC_GPIO, BTN_PLAY_GPIO, BTN_FUNC_GPIO, BTN_FX_GPIO,
        BTN_BPM_UP_GPIO, BTN_BPM_DN_GPIO,
        BTN_PAT_UP_GPIO, BTN_PAT_DN_GPIO,
    };
    for (size_t i = 0; i < sizeof(mine) / sizeof(mine[0]); i++) {
        for (size_t j = 0; j < sizeof(used_by_others) / sizeof(used_by_others[0]); j++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(mine[i], used_by_others[j],
                "GPIO collision between modifier button and another subsystem");
        }
    }
}