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