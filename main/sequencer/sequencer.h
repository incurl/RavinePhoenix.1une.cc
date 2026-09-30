/*
 * sequencer.h — 16-step sequencer with pattern chain.
 */
#ifndef PO33_SEQUENCER_H
#define PO33_SEQUENCER_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sequencer_init(void);

/* Transport */
void sequencer_play(void);
void sequencer_stop(void);
bool sequencer_is_playing(void);
void sequencer_set_bpm(uint16_t bpm);
void sequencer_set_pattern(uint8_t pattern);
uint16_t sequencer_get_bpm(void);
uint8_t  sequencer_get_current_step(void);
uint8_t  sequencer_get_current_pattern(void);

/* Pattern building helpers */
void sequencer_set_step_slot(uint8_t pattern, uint8_t step,
                             uint8_t slot, uint8_t note);

/* Called from audio task on every render block. */
void sequencer_tick(void);

/* Debug */
void sequencer_print_status(void);

#ifdef __cplusplus
}
#endif

#endif