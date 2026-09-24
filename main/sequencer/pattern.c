/*
 * pattern.c — pattern storage + helpers.
 */
#include "pattern.h"
#include <string.h>

pattern_t g_patterns[PATTERN_COUNT];
uint8_t    g_chain[PATTERN_CHAIN_MAX];
uint8_t    g_chain_len = 0;

void pattern_init_all(void)
{
    for (int p = 0; p < PATTERN_COUNT; p++) {
        pattern_clear(&g_patterns[p]);
        snprintf(g_patterns[p].name, sizeof(g_patterns[p].name),
                 "PAT %d", p);
    }
    g_chain_len = 0;
}

void pattern_clear(pattern_t *p)
{
    memset(p, 0, sizeof(*p));
    for (int i = 0; i < STEPS_PER_PATTERN; i++) {
        p->steps[i].slot_id = 0xFF;   /* empty */
    }
}

void pattern_set_step(uint8_t pattern, uint8_t step, const step_t *s)
{
    if (pattern >= PATTERN_COUNT || step >= STEPS_PER_PATTERN) return;
    g_patterns[pattern].steps[step] = *s;
}

void pattern_get_step(uint8_t pattern, uint8_t step, step_t *out)
{
    if (pattern >= PATTERN_COUNT || step >= STEPS_PER_PATTERN) {
        memset(out, 0, sizeof(*out));
        out->slot_id = 0xFF;
        return;
    }
    *out = g_patterns[pattern].steps[step];
}