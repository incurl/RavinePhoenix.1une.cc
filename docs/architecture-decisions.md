# Architecture Decision Records

This document captures the significant architectural decisions made in the
RavinePhoenix project. Each entry follows the lightweight
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
`ravinephoenix.1une.cc` that would:

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

The RavinePhoenix firmware **reverses** this assignment:

- **Slots 1–8:** drum (auto-slicing via F-024).
- **Slots 9–16:** melodic (auto-mapping via F-005).

This is visible in `main/config.h` (`SLOT_DRUM_COUNT = 8`), in
`main/audio/amy_bridge.c` (the `slot < SLOT_DRUM_COUNT` checks in
`amy_bridge_auto_note_for_step` and `amy_bridge_auto_slice_for_step`),
and in the test suite (`main/tests/test_amy_bridge.c:49-53`).

The Music Course book (`docs/RavinePhoenix_Music_Course.md`) describes
the **PO-33** convention in chapter 2 — drum = 9–16, melodic = 1–8 —
because the book is written as a tutorial that assumes the user is
coming from a PO-33 background. The Makers' Companion
(`docs/MAKERS_COMPANION.md`) describes the **RavinePhoenix** convention
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

1. **RavinePhoenix should lead with drum beats.** Slots 1–8 are
   the first slots the user encounters (numerically, on the device,
   and in the on-screen UI). Treating them as drum slots means a
   user who picks up the device cold and starts tapping pads lands
   in the rhythm-first workflow that the project is built around.
   Putting melodic slots 1–8 would invert that.

2. **Differentiation from the PO-33.** A user migrating from a real
   PO-33 should immediately feel that RavinePhoenix is its own
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
- New users with no PO-33 background can pick up RavinePhoenix and
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
  describes PO-33, not RavinePhoenix). This is a separate bug.
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

- Rewrite chapter 2 of `docs/RavinePhoenix_Music_Course.md` (lines
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

- `docs/RavinePhoenix_Music_Course.md` chapter 2 (lines 312, 340–341)
  — describes PO-33 convention.
- `docs/MAKERS_COMPANION.md` lines 580–581, 2333 — describes
  RavinePhoenix convention (matches firmware).
- `docs/DESIGN.md` line 140, 892–893 — RavinePhoenix convention
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
