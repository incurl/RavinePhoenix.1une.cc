/*
 * input.h — button-event dispatcher.
 *
 * Drains the FreeRTOS button queue (populated by buttons_tick()) and
 * routes each event to the matching subsystem handler (transport,
 * pattern selection, BPM, REC, FX, WRITE, step toggles, etc.).
 *
 * For now this is a single switch in input_dispatch() that prints each
 * event and dispatches the few transport-level actions that have an
 * existing sequencer_* / amy_bridge_* API to call. The full UI state
 * machine (write mode, sketch picker, tweak-mode UX) is queued for
 * follow-up commits; each handler that lands later will land as a new
 * case here.
 *
 * Designed to be called from button_scan_task at ~10 ms cadence. The
 * function drains *all* queued events per call so a button-burst
 * during a brief scan gap never loses input.
 */
#ifndef PO33_INPUT_H
#define PO33_INPUT_H

#include "ui/buttons.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Drain every event currently in the button queue. No-op if empty. */
void input_drain(void);

#ifdef __cplusplus
}
#endif

#endif /* PO33_INPUT_H */