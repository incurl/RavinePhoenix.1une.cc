/*
 * test_buttons.c — verify the button-ID layout after the PO-33 modifier rework.
 *
 * The hardware layout is:
 *   - 7 dedicated modifier buttons, laid out PO-33 style:
 *       top row      (left to right):  SOUND, PATTERN, BPM
 *       right column (top to bottom):  REC, FX, PLAY, WRITE
 *     The column sits under Knob B, beside the step matrix.
 *     They occupy BTN IDs 0..6.
 *   - 16 polymorphic step buttons on a 4x4 matrix. The matrix costs
 *     only 8 GPIOs (4 row + 4 col scan). They occupy BTN IDs 7..22
 *     (BTN_STEP1..BTN_STEP16).
 *
 * These tests verify the enum, the BTN_IS_STEP macro, the GPIO budget,
 * and the matrix row-major mapping that buttons.c uses.
 */
#include "unity.h"
#include "config.h"

TEST_CASE("button enum has 23 IDs", "[buttons]")
{
    /* Top row. */
    TEST_ASSERT_EQUAL(0, BTN_SOUND);
    TEST_ASSERT_EQUAL(1, BTN_PATTERN);
    TEST_ASSERT_EQUAL(2, BTN_BPM);

    /* Right column (under Knob B). */
    TEST_ASSERT_EQUAL(3, BTN_REC);
    TEST_ASSERT_EQUAL(4, BTN_FX);
    TEST_ASSERT_EQUAL(5, BTN_PLAY);
    TEST_ASSERT_EQUAL(6, BTN_WRITE);

    /* Step matrix. */
    TEST_ASSERT_EQUAL(7, BTN_STEP1);
    TEST_ASSERT_EQUAL(22, BTN_STEP16);
    TEST_ASSERT_EQUAL(23, BTN_COUNT);
}

TEST_CASE("step button IDs are contiguous", "[buttons]")
{
    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQUAL(BTN_STEP1 + i, 7 + i);
    }
}

TEST_CASE("BTN_IS_STEP macro is correct", "[buttons]")
{
    /* All 7 modifier buttons are NOT steps. */
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_SOUND));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_PATTERN));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_BPM));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_REC));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_FX));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_PLAY));
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_WRITE));

    /* All 16 step buttons ARE steps. */
    for (int i = BTN_STEP1; i <= BTN_STEP16; i++) {
        TEST_ASSERT_TRUE_MESSAGE(BTN_IS_STEP(i), "expected step button");
    }

    /* Boundary check. */
    TEST_ASSERT_FALSE(BTN_IS_STEP(BTN_COUNT));
    TEST_ASSERT_FALSE(BTN_IS_STEP(255));
}

TEST_CASE("button counts add up", "[buttons]")
{
    /* 16 matrix positions + 7 dedicated modifier GPIOs = 23 logical buttons. */
    TEST_ASSERT_EQUAL(23, BTN_COUNT);
    TEST_ASSERT_EQUAL(16, BTN_ROW_COUNT * BTN_COL_COUNT);
    TEST_ASSERT_EQUAL(7,  BTN_GPIO_COUNT);
    TEST_ASSERT_EQUAL(23, BTN_ROW_COUNT * BTN_COL_COUNT + BTN_GPIO_COUNT);
}

TEST_CASE("GPIO budget fits the ESP32-S3", "[buttons]")
{
    /* Buttons: 8 matrix pins (4 row + 4 col) + 7 modifier GPIOs = 15.
     * Plus 2 analog knob channels = 17 button/knob GPIOs.
     * Other subsystems: I2S 6 + TFT 6 + LEDs 2 + sync 2 = 16.
     * Grand total 33 of the WROOM-1's 45 GPIOs, leaving 12 free. */
    const int matrix_pins   = BTN_ROW_COUNT + BTN_COL_COUNT;   /* 8  */
    const int modifier_pins = BTN_GPIO_COUNT;                  /* 7  */
    const int button_pins   = matrix_pins + modifier_pins;     /* 15 */
    const int grand_total   = button_pins + 2 + 6 + 6 + 2 + 2; /* 33 */

    TEST_ASSERT_EQUAL(8,  matrix_pins);
    TEST_ASSERT_EQUAL(15, button_pins);
    TEST_ASSERT_EQUAL(33, grand_total);
    TEST_ASSERT_LESS_OR_EQUAL(45, grand_total);
}

TEST_CASE("modifier GPIOs are the 7 expected pins", "[buttons]")
{
    TEST_ASSERT_EQUAL(GPIO_NUM_11, BTN_SOUND_GPIO);
    TEST_ASSERT_EQUAL(GPIO_NUM_44, BTN_PATTERN_GPIO);
    TEST_ASSERT_EQUAL(GPIO_NUM_13, BTN_BPM_GPIO);
    TEST_ASSERT_EQUAL(GPIO_NUM_41, BTN_REC_GPIO);
    TEST_ASSERT_EQUAL(GPIO_NUM_12, BTN_FX_GPIO);
    TEST_ASSERT_EQUAL(GPIO_NUM_42, BTN_PLAY_GPIO);
    TEST_ASSERT_EQUAL(GPIO_NUM_43, BTN_WRITE_GPIO);
}

TEST_CASE("GPIO assignments don't collide with I2S / TFT / etc.", "[buttons]")
{
    /* BTN_ROW_PINS / BTN_COL_PINS are brace initializers, so bind them
     * to local arrays before indexing (BTN_ROW_PINS[0] does not parse). */
    const gpio_num_t rows[BTN_ROW_COUNT] = BTN_ROW_PINS;
    const gpio_num_t cols[BTN_COL_COUNT] = BTN_COL_PINS;

    const gpio_num_t used_by_others[] = {
        I2S_OUT_BCLK_GPIO, I2S_OUT_LRCK_GPIO, I2S_OUT_DATA_GPIO,
        I2S_IN_BCLK_GPIO,  I2S_IN_LRCK_GPIO,  I2S_IN_DATA_GPIO,
        TFT_MOSI_GPIO, TFT_SCK_GPIO, TFT_CS_GPIO, TFT_DC_GPIO,
        TFT_RST_GPIO, TFT_BL_GPIO,
        LED_REC_GPIO, LED_PLAY_GPIO,
        SYNC_OUT_GPIO, SYNC_IN_GPIO,
        KNOB_A_GPIO, KNOB_B_GPIO,
        rows[0], rows[1], rows[2], rows[3],
        cols[0], cols[1], cols[2], cols[3],
    };
    const gpio_num_t mine[] = {
        BTN_SOUND_GPIO, BTN_PATTERN_GPIO, BTN_BPM_GPIO,
        BTN_REC_GPIO, BTN_FX_GPIO, BTN_PLAY_GPIO, BTN_WRITE_GPIO,
    };

    /* No modifier GPIO may collide with another subsystem. */
    for (size_t i = 0; i < sizeof(mine) / sizeof(mine[0]); i++) {
        for (size_t j = 0; j < sizeof(used_by_others) / sizeof(used_by_others[0]); j++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(mine[i], used_by_others[j],
                "GPIO collision between modifier button and another subsystem");
        }
    }

    /* ...and the 7 modifier GPIOs must be distinct from each other. */
    for (size_t i = 0; i < sizeof(mine) / sizeof(mine[0]); i++) {
        for (size_t j = i + 1; j < sizeof(mine) / sizeof(mine[0]); j++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(mine[i], mine[j],
                "duplicate modifier button GPIO");
        }
    }
}

TEST_CASE("no modifier button uses a strapping pin", "[buttons]")
{
    /* GPIO 0-3 are strapping pins on the ESP32-S3 and must stay free. */
    const gpio_num_t mine[] = {
        BTN_SOUND_GPIO, BTN_PATTERN_GPIO, BTN_BPM_GPIO,
        BTN_REC_GPIO, BTN_FX_GPIO, BTN_PLAY_GPIO, BTN_WRITE_GPIO,
    };
    for (size_t i = 0; i < sizeof(mine) / sizeof(mine[0]); i++) {
        TEST_ASSERT_GREATER_THAN_MESSAGE(GPIO_NUM_3, mine[i],
            "modifier button must not use a strapping pin (GPIO 0-3)");
    }
}

TEST_CASE("matrix row-major mapping matches buttons.c", "[buttons]")
{
    /* buttons.c computes: btn = BTN_STEP1 + r * BTN_COL_COUNT + c */
    for (int r = 0; r < BTN_ROW_COUNT; r++) {
        for (int c = 0; c < BTN_COL_COUNT; c++) {
            int id = BTN_STEP1 + r * BTN_COL_COUNT + c;
            TEST_ASSERT_TRUE_MESSAGE(BTN_IS_STEP(id),
                "row-major mapping produced a non-step ID");
        }
    }
    /* Corner cases. */
    TEST_ASSERT_EQUAL(BTN_STEP1,  BTN_STEP1 + 0 * 4 + 0);
    TEST_ASSERT_EQUAL(BTN_STEP4,  BTN_STEP1 + 0 * 4 + 3);
    TEST_ASSERT_EQUAL(BTN_STEP5,  BTN_STEP1 + 1 * 4 + 0);
    TEST_ASSERT_EQUAL(BTN_STEP16, BTN_STEP1 + 3 * 4 + 3);
}
