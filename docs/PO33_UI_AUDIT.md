# PO-33 UI Audit — manual vs codebase

**Authority:** `docs/CONTROL_REFERENCE.md` is the single source of truth for
UI interactions (ADR-0003). This file is the *audit trail*: it records, per
manual interaction, what the code does and where the gaps are.

- **Source of truth:** [lode/PO-33 README](https://raw.githubusercontent.com/lode/PO-33/main/README.md).
- **Regenerated:** 2026-10-06 (the previous version was a 2026-09-30 snapshot
  whose "code behaviour" column was overtaken by the Tier-A work; see the
  resolution table below).
- **Labels:** ✅ matches the manual · ⚠️ sanctioned deviation (ADR-0002 or a
  longer gate) · ➕ Ravine: Phoenix-only gesture · ❌ manual feature not
  implemented.

## Status table (regenerated from current code)

| # | PO-33 manual (verbatim) | Code behaviour | Label |
|---|---|---|---|
| 1 | `record own sound` — hold record (star) + number | `BTN_REC` held + pad → `rec_on_step()`; release → `rec_on_release()` | ✅ |
| 2 | `record own sound (live)` — [play] hold write + number(s) | not implemented | ❌ |
| 3 | `select sound` — hold sound (S) + number | `SOUND` + pad → `sound_on_step()` | ✅ |
| 4 | `play a sound` — [select sound] press number | bare pad → `play_active_slot()` (active slot; pad = scale/slice) | ✅ |
| 5 | `select pattern` — hold pattern (⠛) + number | `PATTERN` + pad → `pattern_on_step()` | ✅ |
| 6 | `enter/exit write mode` — [select pattern] press write (·) | `WRITE` tap → `write_mode_enter/exit()` | ✅ |
| 7 | `fill pattern` — write mode, press sound#, press step#s | write mode + pad → `write_mode_apply_step()` binds active slot to step N | ⚠️ slot chosen before write mode (see CONTROL_REFERENCE note A) |
| 8 | `play pattern` — [select pattern] press play (>) | `PLAY` tap → `sequencer_play/stop()` | ✅ |
| 9 | `change patterns` — hold pattern (⠛) + number(s) | `PATTERN` + pad → set **and append** (`sequencer_chain_append()`) | ✅ |
| 10 | `select tweak parameter` — press fx (FX) | `FX` tap → `tweak_mode_cycle()` | ✅ |
| 11 | `tweak parameter` — [select tweak] turn knob A/B | `tweak_apply_step()` / `tweak_apply_slot()` | ✅ (filter resonance not per-step yet) |
| 12 | `add effect` — [play] hold fx + number (1-15) | `FX` + pad 1–15 → `fx_on_step()` | ✅ |
| 13 | `add & save effect in pattern` — [write][play] hold fx + 1-15 | write mode + `FX` + pad → `sequencer_save_fx_to_pattern()` | ✅ |
| 14 | `clear effect in pattern` — [write][play] hold fx + 16 | `FX` + pad 16 → `PO33_FX_NONE` (+ pattern clear in write mode) | ✅ |
| 15 | `change swing` — hold bpm + turn knob A | `BPM` long-press + Knob A → `sequencer_set_swing()` | ✅ |
| 16 | `change tempo (fine tuned)` — hold bpm + turn knob B | `BPM` long-press + Knob B → `sequencer_set_bpm()` | ✅ |
| 17 | `change tempo (pre-defined modes)` — press bpm | `BPM` tap → `sequencer_cycle_bpm_preset()` | ✅ |
| 18 | `change volume` — hold bpm + number (≤5) | `BPM` long-press + pad 1–5 → `amy_bridge_set_volume_level()` | ✅ |
| 19 | `copy sound` — [select] hold write + sound + number | not implemented | ❌ |
| 20 | `copy slice` — hold write + sound + number + number | not implemented | ❌ |
| 21 | `copy pattern` — [select pattern] hold write + pattern + number | not implemented | ❌ |
| 22 | `delete sound` — [select] hold record + sound | not implemented over buttons (UART `slot_clear` only) | ❌ |
| 23 | `delete pattern` — [select pattern] hold record + pattern | `REC` + `PATTERN` 600 ms → `sequencer_clear_current_pattern()` | ⚠️ longer gate |
| 24 | `factory reset` — hold pattern + insert batteries | `REC` + `BPM` 5 s → `storage_factory_reset()` | ➕ different gesture |
| 25 | `battery status` — press sound + bpm | not implemented | ❌ |
| 26 | `volume level` — press bpm (numbers lit) | `BPM` tap cycles tempo; level display not implemented | ➕ dual role |
| 27 | `active sounds/patterns` — press sound or pattern | not implemented; picker is `WRITE` long-press | ➕ different gesture |

## Bare-button behaviour

| Button | Tap | Long-press | Label |
|---|---|---|---|
| `PLAY` | toggle transport | no-op | ✅ |
| `BPM` | cycle tempo preset | BPM-adjust mode (Knob A = swing, Knob B = fine BPM); **held** + 1–5 = volume | ✅ |
| `PATTERN` | no-op | no-op | ✅ |
| `SOUND` | no-op | no-op | ✅ |
| `FX` | cycle tweak mode | **held 2 s alone → toggle sync IN** | ➕ |
| `REC` | no-op | no-op (start on `REC`+pad) | ✅ |
| `WRITE` | toggle write mode | **enter sketch picker** | ➕ |

## Previously-reported "lies" — resolution status

The 2026-09-30 audit listed six lies. Current status:

| Lie | Then | Now |
|---|---|---|
| #1 BPM + Knob A = swing (not "fine tempo") | code had the knobs swapped | **✅ fixed** — `s_bpm_held` polling: A = swing, B = fine BPM |
| #2 FX + 16 = swing | code had an invented swing stub | **✅ fixed** — `FX` + 16 = `PO33_FX_NONE` (manual entry 16) |
| #3 "FX needs a separate Tweak button" | docs claimed so | **✅ fixed** — docs are correct; `FX` tap is the tweak toggle |
| #4 effects list 1–16 vs ours | ordering/name mismatches vs the manual | **open** — `po33_fx_t` still lacks `SCRATCH` (only `SCRATCH_FAST`) and its order differs; see `TIER_A_SUMMARY.md` |
| #5 F-001 "✅ done" vs no REC binding | docs claimed done; REC was unwired | **✅ fixed** — REC is wired (`rec_on_step`/`rec_on_release`) |
| #6 BPM tap: volume OR tempo? | unimplemented | **open** — tempo half done; volume *display* half still missing (#26) |

## Open gaps (unchanged by this audit)

- Live recording into a playing pattern (#2).
- Copy sound / slice / pattern (#19–#21).
- Delete sound (#22).
- Battery status (#25); volume-level display (#26); active sounds/patterns (#27).
- Per-step filter resonance storage (`DESIGN.md` F-017).
- `po33_fx_t` ordering/`SCRATCH` (Lie #4).

## See also

- `docs/CONTROL_REFERENCE.md` — the authoritative interaction table.
- `docs/architecture-decisions.md` ADR-0003 — the "manual is ground truth" decision.

