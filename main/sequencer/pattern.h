/*
 * pattern.h — pattern + step data structures.
 */
#ifndef PO33_PATTERN_H
#define PO33_PATTERN_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct {
    uint8_t  slot_id;        /* 0..15, 0xFF = empty */
    uint8_t  note;           /* 0..127, 60 = middle C */
    uint8_t  velocity;       /* 0..127 */
    uint8_t  filter_cutoff;  /* 0..255 */
    uint8_t  filter_resonance; /* 0..255 (F-017). 0 = no resonance,
                                * ~128 = moderate resonance, 255 = max. */
    uint8_t  effect;         /* effect_id_t */
    uint8_t  effect_p1;
    uint8_t  effect_p2;
    bool     plock_active;   /* if true, this step has overrides */
} step_t;

typedef struct {
    step_t steps[STEPS_PER_PATTERN];
    char   name[16];
} pattern_t;

/* The 16 patterns + chain buffer */
extern pattern_t g_patterns[PATTERN_COUNT];
extern uint8_t   g_chain[PATTERN_CHAIN_MAX];
extern uint8_t   g_chain_len;

void pattern_init_all(void);
void pattern_clear(pattern_t *p);
void pattern_set_step(uint8_t pattern, uint8_t step, const step_t *s);
void pattern_get_step(uint8_t pattern, uint8_t step, step_t *out);

#endif