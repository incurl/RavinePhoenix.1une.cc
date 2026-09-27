# Hardware design files

This folder holds the **design files** for the PO-33 K.O! ESP32-S3 emulator hardware: schematics, PCB layouts, gerbers, enclosure models, BOM, datasheets, and assembly photos.

## Layout

| File / folder | Contents |
|---|---|
| `HARDWARE.md` | The beginner build guide (markdown; ~5 600 words). Read this first. |
| `schematic/` | KiCad `.kicad_sch`, schematic-as-PDF, netlists. |
| `pcb/`       | KiCad `.kicad_pcb`, gerber `.gbr`, drill files `.drl`, BOM-as-CSV. |
| `enclosure/` | 3D models: `.step`, `.stl`, `.3mf`, FreeCAD `.FCStd`, OpenSCAD `.scad`. |
| `datasheets/`| PDF excerpts of the PCM5102A, INMP441, ILI9341 (useful when reviewing the schematic). |
| `assembly-photos/` | Build photos for documentation and tutorials. |

## Conventions

- **Source files (KiCad, FreeCAD, OpenSCAD) are committed.**
- **Manufacturing outputs (rendered STEP previews, gerber ZIPs) are not committed** — they go in GitHub Releases alongside the firmware `.bin` files. Reason: a single gerber ZIP can be 200 KB; several revisions would bloat the repo.
- **Each design revision gets its own subfolder**: `schematic/rev-A/`, `pcb/rev-A/`, etc. The top-level files refer to the latest revision. Old revisions are kept forever for archaeology.
- **Licence**: hardware is MIT (same as the firmware). If you prefer a hardware-specific licence like CERN-OHL-S for a real schematic, add a `LICENSE_HARDWARE` file at the root of `hardware/` and reference it in the schematic's metadata.
- **File names**: `main.kicad_sch`, `main.kicad_pcb`, `BOM.csv` at the top of each subfolder. Multi-revision: `main.kicad_sch` always points to the latest; revisions live in `rev-A/`, `rev-B/`.

## Where to start

- To **build one**: read `HARDWARE.md`.
- To **design one**: open the schematic in `schematic/` (when it exists), modify it, export gerbers to `pcb/<rev>/`, then send the gerber ZIP to your favourite PCB fab house (JLCPCB, PCBWay, OSH Park all accept KiCad gerbers).
- To **report a hardware bug**: open an issue with photos and a description of what went wrong.

## Contributing

Pull requests welcome. For new PCB revisions:

1. Copy the latest revision's schematic + PCB into a new `rev-X/` folder.
2. Add a brief `CHANGELOG.md` line describing what changed and why.
3. Export fresh gerbers and BOM.csv.
4. Update the top-level `schematic/main.kicad_sch` symlink (or reference) to point at the new revision.
5. Open a PR.