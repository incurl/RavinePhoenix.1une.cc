/*
 * test_amy_bridge.c — Unity smoke test for the AMY bridge.
 *
 * Verifies that:
 *   1. amy_bridge_init() succeeds.
 *   2. We can register a slot with synthetic audio.
 *   3. amy_bridge_play_note() does not crash and AMY reports no OOM.
 */
#include "unity.h"
#include "audio/amy_bridge.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TEST_CASE("amy_bridge_init smoke", "[amy]")
{
    esp_err_t e = amy_bridge_init();
    TEST_ASSERT_EQUAL(ESP_OK, e);
    TEST_ASSERT_EQUAL_UINT32(0, amy_get_oom_count());
}

TEST_CASE("register and trigger a slot", "[amy]")
{
    /* 0.25 s of 440 Hz sine at 44100 Hz mono int16. */
    const size_t n = 44100 / 4;
    static int16_t sine[44100 / 4];
    for (size_t i = 0; i < n; i++) {
        sine[i] = (int16_t)(8000.0 * sin(2.0 * M_PI * 440.0 * i / 44100.0));
    }
    esp_err_t e = amy_bridge_register_slot(8, sine, n, 44100, false);
    TEST_ASSERT_EQUAL(ESP_OK, e);
    TEST_ASSERT_EQUAL_size_t(n, amy_bridge_slot_len_samples(8));

    e = amy_bridge_play_note(8, 60, 100, PO33_FX_NONE, 0, 0);
    TEST_ASSERT_EQUAL(ESP_OK, e);

    /* AMY's bridge should still report no OOM after a single note. */
    TEST_ASSERT_EQUAL_UINT32(0, amy_get_oom_count());
}