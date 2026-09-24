/*
 * test_sequencer.c — Unity tests for pattern + sequencer.
 *
 * Build with `idf.py -C build test` after enabling Unity in menuconfig.
 */
#include "unity.h"
#include "sequencer/pattern.h"
#include "sequencer/sequencer.h"

TEST_CASE("pattern starts empty", "[pattern]")
{
    pattern_init_all();
    TEST_ASSERT_EQUAL_UINT8(0xFF, g_patterns[0].steps[0].slot_id);
}

TEST_CASE("step slot assignment roundtrips", "[pattern]")
{
    pattern_init_all();
    sequencer_set_step_slot(0, 5, 3, 67);
    step_t s;
    pattern_get_step(0, 5, &s);
    TEST_ASSERT_EQUAL_UINT8(3, s.slot_id);
    TEST_ASSERT_EQUAL_UINT8(67, s.note);
    TEST_ASSERT_TRUE(s.plock_active);
}

TEST_CASE("bpm clamps to range", "[sequencer]")
{
    sequencer_set_bpm(10);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(MIN_BPM, sequencer_get_bpm());
    sequencer_set_bpm(9999);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(MAX_BPM, sequencer_get_bpm());
}

TEST_CASE("pattern select valid", "[sequencer]")
{
    sequencer_set_pattern(7);
    TEST_ASSERT_EQUAL_UINT8(7, sequencer_get_current_pattern());
}