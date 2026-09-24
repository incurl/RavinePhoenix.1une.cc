/*
 * test_patterns.c — hard-coded demo pattern (kick + hat + bassline).
 *
 * Useful for sanity-testing the engine without recording any samples.
 */
#include "sequencer/pattern.h"
#include "sequencer/sequencer.h"
#include "audio/amy_bridge.h"
#include <string.h>

/* Soft-load demo pattern 0: kick on 0,4,8,12; hat on 2,6,10,14; bass every other. */
void test_patterns_load_demo(void)
{
    sequencer_set_step_slot(0, 0,  0, 60);   /* kick */
    sequencer_set_step_slot(0, 4,  0, 60);
    sequencer_set_step_slot(0, 8,  0, 60);
    sequencer_set_step_slot(0, 12, 0, 60);

    sequencer_set_step_slot(0, 2,  1, 60);   /* hat */
    sequencer_set_step_slot(0, 6,  1, 60);
    sequencer_set_step_slot(0, 10, 1, 60);
    sequencer_set_step_slot(0, 14, 1, 60);

    for (int i = 0; i < 16; i += 2) {
        sequencer_set_step_slot(0, i, 8, 48 + (i / 2) * 2);  /* bass */
    }
}

void test_patterns_load_all(void)
{
    pattern_init_all();
    test_patterns_load_demo();
}