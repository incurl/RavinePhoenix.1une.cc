# PO-33 K.O! — project website

Static site for [ravinephoenix.1une.cc](https://ravinephoenix.1une.cc).
Built with [Eleventy](https://www.11ty.dev/) + [Tailwind CSS](https://tailwindcss.com/),
flashing powered by [ESP Web Tools](https://esphome.github.io/esp-web-tools/).

> **Why Eleventy?** See [ADR-0001](../docs/architecture-decisions.md#adr-0001--use-eleventy-11ty-for-the-static-website).

## Develop

```bash
npm install
npm run dev      # http://localhost:8080, hot reload
```

## Build

```bash
npm run build    # outputs _site/
```

`_site/` is what GitHub Pages serves.

## Deploy

Pushed to `main` → GitHub Actions builds and deploys `_site/` to GitHub Pages.

DNS for `ravinephoenix.1une.cc`:
```
CNAME  ravinephoenix.1une.cc  →  <your-github-username>.github.io
A     ravinephoenix.1une.cc  →  185.199.108.153
A     ravinephoenix.1une.cc  →  185.199.109.153
A     ravinephoenix.1une.cc  →  185.199.110.153
A     ravinephoenix.1une.cc  →  185.199.111.153
```

HTTPS via Let's Encrypt is automatic.

## Populating real firmware binaries

The `public/firmware/*.bin` files in this repo are placeholders. See
[`public/firmware/README.md`](./public/firmware/README.md) for the
build + copy steps.

## Layout

```
website/
├── package.json
├── .eleventy.js
├── tailwind.config.js
├── postcss.config.js
├── src/
│   ├── _data/site.js
│   ├── _includes/{base,navbar,footer}.njk
│   ├── assets/{css,js,img}/
│   ├── index.njk            # /
│   ├── hardware.njk         # /hardware/
│   ├── workflow.njk         # /workflow/
│   ├── docs.njk             # /docs/
│   └── download.njk         # /download/
└── public/                  # passthrough copied to _site/
    ├── CNAME
    ├── favicon.svg
    └── firmware/
        ├── manifest.json
        ├── README.md
        └── *.bin (placeholders)
```