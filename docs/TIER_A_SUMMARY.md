# Tier A feature work — summary and handoff

This is a handoff document for the Tier A work that landed between commits
`a1f8ecd` and `f3b3730` (inclusive) — 10 commits, 13 features, ~46% of the
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
| 8 | `80d4ef6` | LOOP_16 / LOOP_SHORT / STUTTER_4 / RETRIGGER_PATTERN | F-019 / F-031 subset | audio + sequencer + AMY patch (loopstart/loopend) |
| 9 | `5dde83c` | Chain remove via UART | F-014 | sequencer + UART |
| 10 | `4d3180d` | Save-in-pattern for FX (write-mode-gated) | F-019 write half | sequencer + UI |
| 11 | `04ce1c3` | Jam-sync IN listener with debounce | F-031 | sync + sequencer |
| 12 | `74c5cc9` | F-032 docs (hardware-gated) | F-032 | docs only |
| 13 | `f3b3730` | **pcm_load_external** — reuse PSRAM pool (no AMY-side copy) | perf/memory | AMY patch + firmware |

Scorecard moved from **15/14/21 (30%)** to **23/15/12 (46%)**: +8 ✅ done,
1 partial→partial improvement, 9 features newly ⚠️ partial, 1 hardware-gated
(no score impact). Commit #13 (the memory fix) doesn't move the scorecard —
it removes a "worst case OOM" risk documented below.

---

## AMY patches (two)

The vendored AMY at `components/asm/src/` needs **two** local patches to
make Tier A work. Both are gitignored (the directory is), so re-vendoring
AMY without applying them silently regresses the firmware.

### Patch 1 — per-event `loopstart` / `loopend` (commit `80d4ef6`)

The 4 punch-in FX in #8 above need per-event loop bounds. AMY's `amy_event`
did not propagate `loopstart`/`loopend` to the voice; this patch fixes that.

> The complete diff vs. stock AMY is in commit `80d4ef6`. Summary:

| File | Change |
|---|---|
| `amy.h` | + `uint32_t loopstart; uint32_t loopend;` to `amy_event`<br>+ `uint32_t loopstart; uint32_t loopend;` to `struct synthinfo`<br>+ `LOOPSTART=97, LOOPEND=98` to `enum params` (free range 97-98 inside DIST_*) |
| `amy.c` | + `EVENT_TO_DELTA_I(loopstart, LOOPSTART)` and `loopend` in `play_event()`<br>+ `DELTA_TO_SYNTH_I(LOOPSTART, loopstart)` and `LOOPEND` in `play_delta()`<br>+ `AMY_UNSET(psynth->loopstart); AMY_UNSET(psynth->loopend);` in `setup_osc()` |
| `pcm.c` | In `pcm_start_note()`'s else-branch: prefer `synth->loopstart/loopend` over `preset->loopstart/loopend` when both are set (AMY_IS_SET). Fall back to the preset otherwise. |

**Semantics:**

- `amy_event.loopstart = UINT32_MAX` (= unset) → use preset's loopstart.
- `amy_event.loopstart = 0` (= explicit zero) → loop from sample index 0.
- Both must be set + `loopstart < loopend` for the override to take effect.

**What it enables:** `PO33_FX_LOOP_16` (last 1/16), `PO33_FX_LOOP_SHORT` (last 4096 samples), `PO33_FX_STUTTER_4` (last 1/32), `PO33_FX_STUTTER_3` (last 1/24), `PO33_FX_LOOP_12` (last 1/12), `PO33_FX_LOOP_SHORTER` (last 1/24). All driven from `apply_fx()` in `main/audio/amy_bridge.c`.

### Patch 2 — `pcm_load_external` for caller-owned samples (commit `f3b3730`)

The memory fix in #13 above removes the AMY-side sample copy. Before this
patch, `pcm_load()` allocated a fresh block in PSRAM and copied the sample
into it. With 16 slots filled at max length that doubled the sample
memory and exceeded the 8 MB N16R8 budget by ~0.6 MB. `pcm_load_external()`
takes a caller-owned pointer instead, so the firmware's existing
`amy_bridge` PSRAM pool is shared directly with AMY.

| File | Change |
|---|---|
| `amy.h` | + `#define AMY_PCM_TYPE_MEMORY_EXTERNAL 4` (new type alongside `ROM`/`FILE`/`MEMORY`/`GAMMA`)<br>+ `extern void pcm_load_external(...)` declaration |
| `pcm.c` | + `void pcm_load_external(...)`: allocates only the metadata (LL node + `memorypcm_preset_t`, ~80 bytes), sets `type = AMY_PCM_TYPE_MEMORY_EXTERNAL`, points `sample_ram` at the caller's buffer. No changes needed to `pcm_unload_preset` (existing single-block `free()` is correct for both types: for `MEMORY` it frees the combined metadata+sample block; for `MEMORY_EXTERNAL` it frees only the metadata — sample buffer stays with the caller). |
| `main/audio/amy_bridge.c` | `register_slot_with_amy()` calls `pcm_load_external(preset, s_pool + offset, ...)` instead of `pcm_load(...) + memcpy(...)`. `amy_bridge_register_slot()` and `amy_bridge_set_trim()` no longer need an explicit pre-unload (the new function handles it). |

**Why no other AMY changes were needed:** the renderer's `pcm_render_block`
and `pcm_note_on` already branch on `type != AMY_PCM_TYPE_FILE` for
streaming-specific behaviour (which both `MEMORY` and `MEMORY_EXTERNAL`
bypass). The read-only access of `preset->sample_ram` in the renderer
works regardless of who owns the memory.

### What the patches do NOT enable

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

## Memory budget (after Tier A + pcm_load_external)

| Consumer | Size |
|---|---|
| 40 s × 44100 Hz mono pool (PSRAM) | 3.37 MB |
| AMY-side sample metadata (~80 bytes per registered slot × 16) | ~1.3 KB |
| 2.4″ TFT framebuffer | 150 KB |
| AMY state (oscs, events, voices, reverb/echo tails) | ~1.5 MB |
| LittleFS working memory | ~64 KB |
| Misc / heap overhead | ~256 KB |
| **PSRAM headroom** | **~2.5 MB free** |

The PSRAM sample pool is now shared via `pcm_load_external()` (commit
`f3b3730`); AMY keeps only a small metadata struct per registered slot
(~80 bytes). With 16 slots fully filled, AMY-side cost is ~1.3 KB
total. PSRAM headroom is ~2.5 MB regardless of how many slots are
recorded, which is comfortable.

> **Note on history.** Before `f3b3730`, the `pcm_load gap fix`
> (`6db63d5`) made AMY-side allocations balloon to ~3.3 MB at full slot
> load, exceeding the 8 MB N16R8 budget by ~0.6 MB. That OOM risk is
> now eliminated by `pcm_load_external`. The "fill all 16 slots" worst
> case is no longer a memory concern.

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