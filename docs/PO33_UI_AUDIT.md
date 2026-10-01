# PO-33 UI Audit — Full Verbatim Manual vs Codebase

Source of truth: [lode/PO-33 README](https://raw.githubusercontent.com/lode/PO-33/main/README.md), fetched 2026-09-30. This is the canonical PO-33 manual mirror.

## The 27 PO-33 manual combos

| # | PO-33 Manual (verbatim)                                                | Our Docs Claim                                              | Our Code Behaviour                       | Verdict |
|---|-----------------------------------------------------------------------|-------------------------------------------------------------|------------------------------------------|---------|
| 1 | hold record (star) + number → record own sound                         | F-001: ✅ done. UART: `rec <slot>` then `stoprec`           | BTN_REC: no-op (no binding)              | ❌ DOC LIE: claim "✅ done"; actual is missing |
| 2 | [play pattern] hold write (·) + number(s) → record own sound (live)    | F-003: ❌ missing. UART-only; firmware can't live-record    | BTN_WRITE: no-op                          | ❌ MISSING |
| 3 | hold sound (S) + number → select sound                                 | F-006: hold SOUND + step 1-16 (selects slot)                | SOUND + step → sequencer_set_active_slot  | ✅ done (b4fce86) |
| 4 | [select sound] press number → play a sound                             | F-006 layman implies one-press; manual is two-press. Strict | step press w/o modifier → play active     | ✅ done (b4fce86) |
| 5 | hold pattern (⠛) + number → select pattern                             | F-007: hold PATTERN + step 1-16                              | PATTERN + step → sequencer_set_pattern   | ✅ done (1b60815) |
| 6 | [select pattern] press write (·) → enter/exit write mode               | F-009/F-010: queued for v2                                   | BTN_WRITE: no-op                          | ❌ MISSING |
| 7 | [write mode] press sound#, press step#s → fill pattern                  | F-009/F-010: queued for v2                                   | no write mode                             | ❌ MISSING |
| 8 | [select pattern] press play (>) → play pattern                         | PLAY tap → sequencer_play/stop toggle (the PO-33 PLAY LED mirrors the new state) | PLAY tap → sequencer_play/stop toggle; PLAY LED via leds_set_play | ✅ DONE (matches PO-33) |
| 9 | hold pattern (⠛) + number(s) → change patterns (chain)                | F-013: chain build (partial); F-014: reorder (missing)        | pattern_on_step replaces, not appends    | ⚠️ WRONG semantics (we replace; PO-33 appends) |
|10 | press fx (FX) → toggle tweak parameter (Tone/Filter/Trim)             | F-015: FX tap cycles s_tweak_mode TONE → FILTER → TRIM → TONE | tweak_mode_cycle() called from BTN_FX tap dispatch | ✅ done (tweak-mode commit) |
|11 | [select tweak] turn knob A/B → tweak parameter                          | F-016/F-017/F-018: knob bindings wired via tweak_apply_step / tweak_apply_slot | TONE: Knob A -> MIDI note, Knob B -> velocity; FILTER: Knob A -> filter_cutoff (resonance TODO); TRIM: Knob A/B -> amy_bridge_set_trim on active slot | ✅ done (knob A in all 3 modes; knob B in TONE only; knob B in FILTER is logged TODO; knob B in TRIM sets trim end). Caveat: PO-33 has dedicated tweak-target buttons (S+1, S+2, ...) we lack. |
|12 | [play pattern] hold fx + number (1-15) → add effect                    | F-019: FX + step 1-15 sets active_fx (carried to next note)  | FX + step 1-15 → sequencer_set_active_fx | ✅ done (b4fce86); but **does NOT save into pattern** — this is the "add effect" semantics, not "add+save in pattern" |
|13 | [write mode] [play] hold fx + number (1-15) → add+save effect in pattern | F-019: "save in pattern queued"                              | not implemented                           | ❌ MISSING (depends on write mode, #6) |
|14 | [write mode] [play] hold fx + 16 → clear effect in pattern             | F-019: step 16 = PO33_FX_NONE ("no effect")                   | step 16 sets PO33_FX_NONE                | ✅ done (badaa4f). The PO-33 "clear in pattern" half lands in write mode (F-009, queued). |
|15 | hold bpm (handle) + turn knob A → change swing                         | F-020: BPM long-press + Knob A = swing (8 levels); release keeps the level | sequencer_set_swing() from s_bpm_held polling; on_step() defers off-beat notes via s_swing_timer | ✅ done (badaa4f) |
|16 | hold bpm (handle) + turn knob B → change tempo (fine tuned)            | F-020: BPM long-press + Knob B = continuous fine BPM         | knobs_get_b() read; knob_b_to_bpm() maps to [60..240] | ✅ done (badaa4f) |
|17 | press bpm (handle) → change tempo (pre-defined modes)                  | F-020/F-021: tap cycles 80/120/140 (Hip Hop/Disco/Techno)   | BPM tap → sequencer_cycle_bpm_preset     | ✅ DONE |
|18 | hold bpm (handle) + number (1-5) → change volume (max 5)                    | F-022: ✅ done. Hold BPM + step 1-5 → amy_bridge_set_volume_level(N); multiplier applied per-note in amy_bridge_play_note() | s_bpm_held branch in input_drain() routes BPM-held + step 1-5 BEFORE the modifier lookup (because BPM isn't a press-bound modifier) | ✅ done (volume commit). Display-half (press BPM alone to show current level; per PO-33 "numbers are lit until the current level") queued for v2 in display.c. |
|19 | [select sound] hold write + sound + number → copy sound                 | F-023: ❌ missing                                              | no WRITE handler                          | ❌ MISSING (multi-modifier combo; not supported by framework) |
|20 | hold write + sound + 9-16 + 1-16 → copy slice                          | F-024: ❌ missing                                              | no WRITE handler                          | ❌ MISSING |
|21 | [select pattern] hold write + pattern + number → copy pattern           | F-025: ❌ missing                                              | no WRITE handler                          | ❌ MISSING |
|22 | [select sound] hold record + sound → delete sound                       | F-026: ❌ missing                                              | no REC handler                            | ❌ MISSING |
|23 | [select pattern] hold record + pattern → delete pattern                 | F-027: ❌ missing                                              | no REC handler                            | ❌ MISSING |
|24 | (Power off) hold pattern + insert batteries → factory reset            | F-037: storage_erase_all (not yet written)                   | no UI                                     | ❌ MISSING |
|25 | press sound + bpm → battery status                                       | (not in docs)                                                | no UI                                     | ❌ MISSING |
|26 | press bpm → volume level (numbers lit until current)                    | F-022: set-half done; display-half queued | BPM tap → sequencer_cycle_bpm_preset; display.c doesn't render the level on the matrix | ❌ MISSING (display-only; the per-step volume-level binding via F-022 is wired). PO-33 uses BPM tap for BOTH tempo cycling AND volume display cycling. |
|27 | press sound OR pattern → active sounds/patterns                         | F-038: rendering missing                                      | no UI                                     | ❌ MISSING |

## Critical findings — the big lies

### Lie #1: BPM + Knob A = swing (not "fine tempo")
**PO-33 manual:** `change swing` = `hold bpm (handle) + turn knob A`. `change tempo (fine tuned)` = `hold bpm (handle) + turn knob B`.
**Our code/docs:** BPM long-press + Knob A = fine tempo adjust.
**Both halves of the BPM-row binding are swapped.** This is in commit `b0be685` and propagates to docs §1.4, F-020, F-021, HARDWARE.md §4.5b, HARDWARE.md line 240, cheat sheet §8 line 1120, S5.4 line 787.

### Lie #2: FX + 16 = swing (it's "clear effect in pattern")
**PO-33 manual:** `clear effect in pattern` = `[write mode] [play] hold fx (FX) + 16`. `change swing` is the BPM+knob A combo (above). The effects list 1-15 is the 15 effects; entry 16 in the manual's effects table is "no effect" (not swing).
**Our code/docs:** FX + 16 = swing stub. `fx_on_step()` logs "swing not yet implemented".
**Invented mapping; doesn't exist on PO-33.** This is in commit `b4fce86` and propagates to F-019, HARDWARE.md line 242, cheat sheet §8 line 1121.

### Lie #3: F-015 says FX needs a "separate Tweak button"
**PO-33 manual:** `press fx (FX) to toggle between different parameters` (Tone/Filter/Trim). FX tap IS the tweak-mode toggle.
**Our docs:** "We'd need a separate 'Tweak' mode button (perhaps SOUND + FX)."
**Same FX button does tweak-mode cycling AND punch-in effects. Two roles, one button.** Our docs have it wrong.

### Lie #4: PO-33 effects list 1-16 vs our 16 punch-ins
**PO-33 manual effects list (verbatim, 16 entries):**
1. loop 16, 2. loop 12, 3. loop short, 4. loop shorter, 5. unison, 6. unison low, 7. octave up, 8. octave down, 9. stutter 4, 10. stutter 3, 11. scratch, 12. scratch fast, 13. 6/8 quantize, 14. retrigger pattern, 15. reverse, **16. no effect**.
**Our `po33_fx_t`:** NONE(0), LOOP_16(1), LOOP_12(2), LOOP_SHORT(3), LOOP_SHORTER(4), UNISON(5), UNISON_LOW(6), OCTAVE_UP(7), OCTAVE_DOWN(8), STUTTER_4(9), STUTTER_3(10), SCRATCH_FAST(11), REVERSE(12), RETRIGGER_PATTERN(13), 68_QUANTIZE(14), FILTER_SWEEP(15), BITCRUSH(16), COUNT=18.
**Three problems:** (a) PO-33 has `scratch` (entry 11) and `scratch fast` (12); we have only `SCRATCH_FAST`. (b) PO-33 has 15 effects + "no effect" as 16; we have 16 effects + NONE separately. (c) Order of UNISON_LOW/OCTAVE_DOWN/etc. differs (PO-33: octave up=7, octave down=8, unison low=6; ours: UNISON_LOW=6, OCTAVE_UP=7, OCTAVE_DOWN=8 — same order but PO-33 puts unison low before octaves).

### Lie #5: F-001 "✅ done" claim vs no binding
F-001 status says "✅ done" because the code has `amy_bridge_start_record()`. But BTN_REC has no input binding — pressing REC does nothing. The status should be "⚠️ partial" or "❌ missing".

### Lie #6: BPM tap conflict — volume OR tempo?
**PO-33 manual:** `change tempo (pre-defined modes)` = `press bpm (handle) to toggle between different levels`. AND `volume level` = `press bpm (handle); numbers are lit until the current level`.
**Our code:** BPM tap → cycle tempo presets (80/120/140).
**The PO-33 uses BPM tap for BOTH tempo cycling AND volume display cycling.** These are distinguished by state (when headphones connected vs not, per the manual's note). We can't replicate this distinction without knowing the current volume level state, which is unimplemented. PO-33 wins on this one — single button, dual purpose, distinguished by state.

## Where we DO match PO-33 ground truth

| Item | Notes |
|---|---|
| Item | Notes |
|---|---|
| PLAY tap → play/stop toggle | Matches PO-33 "press play (>)" |
| BPM tap → cycle tempo presets (Hip Hop 80 / Disco 120 / Techno 140) | Matches PO-33 "press bpm (handle) to toggle between different levels" |
| PATTERN held + step → select pattern AND append to chain | Matches PO-33 "hold pattern (⠛) + number" + "change patterns" |
| SOUND held + step → select slot | Matches PO-33 "hold sound (S) + number" |
| Step press without modifier after SOUND select → plays once | Matches PO-33 "[select sound] press number" |
| REC held + step → record; release → stop | Matches PO-33 "hold record (star) + number, make sound, release buttons" |
| BPM held + Knob A → swing (8 levels) | Matches PO-33 "change swing" |
| BPM held + Knob B → fine BPM | Matches PO-33 "change tempo (fine tuned)" |
| FX held + step 1–15 → set active FX | Matches PO-33 "hold fx (FX) + number (1-15)" |
| FX held + step 16 → PO33_FX_NONE ("no effect") | Matches PO-33 manual effects table entry 16 |
| FX tap → cycle tweak parameter (Tone → Filter → Trim → Tone) | Matches PO-33 "press fx (FX) to toggle between different parameters" |
| In tweak mode, knob A/B adjust the active parameter | Matches PO-33 "[select tweak parameter] turn knob A/B" |
| BPM held + step 1–5 → volume level (set-half) | Matches PO-33 "change volume". The PO-33 also uses BPM tap alone to *display* the level; the display-half is queued for v2. |

## Audit updates after the encoder + sketch-picker + storage commits

The picker itself isn't a single line in the 27-row table — it lives
in `docs/DESIGN.md` §11.9 as a v2 UI proposal. With the encoder
driver (commit `a5d22a7`), the picker state machine (commit `2539b14`),
and the storage layer + UART verbs (this commit) landed, the picker
end-to-end works. Still queued:

- TFT rendering of the picker (display.c work)
- `apply_fx()` cases that are still no-ops (LOOP_16, LOOP_12, ...)
- F-038 active sounds/patterns display (per-step + display per-mode)
- Atomic-rename pattern for sketch saves (currently LittleFS write;
  docs §11.8 lists it as future hardening).

## What's queued vs what's a lie

Items 1, 9, 14, 15, 16 in the table above were previously marked MISSING/WRONG; they're now done.

What's still queued (and not "lie"-grade — just deferred):
- F-009/F-010 write mode + fill pattern (audit items 6, 7, 13)
- F-022 volume display-half + battery status (audit items 25, 26)
- F-023/F-024/F-025/F-026/F-027 copy/delete (audit items 19-23)
- F-037 factory reset (audit item 24)
- F-038 active sounds/patterns display (audit item 27)
- TFT surfacing of the tweak mode label (v2 display work; the
  state is correctly tracked in `s_tweak_mode` but `display.c`
  doesn't render it).
- TFT surfacing of the volume level on BPM tap (the PO-33 lights
  up step LEDs to show the current level; display.c doesn't do
  this yet).
- PO-33's dedicated tweak-target buttons (S+1, S+2, etc.) which
  set which *slot* is being tweaked. We don't have those; tweak
  mode affects whichever slot just played. PO-33-grade behaviour
  would queue for v2.

## Recommended fixes (in priority order, after this commit)

1. **BPM tap = volume level display** (audit item 26) — render the current `s_volume_level` on the matrix LEDs when BPM is tapped alone. Requires `display.c` work.
2. **Multi-modifier combos** (F-023/F-024/F-025/F-026/F-027) — would require a multi-modifier framework entry.
3. **Audit `po33_fx_t` enum ordering** vs PO-33 manual effects list. Flash-format-breaking; needs its own discussion.
4. **WRITE enter/exit write mode** (F-009) — biggest remaining piece; needed for F-010, F-013's "save in pattern", F-023, F-025.
5. **Tweak-mode label on TFT** — render `s_tweak_mode` in `display.c` so the user can see which mode is active.
6. **Add `step_t.resonance` field** — currently knob B in Filter mode is logged but not stored (no per-step resonance data).