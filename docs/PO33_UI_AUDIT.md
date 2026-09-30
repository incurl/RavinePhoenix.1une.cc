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
| 8 | [select pattern] press play (>) → play pattern                         | (F-022 documents PLAY = tap toggle)                          | PLAY tap → sequencer_play/stop toggle    | ✅ DONE (matches PO-33) |
| 9 | hold pattern (⠛) + number(s) → change patterns (chain)                | F-013: chain build (partial); F-014: reorder (missing)        | pattern_on_step replaces, not appends    | ⚠️ WRONG semantics (we replace; PO-33 appends) |
|10 | press fx (FX) → toggle tweak parameter (Tone/Filter/Trim)             | F-015: ❌ missing; "We'd need a separate Tweak button" (WRONG) | BTN_FX tap: no-op                       | ❌ MISSING + DOC LIE (FX IS the tweak button!) |
|11 | [select tweak] turn knob A/B → tweak parameter                          | F-016/F-017/F-018: knobs wire, binding pending                | knobs read but not bound                  | ❌ MISSING |
|12 | [play pattern] hold fx + number (1-15) → add effect                    | F-019: FX + step 1-15 sets active_fx (carried to next note)  | FX + step 1-15 → sequencer_set_active_fx | ✅ done (b4fce86); but **does NOT save into pattern** — this is the "add effect" semantics, not "add+save in pattern" |
|13 | [write mode] [play] hold fx + number (1-15) → add+save effect in pattern | F-019: "save in pattern queued"                              | not implemented                           | ❌ MISSING (depends on write mode, #6) |
|14 | [write mode] [play] hold fx + 16 → clear effect in pattern             | (not in our docs at all)                                     | step 16 logs "swing stub"                | ❌ WRONG (we invented "FX+16=swing"; PO-33 says FX+16=clear-effect; swing is BPM+knob A!) |
|15 | hold bpm (handle) + turn knob A → change swing                         | F-020/F-021 + HARDWARE.md §4.5b: BPM long-press + Knob A = fine tempo | BPM long-press + Knob A = fine tempo | ❌ WRONG (this should be SWING, not fine tempo!) |
|16 | hold bpm (handle) + turn knob B → change tempo (fine tuned)            | F-020/F-021: ❌ missing                                       | Knob B never read                          | ❌ MISSING |
|17 | press bpm (handle) → change tempo (pre-defined modes)                  | F-020/F-021: tap cycles 80/120/140 (Hip Hop/Disco/Techno)   | BPM tap → sequencer_cycle_bpm_preset     | ✅ DONE |
|18 | click bpm + number → change volume (max 5)                              | F-022: ❌ missing                                              | not bound                                 | ❌ MISSING |
|19 | [select sound] hold write + sound + number → copy sound                 | F-023: ❌ missing                                              | no WRITE handler                          | ❌ MISSING (multi-modifier combo; not supported by framework) |
|20 | hold write + sound + 9-16 + 1-16 → copy slice                          | F-024: ❌ missing                                              | no WRITE handler                          | ❌ MISSING |
|21 | [select pattern] hold write + pattern + number → copy pattern           | F-025: ❌ missing                                              | no WRITE handler                          | ❌ MISSING |
|22 | [select sound] hold record + sound → delete sound                       | F-026: ❌ missing                                              | no REC handler                            | ❌ MISSING |
|23 | [select pattern] hold record + pattern → delete pattern                 | F-027: ❌ missing                                              | no REC handler                            | ❌ MISSING |
|24 | (Power off) hold pattern + insert batteries → factory reset            | F-037: storage_erase_all (not yet written)                   | no UI                                     | ❌ MISSING |
|25 | press sound + bpm → battery status                                       | (not in docs)                                                | no UI                                     | ❌ MISSING |
|26 | press bpm → volume level (numbers lit until current)                    | F-022: queued                                                | no UI                                     | ❌ MISSING (PO-33 says BPM tap = BOTH volume level AND tempo cycle! Conflict with #17!) |
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
| PLAY tap → play/stop toggle | Matches PO-33 "press play (>)" |
| BPM tap → cycle tempo presets (Hip Hop 80 / Disco 120 / Techno 140) | Matches PO-33 "press bpm (handle) to toggle between different levels" |
| PATTERN held + step → select pattern | Matches PO-33 "hold pattern (⠛) + number" |
| SOUND held + step → select slot | Matches PO-33 "hold sound (S) + number" |
| Step press without modifier after SOUND select → plays once | Matches PO-33 "[select sound] press number" |

## What's queued vs what's a lie

The docs flag many items as "queued for v2". The issue is that *some* of those queued items have *wrong* specifics committed. Specifically:
- BPM-Knob bindings: committed with WRONG mapping (swing vs fine tempo swapped)
- FX+16 binding: committed with WRONG mapping (swing stub invented; should be clear-effect, behind write mode)
- FX tap tweak-mode cycle: docs say "needs separate Tweak button"; correct is "FX tap cycles tweak"

## Recommended fixes (in priority order)

1. **Swap BPM + knob bindings.** Knob A = swing (8 discrete levels? or continuous?). Knob B = fine tempo. This is a 4-line code change (rename `knob_a_to_bpm` → `knob_a_to_swing`, add `knob_b_to_bpm`; update `s_bpm_held` polling to read both knobs).
2. **Remove the FX+16 swing stub. Replace with `PO33_FX_NONE` (entry 16 = "no effect" in PO-33's list).** Also fix the docs to say "FX+16 = clear effect in pattern" (behind write mode), not "swing".
3. **F-015 docs: "press FX to toggle tweak mode"** — same button as punch-in FX. Two roles, no separate button needed.
4. **Audit `po33_fx_t` ordering** against PO-33 manual list; add `PO33_FX_SCRATCH` (PO-33's entry 11, missing from our enum).
5. **F-001 status flip to ⚠️ partial** (REC has no binding).
6. **F-013 status: replace vs append semantics** — our `pattern_on_step` replaces; PO-33 chain-build appends.

