# Architecture Decision Records

This document captures the significant architectural decisions made in the
Ravine: Phoenix project. Each entry follows the lightweight
**ADR (Architecture Decision Record)** format proposed by Michael Nygard:

- **Status** — `Accepted`, `Superseded`, or `Deprecated`.
- **Context** — the forces at play (constraints, options, trade-offs).
- **Decision** — the choice we made.
- **Consequences** — what becomes easier or harder because of this choice.

ADRs are immutable once accepted. If a decision is reversed, the original ADR
is marked `Superseded` and a new one references it.

The aim is short, skimmable records that a new contributor can read in five
minutes and understand *why the project looks the way it does*. We do not
record every micro-decision — only those that shape the project.

---

## ADR-0001 — Use Eleventy (11ty) for the static website

- **Status:** Accepted
- **Date:** 2026-09-25 (initial commit `7f02722`), documented 2026-10-05
- **Deciders:** Peter (initial), validated by linecount on follow-up work

### Context

The project ships firmware for an ESP32-S3 board that emulates the
Teenage Engineering PO-33 K.O!. We needed a website at
`ravine.1une.cc` that would:

1. Host build instructions, hardware pinouts, a UART shell reference,
   and the project's two long-form companion books.
2. Embed a **browser firmware flasher** (ESP Web Tools) so users can
   install the firmware without a local toolchain.
3. Deploy automatically via GitHub Pages on every push to `main`.
4. Have minimal moving parts — the firmware project itself is the
   primary deliverable, not the website.

The website is **small** (~50 pages, mostly static), **serverless** (no
runtime, only `actions/upload-pages-artifact`), and **content-heavy** (lots
of long-form prose, no client-side interactivity beyond a small flasher
script and a reader UI).

### Alternatives considered (briefly)

| Option | Why not |
|---|---|
| **Hugo** | Fastest raw build is irrelevant at this scale; Go templates are less ergonomic for inline prose than Nunjucks. |
| **Jekyll** | GitHub-Pages-default, but Ruby toolchain adds friction on a Node-heavy project. |
| **Next.js / Astro / SvelteKit** | Heavy: client bundles, hydration, framework lock-in. None of the pages need a SPA. |
| **Astro** | Closest runner-up. Could support islands for the future reader, but adds a bundler. Overkill today. |
| **Plain HTML + bash** | No layout / includes / pagination support — we'd reinvent 11ty badly. |
| **Hand-rolled Markdown → HTML** | Already reinventing 11ty. |

### Decision

Use **Eleventy (11ty) v2** with **Nunjucks** as the templating engine,
**Tailwind CSS** for styling, and **markdown-it** for the long-form books
(added in `013a087`).

The configuration is intentionally minimal:

```js
// website/.eleventy.js — ~21 lines before the reader app, ~150 after
templateFormats: ["njk", "html", "md"],
htmlTemplateEngine: "njk",
markdownTemplateEngine: "njk",
```

### Rationale

1. **Eleventy's default is sensible.** Out of the box it supports `.njk`,
   `.html`, and `.md` with one templating engine, layout chaining via
   frontmatter, pagination, and collections. The project's first config
   file was 21 lines.
2. **Nunjucks is familiar.** Jinja2-style syntax (`{% for %}`,
   `{% include %}`, `| filters`) is the lowest-friction option for anyone
   who has touched Python or PHP web frameworks.
3. **No SPA required.** The browser flasher is a `<script type="module">`
   tag; the reader is 80 KB of vanilla JS. We do not need client routing,
   hydration, or SSR.
4. **GitHub Pages friendly.** `npm ci && npm run build` produces
   `_site/`; `actions/upload-pages-artifact@v3` does the rest. No
   bundler, no serverless functions.
5. **Tailwind coexists without friction.** Tailwind's standalone CLI
   watches `src/assets/css/main.css` independently. They do not fight
   each other.
6. **The book's content shape fits 11ty's pagination.** Each chapter is
   a permalink. The reader app's chapter splitter (in `.eleventy.js`)
   reads `docs/*.md` at build time, splits on `## Chapter` /
   `## Appendix` / `# Introduction` headings, and emits one URL per
   chapter via `pagination:` frontmatter.

### Consequences

**Easier:**
- Adding a new page = dropping a `.njk` file in `src/` with
  `layout: base.njk` frontmatter. No router config, no plugin.
- Long-form docs and nav pages share one toolchain.
- Build is fast (~0.3 s for ~50 pages including the reader). Hot reload
  via `eleventy --serve --port=8080`.
- The reader's `book_pref`, `book_next`, `book_all_chapters` data flow
  was added in ~50 lines of `.eleventy.js` (the `annotate()` helper).

**Harder:**
- No built-in image optimization. If we ship a lot of large images,
  we'd add `@11ty/eleventy-img` (or migrate).
- Nunjucks layouts don't see template-local `{% set %}` variables —
  data has to be on the page data chain (frontmatter, collection
  items, or computed-then-attached-to-item). This bit us once in the
  reader work; we solved it by pre-computing all sidebar / prev / next
  data in `.eleventy.js` and attaching it to the paginated item.
- No MDX. If we want React components inside Markdown later, we'd need
  to evaluate Astro.

### Reversibility

Migration to **Astro** is realistic (~1–2 days of work to rewrite
`.eleventy.js` as `astro.config.mjs`, port `.njk` files to `.astro`
components, and replace the Tailwind CLI with the Astro Tailwind
integration). Migration to a SPA (Next, SvelteKit) would be a larger
rewrite because of the SSR-vs-static distinction.

We will revisit this ADR if any of the following become true:

- The site grows past ~500 pages and `eleventy --serve` becomes slow.
- A page needs true client-side state (a sequencer editor, an audio
  recorder, etc.).
- We need MDX or React-in-Markdown.
- We need built-in image optimization.

### See also

- `website/README.md` — build + dev commands.
- `website/.eleventy.js` — the actual config (~150 lines including the
  reader chapter splitter added in `013a087`).
- Commit `7f02722` — the original site commit (Sep 25 2026).
- Commit `013a087` — the reader app, where the templating engine
  proved it could handle the long-form books without modification.
---

## ADR-0002 — Drum and melodic slot ranges are reversed vs. the PO-33

- **Status:** Accepted (as an intentional misalignment)
- **Date:** Documented 2026-10-05; misalignment present since the
  initial firmware commit
- **Deciders:** Peter (firmware side), documented retroactively after
  the Music Course chapter 2 review surfaced the question.

### Context

The PO-33 K.O! (the hardware Teenage Engineering device we emulate)
assigns its 16 sample slots as follows:

- **Slots 1–8:** melodic — auto-mapped to a chromatic scale one
  octave wide (pad 1 = C4, pad 16 = D#5).
- **Slots 9–16:** drum — auto-sliced into 16 equal pieces, one per
  pad.

The Ravine: Phoenix firmware **reverses** this assignment:

- **Slots 1–8:** drum (auto-slicing via F-024).
- **Slots 9–16:** melodic (auto-mapping via F-005).

This is visible in `main/config.h` (`SLOT_DRUM_COUNT = 8`), in
`main/audio/amy_bridge.c` (the `slot < SLOT_DRUM_COUNT` checks in
`amy_bridge_auto_note_for_step` and `amy_bridge_auto_slice_for_step`),
and in the test suite (`main/tests/test_amy_bridge.c:49-53`).

The Music Course book (`docs/Ravine_Phoenix_Music_Course.md`) describes
the **PO-33** convention in chapter 2 — drum = 9–16, melodic = 1–8 —
because the book is written as a tutorial that assumes the user is
coming from a PO-33 background. The Makers' Companion
(`docs/MAKERS_COMPANION.md`) describes the **Ravine: Phoenix** convention
— drum = 1–8, melodic = 9–16 — because it is firmware-facing. The
two books are inconsistent with each other, and the firmware is
inconsistent with the Music Course.

### Decision

**Keep the misalignment.** The slot range reversal between the
firmware and the Music Course is intentional and should not be
"fixed" by either:

1. Changing the firmware to match the book, or
2. Changing the book to match the firmware.

### Rationale

1. **Ravine: Phoenix should lead with drum beats.** Slots 1–8 are
   the first slots the user encounters (numerically, on the device,
   and in the on-screen UI). Treating them as drum slots means a
   user who picks up the device cold and starts tapping pads lands
   in the rhythm-first workflow that the project is built around.
   Putting melodic slots 1–8 would invert that.

2. **Differentiation from the PO-33.** A user migrating from a real
   PO-33 should immediately feel that Ravine: Phoenix is its own
   instrument, not a 1:1 clone. The slot-range swap is a small but
   visible "this is not the device you're used to" signal that
   encourages exploration rather than muscle-memory-driven
   frustration.

3. **The Music Course is a tutorial, not a spec.** It teaches the
   *musical concept* (drum vs. melodic, slicing vs. pitch mapping)
   using PO-33 vocabulary because that is what most readers already
   know. The Makers' Companion teaches the *firmware behavior*. Both
   can be correct in their own frame.

4. **The cost of "fixing" is higher than the cost of the
   discrepancy.** Aligning the two would require either rewriting
   the book chapter 2 (and losing the PO-33 mental model many
   readers arrive with) or changing the firmware (and breaking the
   drum-first design intent). Neither is worth it for a
   documentation issue.

### Consequences

**Easier:**
- New users with a PO-33 background can read the Music Course and
  learn the musical concepts without learning a new vocabulary.
- New users with no PO-33 background can pick up Ravine: Phoenix and
  start making beats immediately (slots 1–8 are drums, which is the
  intuitive starting point for beat-making).
- The firmware can ship with a drum-first default state.

**Harder:**
- The Music Course and the firmware disagree. A user who reads the
  book and then records into slot 9 expecting drum behavior will
  get melodic behavior instead. The book should grow a sidebar note
  acknowledging this (separate from this ADR — tracked as a
  documentation TODO).
- `docs/DESIGN.md` line 1240 is itself inconsistent with the rest of
  DESIGN.md (it says "slots 1–8 melodic, slots 9–16 drum", which
  describes PO-33, not Ravine: Phoenix). This is a separate bug.
- Future contributors will be tempted to "fix" one side or the
  other. This ADR is the reason not to.

### Reversibility

If we decide to reverse the firmware (back to PO-33 convention:

- Touch every `slot < SLOT_DRUM_COUNT` check in `main/audio/amy_bridge.c`
  and replace with `slot >= SLOT_DRUM_COUNT` (or rename the constant).
- Update `main/config.h` if the per-slot max bytes also need to
  swap (currently drum = 2 s max, melodic = 3 s max — PO-33 caps
  drum slots shorter, so this would also reverse).
- Update the Music Course chapter 2 to match (which would mean
  removing the "lead with drums" framing from the tutorial).
- Update the Makers' Companion (which would then read like the book).
- Estimated effort: ~half a day for code + docs, plus regression
  testing on the auto-slicing math.

If we decide to align the book to the firmware:

- Rewrite chapter 2 of `docs/Ravine_Phoenix_Music_Course.md` (lines
  312, 340, 341, ~20 other places).
- Update Part III's "melodic slots (slots 1–8)" framing.
- Estimated effort: ~2 hours.

We will revisit this ADR if any of the following become true:

- Real-world user reports indicate the discrepancy is causing
  confusion in the field (e.g., a flurry of GitHub issues saying
  "I recorded into slot 9 and it played pitched, not sliced").
- We ship a "PO-33 compatibility mode" toggle that switches the
  firmware to the PO-33 convention, in which case the book becomes
  accurate under that mode and the firmware is internally
  inconsistent.
- The drum-first design intent is revisited and reversed (e.g., if
  the project pivots toward melodic-first features).

### See also

- `docs/Ravine_Phoenix_Music_Course.md` chapter 2 (lines 312, 340–341)
  — describes PO-33 convention.
- `docs/MAKERS_COMPANION.md` lines 580–581, 2333 — describes
  Ravine: Phoenix convention (matches firmware).
- `docs/DESIGN.md` line 140, 892–893 — Ravine: Phoenix convention
  (matches firmware). Line 1240 — PO-33 convention (a bug, separate
  from this ADR).
- `main/config.h` lines 23–25 — `SLOT_DRUM_COUNT = 8`.
- `main/audio/amy_bridge.c` lines 143–150 (`auto_note_for_step`),
  175–194 (`auto_slice_for_step`) — the runtime checks.
- `main/tests/test_amy_bridge.c` lines 49–62 — confirms the
  firmware convention.
- Commit `4c1e971` — fixed an unrelated slicing-text bug in the
  Music Course chapter 2 that incorrectly described slicing as
  covering only pads 9–16. That bug is independent of this ADR.

---

## ADR-0003 — The PO-33 manual is the ground truth for UI interactions

- **Status:** Accepted
- **Date:** 2026-10-06
- **Deciders:** Peter (project owner).

### Context

The modifier buttons (`SOUND`, `PATTERN`, `BPM`, `REC`, `FX`, `PLAY`,
`WRITE`) and the 16 step pads are documented in four places — the design
doc (`docs/DESIGN.md`), the hardware guide (`hardware/HARDWARE.md`), the
two long-form books (`docs/MAKERS_COMPANION.md`,
`docs/Ravine_Phoenix_Music_Course.md`), and the project website
(`website/src/`). An audit found each surface described the same gestures
differently, and several described behaviour the firmware does not have
(or omitted behaviour it does). Root cause: no single source of truth.

A second class of problem is conceptual: the pads do **not** map 1:1 to
the 16 sample slots. The PO-33 manual defines two distinct verbs —
`select sound` (hold `SOUND` + number) and `play a sound`
(`[select sound]` press number) — and its definitions state that a melodic
sound "plays whole sound on a scale" while a drum sound "plays slice of a
sound". So after selecting a sound, the 16 pads are a *scale / slicer*
for that one sound. Several documents instead said "pad N plays slot N",
which is wrong.

### Decision

1. **The PO-33 operator manual is the specification for every UI
   interaction.** The canonical mirror is
   [lode/PO-33 README](https://raw.githubusercontent.com/lode/PO-33/main/README.md).
   Where a document — or the firmware — disagrees with the manual, the
   manual wins.
2. **The only sanctioned deviations are the accepted ADRs.** In practice
   that is ADR-0002 (the firmware swaps the melodic/drum slot ranges).
   ADR-0001 (website templating) is not a UI interaction.
3. **A single control reference is created and kept authoritative:**
   `docs/CONTROL_REFERENCE.md`. Every other surface cites it instead of
   restating gestures.
4. **Deviations are labelled, not hidden.** The reference uses four
   labels: ✅ (matches the manual), ⚠️ (sanctioned ADR-0002 deviation),
   ➕ (Ravine: Phoenix-only gesture not in the manual), ❌ (manual feature
   not implemented). Documents reproduce the labels rather than quietly
   "fixing" the text to look conformant.

### Rationale

- Readers of the books arrive from a real PO-33; describing invented
  gestures as if they were standard causes exactly the frustration the
  books are meant to remove.
- A single reference makes drift detectable — the previous four-way
  divergence was only found by an ad-hoc audit.
- Honest labelling of ➕/❌ items lets the firmware close gaps later
  (or not) without the documentation having lied about them in the
  meantime.

### Consequences

**Easier:** one file to update when a gesture changes; books and website
can be checked against it mechanically; the pad-meaning confusion (Note A
of the reference) is settled once.

**Harder:** contributors must update `CONTROL_REFERENCE.md` *before* the
prose surfaces; the reference will make the firmware's ➕/❌ gaps visible,
which is intended.

### Scope

This ADR is a **documentation decision**. It does not change firmware
behaviour. The ➕ gestures and ❌ gaps are recorded, not altered.

### See also

- `docs/CONTROL_REFERENCE.md` — the authoritative table.
- `docs/architecture-decisions.md` ADR-0002 — the sanctioned slot-range swap.
- `main/ui/input.c` — the dispatcher the reference is generated from.

---

## ADR-0004 — Samples belong to their sketch; PSRAM holds at most one sketch's pool

- **Status:** Accepted
- **Date:** 2026-10-06 (commit `tbd` — pending)
- **Deciders:** Peter (audit + patch)

### Context

The PO-33 is "one song = one sketch". Ravine: Phoenix v2 supports **N**
sketches per device (per the original ADR-0002 / DESIGN.md §11
multi-sketch system). Each sketch owns a sample pool of up to 3.37 MB
(`SAMPLE_POOL_SIZE_BYTES`) plus 16 patterns (≈48 KB) plus a chain
(≤128 B). The N16R8 module has 8 MB of PSRAM and ~3–4 MB of that is
available for samples after the TFT framebuffer (150 KB), AMY
runtime state, and a per-render scratch block.

The audit ("check Ravine: Phoenix bootup logic", 2026-10-05) found
that `storage_load_all()` historically loaded all 16 slots' worth
of samples from a *flat* on-disk layout (`/sketches/s0.bin..s15.bin`)
into PSRAM, conflating sketches and conflating "the device's samples"
with "one sketch's samples". This blocked the multi-sketch system
from being useful: switching sketches would overwrite the previous
sketch's samples in PSRAM with no way to get them back.

### Decision

1. **Samples are owned by their sketch** and persisted under
   `/sketches/<id>/samples.bin` (256-byte slot table + raw PCM).
   The on-disk format is defined once, in `docs/DESIGN.md §11.2a`,
   and `storage_sketch_save_samples()` / `storage_sketch_load_samples()`
   round-trip it byte-for-byte.
2. **PSRAM holds at most one sketch's sample pool at a time** — the
   *active* sketch's. `amy_bridge.c`'s 3.37 MB pool is partitioned
   into the 16 slots, all of which belong to the active sketch.
3. **`storage_load_all()` is now sketch-scoped.** It reads NVS key
   `sketches/active_id` (default `"0000"`) and brings only that
   sketch's patterns + chain + samples into PSRAM. If the folder
   doesn't exist (fresh flash, factory-reset since last boot),
   `storage_load_all()` returns ESP_OK with RAM empty — the device
   behaves like a brand-new one.
4. **`storage_save_all()` writes to the active sketch's folder** and
   bumps the NVS `sketches/active_id` so the next boot lands here.
   It is called both by the `save` UART verb and automatically from
   `power_mgmt_enter_deep_sleep()` before the chip sleeps.
5. **NVS namespace `"sketches"`, key `"active_id"`** is the
   canonical "which sketch is active right now" pointer. Boot reads
   it; save and sketch-picker-load write it. `storage_factory_reset()`
   clears it so the next boot starts fresh.

### Rationale

- **Sketch identity is preserved across power-off** (the PO-33
  contract). Power-off + power-on lands the user on the same
  sketch, with the same patterns, the same chain, and the same
  samples — even trim settings, which round-trip through
  `amy_bridge_set_trim()` after `amy_bridge_register_slot()`.
- **Memory is bounded.** With the active sketch's pool ≤ 3.37 MB,
  the device never asks for more PSRAM than the chip has — no
  "loaded 4 sketches' worth of samples, OOM-killed the I²S render
  task" failure mode.
- **Sketch export / backup becomes a folder copy** (or a `.zip` in
  v3) — a sketch is one self-contained directory. ADR-0004 is a
  prerequisite for v3's `storage_sketch_export()`.
- **The "16 MB flash > 8 MB PSRAM" framing is misleading**: the
  issue is not total flash size but per-sketch sample-pool size
  matching the PSRAM pool exactly. This decision matches the
  existing `SAMPLE_POOL_SIZE_BYTES = 40 s × 44.1 kHz × 2 B`
  budget and the existing 4 MB free-in-sketch math in DESIGN.md
  §11.8.

### Consequences

**Easier:**
- `app_main()` boot path can call `storage_load_all()` once after
  `amy_bridge_init()` and the user wakes up where they left off —
  zero manual steps.
- Sketch picker (`WRITE` long-press) loads a chosen sketch by ID
  and `storage_sketch_load_with_samples()` does the right thing
  (load patterns + chain + samples + bump NVS).
- The `samples.bin` format is intentionally a single contiguous
  blob, not 16 per-slot files: one `fwrite()` per save instead of
  16, and one `fread()` per load. Atomic via `<path>.tmp` +
  `rename()`.

**Harder:**
- Switching sketches now means: save current sketch → load new
  sketch → overwrite PSRAM pool. The previous sketch's samples are
  only safe if it was just saved. (If the user edits then switches
  without saving, the edit is lost. Mitigation: the deep-sleep path
  auto-saves first; a debounced on-edit auto-save is v2 future
  work tracked in §11.5.)
- Switching sketches is *not* instant — `storage_sketch_load_samples()`
  does one `fread()` of up to 3.37 MB + per-slot `memcpy` +
  `amy_bridge_register_slot()` calls. At 44.1 kHz sample rate and
  PSRAM bandwidth, this is bounded by the I²S render task sharing
  the SPI bus with LittleFS — empirically < 100 ms for a typical
  1.5 MB sketch. Future: move the load to a low-priority task to
  avoid any I²S glitch.

### Scope

This ADR is a **firmware + filesystem decision**, not a
documentation decision (unlike ADR-0003). It changes the public API
of `storage.{h,c}` (adds `storage_get_active_sketch_id()`,
`storage_set_active_sketch_id()`, `storage_sketch_load_samples()`,
`storage_sketch_save_samples()`, `storage_sketch_load_with_samples()`,
and changes the meaning of `storage_load_all()` / `storage_save_all()`
from "all sketches" to "active sketch"). The user-facing UART verbs
(`save`, `load`) keep their names — the wire protocol doesn't
change.

### See also

- `docs/DESIGN.md §5.6`, §8, §11.2, §11.2a, §11.4, §11.5, §11.6 — updated
  to match the implemented behaviour.
- `main/storage/storage.{h,c}` — the implementation.
- `main/system/power_mgmt.c::power_mgmt_enter_deep_sleep()` —
  pre-sleep save.
- `main/main.c::app_main()` — boot-time load (step 4a).
- ADR-0002 — the drum/melodic slot-range swap (orthogonal).
- ADR-0003 — UI-interaction ground truth (orthogonal).
