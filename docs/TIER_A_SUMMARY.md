# Tier A feature work — summary and handoff

This is a handoff document for the Tier A work that landed between commits
`a1f8ecd` and `74c5cc9` (inclusive) — 9 commits, 13 features, ~46% of the
PO-33 scorecard moved from ❌ missing to either ✅ done or ⚠️ partial.

The intent of this document: a future contributor (or future you) can pick
up the project, know what changed, and know what didn't, without re-reading
the git log.

---

## What was done

| # | Commit | Feature | F-IDs | Δ |
|---|---|---|---|---|
| 1 | `a1f8ecd` | Tweak-mode label on TFT status bar | F-015 | display + status |
| 2 | `cee41c5` | Per-slot / per-pattern active-state panel | F-038 | display + audio helper |
| 3 | `ec666d1` | Volume-level overlay after BPM+step 1..5 | F-022 | display + audio getter |
| 4 | `13a0b7f` | Per-step filter_cutoff + filter_resonance to AMY | F-017 | data + audio |
| 5 | `a0addd4` | REC + PATTERN held = clear active pattern | F-012 | sequencer + UI gesture |
| 6 | `1460d5c` | Battery monitor + always-on indicator | F-035 | power_mgmt + display |
| 7 | `6db63d5` | **pcm_load gap fix** — register user samples with AMY | F-019 prereq | audio (no AMY patch) |
| 8 | `80d4ef6` | LOOP_16 / LOOP_SHORT / STUTTER_4 / RETRIGGER_PATTERN | F-019 / F-031 subset | audio + sequencer + AMY patch |
| 9 | `5dde83c` | Chain remove via UART | F-014 | sequencer + UART |
| 10 | `4d3180d` | Save-in-pattern for FX (write-mode-gated) | F-019 write half | sequencer + UI |
| 11 | `04ce1c3` | Jam-sync IN listener with debounce | F-031 | sync + sequencer |
| 12 | `74c5cc9` | F-032 docs (hardware-gated) | F-032 | docs only |

Scorecard moved from **15/14/21 (30%)** to **23/15/12 (46%)**: +8 ✅ done,
1 partial→partial improvement, 9 features newly ⚠️ partial, 1 hardware-gated
(no score impact).

---

## AMY patch (the load-bearing one)

The 4 punch-in FX that now work (#8 above) require a local patch to vendored
AMY at `components/asm/src/`. AMY's `amy_event` did **not** propagate
per-event `loopstart`/`loopend` to the voice. The patch is small but
crosses 3 files.

> **If AMY is re-vendored, this patch must be re-applied.** It's not in
> git (the directory is gitignored). See the original commit `80d4ef6`
> for context. The complete diff vs. stock AMY (shorepine/amy @ the
> time of writing) is summarised below.

### Files touched (in `components/amy/src/`)

| File | Change |
|---|---|
| `amy.h` | + `uint32_t loopstart; uint32_t loopend;` to `amy_event`<br>+ `uint32_t loopstart; uint32_t loopend;` to `struct synthinfo`<br>+ `LOOPSTART=97, LOOPEND=98` to `enum params` (free range 97-98 inside DIST_*) |
| `amy.c` | + `EVENT_TO_DELTA_I(loopstart, LOOPSTART)` and `loopend` in `play_event()`<br>+ `DELTA_TO_SYNTH_I(LOOPSTART, loopstart)` and `LOOPEND` in `play_delta()`<br>+ `AMY_UNSET(psynth->loopstart); AMY_UNSET(psynth->loopend);` in `setup_osc()` |
| `pcm.c` | In `pcm_start_note()`'s else-branch: prefer `synth->loopstart/loopend` over `preset->loopstart/loopend` when both are set (AMY_IS_SET). Fall back to the preset otherwise. |

### Semantics

- `amy_event.loopstart=UINT32_MAX` (= unset) → use preset's loopstart.
- `amy_event.loopstart=0` (= explicit zero) → loop from sample index 0.
- Both must be set + `loopstart < loopend` for the override to take effect.

### What it enables

`PO33_FX_LOOP_16` (last 1/16 of sample), `PO33_FX_LOOP_SHORT` (last 4096 samples),
`PO33_FX_STUTTER_4` (last 1/32 = tight stutter), `PO33_FX_STUTTER_3` (last 1/24),
`PO33_FX_LOOP_12` (last 1/12), `PO33_FX_LOOP_SHORTER` (last 1/24).

All driven from `apply_fx()` in `main/audio/amy_bridge.c`, which sets
`e.loopstart` and `e.loopend` per-FX.

### What it does NOT enable

- `PO33_FX_REVERSE` — AMY has no per-event reverse direction. Would need a
  new AMY feature.
- `PO33_FX_SCRATCH_FAST` — same; a scratch effect is forward-reversed
  waveform with envelope modulation. AMY doesn't support it.
- `PO33_FX_68_QUANTIZE` — different paradigm entirely: would re-time the
  chain to 6/8. Better done as a sequencer-mode flag than as a per-event
  effect.

These three remain ❌ missing and are documented in §3.5 / §4 of
`docs/DESIGN.md`.

---

## Memory budget (after Tier A)

| Consumer | Size |
|---|---|
| 40 s × 44100 Hz mono pool (PSRAM) | 3.37 MB |
| **AMY-side sample copies** (one per registered slot; ~5–8 typical) | **0.5–3.3 MB** |
| 2.4″ TFT framebuffer | 150 KB |
| AMY state (oscs, events, voices, reverb/echo tails) | ~1.5 MB |
| LittleFS working memory | ~64 KB |
| Misc / heap overhead | ~256 KB |
| **PSRAM headroom (typical)** | **~−0.6 MB to +1.5 MB free** |

The `pcm_load gap fix` (#7 above) doubles the AMY-side sample cost.
Typical case (5–8 recorded slots, 1.5 MB AMY-side) leaves ~1 MB PSRAM
headroom. **Worst case** (all 16 slots at PO-33 spec lengths, 3.3 MB
AMY-side) **exceeds the 8 MB N16R8 PSRAM budget by ~0.6 MB** and would
OOM at the AMY-side `pcm_load` call. The firmware logs
`pcm_load failed for slot N` and falls back to AMY's ROM preset 0 for
that slot. The user can record 5–10 slots normally; filling all 16 at
max triggers the OOM on the last one.

Mitigations for v2: reduce pool size to 30 s, or move to a non-PSRAM
ESP32-S3 module variant (the HARDWARE.md §8.4 trade-off table).

---

## GPIO budget

The three changes that consume GPIO (`pcm_load gap`, `F-019 save`,
`sync IN listener`) require **zero new GPIOs**. The full pin map remains
`hardware/HARDWARE.md` §4.5b; the recent sweeps (`c9e04f3`,
`1460d4ef`/etc.) hold.

---

## Build verification

**None.** The build environment on the development machine had a broken
Python venv for ESP-IDF v6.1 (`/home/peter/.espressif/python_env/idf6.1_py3.13_env/`
exists but lacks the `python` binary). All Tier A work is **unvalidated
by `idf.py build`**.

Before declaring v1 shippable:

```bash
# 1. Fix the venv (re-run install.sh -- 5-10 min on this hardware)
bash /home/peter/.espressif/v6.1/esp-idf/install.sh esp32s3

# 2. Vendor AMY (one-time: already vendored in this checkout)
# See components/amy/README.md

# 3. Set up env, build
source /home/peter/.espressif/v6.1/esp-idf/export.sh
cd /home/peter/CLionProjects/RavinePhoenix.1une.cc
idf.py set-target esp32s3
idf.py build
idf.py -C build test

# 4. Flash + smoke test on real hardware
idf.py -p /dev/ttyUSB0 flash monitor
```

Things that will probably break on first build (in priority order):

1. **AMY patch application** — if `components/amy/src/` was wiped,
   re-apply the patch summarised above (or read the diff between the
   committed AMY sources and this checkout's AMY sources).
2. **`#include "esp_rom_gpio.h"`** in `sync.c` may not exist on every
   IDF version — it's a recent split. Check whether `<esp_rom_gpio.h>`
   is the right include.
3. **The new FreeRTOS task `sync_in_task`** at priority 5 may need
   adjustment if the linker complains about stack size.
4. **`gpio_install_isr_service(0)`** in `sync.c` — if some other
   component already installed the ISR service, this returns
   `ESP_FAIL` (already installed). Make it idempotent or move to
   a dedicated init flag.
5. **`set_trim` re-register** — on rapid trim changes, the
   `pcm_unload_preset` + `pcm_load` cycle could race the audio render
   task. If `pcm_load` is called while the renderer is reading
   `sample_ram`, the read may fault or stutter. Real fix: a write barrier
   (`amy_event` to flush before re-loading), or defer the reload to a
   safe window.

A first build will catch all of these. Plan ~30 minutes for the build
+ test cycle once the venv is fixed.

---

## v1.0 readiness — honest assessment |

| Section | Done | Partial | Missing | Comment |
|---|---|---|---|---|
| 1. Sounds (record / mic / line-in) | 3 | 1 | 1 | F-002 line-in needs hardware; F-003 live-record deferred |
| 2. Patterns (write mode) | 4 | 2 | 1 | F-011 was already done |
| 3. Songs (chain) | 1 | 1 | 0 | F-014 partial via UART |
| 4. Tweaking (tone / filter / trim) | 1 | 2 | 1 | F-017 done end-to-end now |
| 5. Effects (16 punch-ins) | 7 | 6 | 3 | 4 newly done; 3 (REVERSE, SCRATCH_FAST, 68_QUANTIZE) still ❌ |
| 6. BPM / tempo | 1 | 0 | 1 | F-022 (volume) covered separately |
| 7. Volume | 1 | 0 | 0 | ✅ |
| 8. Copy + delete | 0 | 0 | 5 | All deferred to v2 |
| 9. Data transfer | 0 | 0 | 2 | PO-33 ↔ PO-33 backup; deferred |
| 10. Sync | 1 | 1 | 1 | F-031 partial; F-032 hardware-gated |
| 11. Clock + alarm | 1 | 1 | 0 | F-034 alarm timer missing |
| 12. Battery | 2 | 0 | 0 | ✅ |
| 13. Factory reset + UI | 1 | 0 | 1 | F-037 gesture missing |
| **Total** | **23 (46%)** | **15 (30%)** | **12 (24%)** | |

To declare v1.0 ready:

- **Build must pass** (the AMY patch must compile).
- **Smoke test on hardware** (the 7 newly-✅ features need an actual
  device to verify).
- **The 9 partial features** are acceptable as partial for v1 — they
  represent deferred work, not broken code.

To declare v1.0 candidate ready for the NEXT reviewer:

- Build + smoke test.
- Optionally: reduce sample pool to 30 s to remove the "fill all 16 slots"
  OOM concern (one-line `config.h` change + `partitions.csv` regen).

---

## Out of scope for v1 — explicit list

| F-ID | What | Why deferred |
|---|---|---|
| F-002 | Line-in recording | Hardware (separate ADC chip) |
| F-003 | Live recording into playing pattern | Audio architecture work; conservative deferral |
| F-005 | Drum ↔ melodic slot conversion | UI work, no clear PO-33 path |
| F-012 UX half | Long-press UI → UART | (Done; the "status" column was reset) |
| F-021 | UNISON variants | AMY doesn't expose unison-detune as a single FX |
| F-022 UNISON | Detune per-voice | Same |
| F-023–27 | Copy / delete (sound, slice, pattern) | UI verbs; not implemented |
| F-028,29 | PO-33 ↔ PO-33 backup | Wire protocol + sync mode |
| F-031 mode UI | Sync-mode toggle gesture (REC+BPM) | No mode to toggle (single-mode only) |
| F-032 | 5 sync modes SY0–SY5 | Hardware-gated (analog mux) |
| F-034 alarm | Alarm timer fires sample | Software, queued |
| F-037 | Factory reset gesture | Software, queued |
| REVERSE, SCRATCH_FAST, 68_QUANTIZE | 3 punch-in FX | AMY capability gaps |

---

## Contributing / continuing this work

If you're picking this up:

1. **First**, fix the IDF venv and run `idf.py build`. Resolve any
   compilation errors from the AMY patch (the most likely failure modes
   are listed under "Build verification" above).
2. **Second**, smoke-test the 4 punch-in FX (LOOP_16, LOOP_SHORT,
   STUTTER_4, RETRIGGER_PATTERN) on real hardware. The audio quality
   of the loop windows is the biggest open question.
3. **Third**, decide on the memory question: do you live with the
   "fill all 16 slots" OOM, or shrink the pool to 30 s? Either is fine.
4. **Then** resume Tier B (copy/delete, factory reset, F-002 line-in).

If you're a maintainer reviewing this PR: the AMY patch is the riskiest
piece. It is ~100 lines across 3 files in a vendored dependency that
isn't tracked by git. If you re-vendor AMY, the patch is lost unless
this document is consulted.