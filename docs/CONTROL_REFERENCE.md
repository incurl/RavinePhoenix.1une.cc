# Control Reference — PO-33 manual vs Ravine: Phoenix firmware

This is the **single source of truth** for how the modifier buttons and the
16 step pads behave. Every other document (`DESIGN.md`, `HARDWARE.md`,
`MAKERS_COMPANION.md`, `Ravine_Phoenix_Music_Course.md`) and the project
website should *cite this file* rather than restate a gesture, so the four
surfaces cannot drift apart again.

## Ground truth

1. **The PO-33 operator manual is the specification for every UI
   interaction.** The canonical mirror is
   [lode/PO-33 README](https://raw.githubusercontent.com/lode/PO-33/main/README.md).
   Where a doc or the firmware disagrees with the manual, the *manual wins*.
2. **The only sanctioned deviations are the two ADRs**
   (see `docs/architecture-decisions.md`):
   - **ADR-0001** — website templating (not a UI interaction; out of scope here).
   - **ADR-0002** — the firmware swaps the melodic/drum slot ranges relative
     to the manual (manual: melodic 1–8, drum 9–16; firmware: **drum 1–8,
     melodic 9–16**). This is intentional and is the *only* UI deviation.
3. **The firmware is not modified by documentation work.** Where the firmware
   adds a gesture the manual does not have, or omits a manual feature, this
   file records it honestly with a label. Do not "fix" the docs to hide a
   difference.

### Labels used throughout

| Label | Meaning |
|---|---|
| ✅ | Firmware matches the manual. |
| ⚠️ | Sanctioned deviation (ADR-0002 slot ranges only). |
| ➕ | Ravine: Phoenix-only gesture — **not in the PO-33 manual**. |
| ❌ | Manual feature the firmware does not implement (yet). |

## The master table

Manual wording in the first column is quoted from the lode/PO-33 README.
"Firmware" cites the implementing function in `main/ui/input.c` or
`main/sequencer/sequencer.c`.

| # | PO-33 manual (verbatim) | Firmware behaviour | Label |
|---|---|---|---|
| 1 | `record own sound` — **hold record (star) + number**, make sound, release buttons | `BTN_REC` held + pad N → `rec_on_step()` records into slot N; releasing `BTN_REC` → `rec_on_release()` stops | ✅ |
| 2 | `record own sound (live)` — `[play pattern]` **hold write (·) + number(s)** | not implemented | ❌ |
| 3 | `select sound` — **hold sound (S) + number** | `SOUND` held + pad N → `sound_on_step()` sets active slot N−1 | ✅ |
| 4 | `play a sound` — `[select sound]` **press number** | pad press with no modifier → `play_active_slot()` plays the **active** slot (pad index → scale degree / slice) | ✅ |
| 5 | `select pattern` — **hold pattern (⠛) + number** | `PATTERN` held + pad N → `pattern_on_step()` sets pattern N−1 | ✅ |
| 6 | `enter/exit write mode` — `[select pattern]` **press write (·)** | `WRITE` tap → `write_mode_enter()` / `write_mode_exit()` | ✅ |
| 7 | `fill pattern` — `[enter/exit write mode]` press number of sound to add/remove, then press numbers of steps where to add (press again to remove) | write mode + pad N → `write_mode_apply_step()` binds the **active slot** to **step N** (tap = toggle, long-press = clear) | ⚠️ slot is chosen with `SOUND + number` *before* write mode; see note A |
| 8 | `play pattern` — `[select pattern]` **press play (>)** | `PLAY` tap → `sequencer_play()` / `sequencer_stop()` | ✅ |
| 9 | `change patterns` — **hold pattern (⠛) + number(s)** (repeats allowed) | `PATTERN` held + pad N → `pattern_on_step()` selects **and** `sequencer_chain_append()`s | ✅ |
| 10 | `select tweak parameter` — **press fx (FX)** to toggle | `FX` tap → `tweak_mode_cycle()` (Tone → Filter → Trim → Tone) | ✅ |
| 11 | `tweak parameter` — `[select tweak parameter]` **turn knob A/B** | `tweak_apply_step()` / `tweak_apply_slot()` | ✅ (filter resonance is not stored per-step yet — see note B) |
| 12 | `add effect` — `[play pattern]` **hold fx (FX) + number (1-15)** | `FX` held + pad 1–15 → `fx_on_step()` sets active FX | ✅ |
| 13 | `add & save effect in pattern` — `[enter write mode]` `[play pattern]` **hold fx (FX) + number (1-15)** | write mode active + `FX` held + pad 1–15 → also `sequencer_save_fx_to_pattern()` | ✅ |
| 14 | `clear effect in pattern` — `[enter write mode]` `[play pattern]` **hold fx (FX) + 16** | `FX` held + pad 16 → `PO33_FX_NONE`; in write mode also clears the pattern | ✅ |
| 15 | `change swing` — **hold bpm (handle) + turn knob A** | `BPM` long-press held + Knob A → `sequencer_set_swing()` (8 levels) | ✅ |
| 16 | `change tempo (fine tuned)` — **hold bpm (handle) + turn knob B** | `BPM` long-press held + Knob B → `sequencer_set_bpm()` (60–240) | ✅ |
| 17 | `change tempo (pre-defined modes)` — **press bpm (handle)** to toggle | `BPM` tap → `sequencer_cycle_bpm_preset()` (80/120/140) | ✅ |
| 18 | `change volume` — **hold bpm (handle) + number** (max 5) | `BPM` long-press held + pad 1–5 → `amy_bridge_set_volume_level()` | ✅ |
| 19 | `copy sound` — `[select sound]` **hold write (·) + sound (S) + number** | not implemented | ❌ |
| 20 | `copy slice` — **hold write (·) + sound (S) + number (9-16) + number (1-16)** | not implemented | ❌ |
| 21 | `copy pattern` — `[select pattern]` **hold write (·) + pattern (⠛) + number** | not implemented | ❌ |
| 22 | `delete sound` — `[select sound]` **hold record (star) + sound (S)** | not implemented over the buttons (UART `slot_clear` only) | ❌ |
| 23 | `delete pattern` — `[select pattern]` **hold record (star) + pattern (⠛)** | approximated: `REC` + `PATTERN` held 600 ms → `sequencer_clear_current_pattern()` | ⚠️ gesture differs — see note C |
| 24 | `factory reset` — **hold pattern (⠛) + insert batteries** | `REC` + `BPM` held 5 s → `storage_factory_reset()` | ➕ different gesture |
| 25 | `battery status` — **press sound (S) + bpm (handle)** | not implemented | ❌ |
| 26 | `volume level` — **press bpm (handle)** (numbers lit until the current level) | `BPM` tap cycles tempo instead; the level is set (not shown) by `BPM + 1-5` | ➕ dual role differs — see note D |
| 27 | `active sounds/patterns` — **press sound (S) or pattern (⠛)** | not implemented; the sketch picker lives on `WRITE` long-press instead | ➕ different gesture |

## Bare-button behaviour (not a hold-modifier combo)

| Button | Tap | Long-press | Label |
|---|---|---|---|
| `PLAY` | toggle transport | no-op | ✅ (manual defines only "press play") |
| `BPM` | cycle tempo preset | enter BPM-adjust mode (Knob A = swing, Knob B = fine BPM); **held** + pad 1–5 = volume | ✅ |
| `PATTERN` | no-op | no-op | ✅ (manual has only "hold + number") |
| `SOUND` | no-op | no-op | ✅ |
| `FX` | cycle tweak mode | **held alone 2 s → toggle sync IN** | ➕ extension |
| `REC` | no-op | no-op (recording starts on `REC`+pad) | ✅ |
| `WRITE` | toggle write mode | **enter sketch picker** | ➕ extension |

## Dispatch precedence (a step-pad press, in order)

1. `BPM` held → pad 1–5 = volume level; pad 6–16 = no-op.
2. Held modifier `REC` → record into that slot (first press wins).
3. Press modifiers in fixed order — `PATTERN` → `SOUND` → `FX` — first held wins.
4. Sketch picker active → pad = jump + load.
5. Write mode active → pad = toggle/clear the bind at **step N**.
6. Otherwise → `play_active_slot()` (plays the active slot; pad index drives pitch/slice).

## Ravine: Phoenix-only extensions (➕ — not in the manual)

These gestures exist in the firmware but have no PO-33 manual counterpart.
They are documented so the operator is not surprised, and so a future
contributor does not mistake them for manual behaviour.

- `WRITE` **long-press** → sketch picker (the manual's "active sounds/patterns" is `SOUND`/`PATTERN`, #27).
- `FX` **held 2 s alone** → toggle sync IN (the manual does not define a sync-in gesture on the buttons).
- `REC` + `PATTERN` (600 ms) → clear the active pattern.
- `REC` + `BPM` (5 s) → factory reset (the manual's factory reset is "hold `PATTERN` + insert batteries", #24).

## Manual features not yet implemented (❌)

Recorded here so the gap is explicit and not papered over:

- Live recording into a playing pattern (#2).
- Copy sound / copy slice / copy pattern (#19, #20, #21).
- Delete sound (#22); delete pattern is approximated (#23).
- Battery status (#25).
- Volume-level display on `BPM` tap (#26).
- Active sounds/patterns view on `SOUND`/`PATTERN` (#27).

## Note A — why the pads are *not* "pad N = slot N"

The manual defines two different verbs for the pads:

- `select sound` — **hold SOUND + number** (picks *which* sound).
- `play a sound` — **`[select sound]` press number** (trigger it).

Combined with the manual's definitions — *"melodic plays whole sound on a
scale"* and *"drum plays slice of a sound"* — the 16 pads are a **scale /
slicer for the currently selected sound**, not a direct 1:1 map to the 16
slots. So after `SOUND + pad 5`, pressing pad 1 plays the *selected* sound
at scale degree 1 (melodic) or slice 1 (drum) — not "slot 1".

This is exactly what `play_active_slot()` implements: the *active slot* is
whatever `SOUND + number` last selected, and the pad index (0–15) is fed to
`amy_bridge_auto_note_for_step()` (melodic) or `amy_bridge_auto_slice_for_step()`
(drum). Any text that says "pad 1 plays slot 1" without the intervening
`SOUND + 1` select is wrong.

## Note B — knob A / knob B (tweak parameters, verbatim from the manual)

| Parameter | Knob A | Knob B |
|---|---|---|
| tone (`ton`) | pitch | volume |
| filter (`Flt`) | high-/low-pass filter | resonance |
| trim (`tri`) | start point | length |

Firmware status: Tone and Trim match. Filter's Knob B (resonance) is read but
not stored as a per-step field yet (see `DESIGN.md` F-017).

## Note C — delete pattern gesture

The manual uses "hold REC + PATTERN". The firmware uses "hold REC + PATTERN
for 600 ms" (a deliberate guard against accidental clears). Same intent,
longer gate.

## Note D — BPM tap is dual-purpose on the real PO-33

The manual uses `BPM` tap for **both** "change tempo (pre-defined modes)"
(#17) and "volume level" display (#26), distinguished on hardware by state.
The firmware implements the tempo half; the volume-level *display* half is
not implemented. See `DESIGN.md` F-020/F-022.

## Note E — slot ranges (ADR-0002)

- **Manual:** melodic 1–8, drum 9–16.
- **Firmware:** drum 1–8, melodic 9–16 (reversed) — `SLOT_DRUM_COUNT = 8` in
  `main/config.h`, checked in `main/audio/amy_bridge.c`.

This reversal is intentional (ADR-0002). Documents that describe the *device*
should state the firmware order; documents that describe the *PO-33 concept*
may keep the manual order, but must carry an "on Ravine: Phoenix the ranges are
swapped" note. See `docs/architecture-decisions.md` ADR-0002.

## Maintenance

If a gesture changes in `main/ui/input.c`, update **this file first**, then
propagate to `DESIGN.md`, `HARDWARE.md`, the two books, and the website.
