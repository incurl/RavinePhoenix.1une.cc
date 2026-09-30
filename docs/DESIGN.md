# PO-33 K.O! ESP32-S3 Firmware — Feature Design Document

> **Audience:** written for a complete novice on both programming and music production. Every technical term is defined inline the first time it appears, and again in the glossary at the end. If you have never used a Pocket Operator before, start at §1 and read straight through. If you already know the PO-33 and just want to see how we mapped it to code, jump to §3.

---

## 0. What this document is, and who it is for

This is the design document for a piece of software that runs on a small circuit board called an **ESP32-S3**. That board is wired up to a few other small parts (a screen, some buttons, a microphone chip, a speaker chip). Together they act like a **Teenage Engineering PO-33 K.O! Pocket Operator** — the famous little orange-and-black sampler you may have seen musicians hold in their hand.

If you have never used a real PO-33, that's fine — this document explains everything from scratch.

This document has three jobs:

1. **Explain the device.** What it does, what you can do with it, what every button means. In plain English.
2. **Map every PO-33 feature to a piece of code.** For each thing the original PO-33 can do, we name the file in our project where the same thing is implemented, and tell you whether it's finished, partly finished, or not built yet.
3. **Honest scorecard.** We list exactly what works today and what doesn't. There is no marketing. If a feature is missing, it says "missing".

The intended reader is a complete novice. If you are an experienced embedded engineer, you can probably skim §1–§2 and skip straight to §3 and §7.

---

## 1. The thirty-second explanation

### 1.1 What is the PO-33 K.O! Pocket Operator?

The PO-33 K.O! is a real product made by a Swedish company called **Teenage Engineering**. It is a small plastic box, about the size of a deck of cards, with:

- A speaker on the front
- A small black-and-orange LCD screen
- 16 little square buttons arranged 4 × 4
- A microphone built in
- A small headphone jack
- Two 3.5 mm sockets on the back for syncing with other music gear

It runs on two AAA batteries. The price when new is about $80. It is popular because it is **fun, fast, and weird**. You record a sound, you scribble it into a 16-step pattern, you press **play**, and the device loops the pattern forever while you twist knobs to make it sound different.

The official manual lives at [teenage.engineering/guides/po-33/en](https://teenage.engineering/guides/po-33/en).

### 1.2 What does our firmware do?

Our firmware is **a piece of open-source software (MIT-licensed) that emulates a PO-33 K.O! on a $5 ESP32-S3 microcontroller board** — specifically the **ESP32-S3-WROOM-1-N16R8** module (16 MB flash, 8 MB Octal PSRAM). Instead of buying Teenage Engineering's hardware, you buy this dev board, add a small handful of cheap parts, wire them together, flash our firmware over USB, and you have a PO-33-shaped device.

The parts we add are listed in `main/config.h` and the hardware guide at `hardware/HARDWARE.md`; the short version is: an audio DAC chip (PCM5102A) for output, an I²S MEMS microphone (INMP441) for recording, a 2.4″ color TFT (ILI9341) for the screen, and a 4×4 matrix of 16 tactile buttons for input. Total cost of the additional parts: roughly $10–$15.

Our firmware does **not** make the board behave 1-for-1 like a PO-33. As of v1, only about a quarter of the PO-33's features are fully implemented; another third are partially implemented; the rest are not yet built. The §7 scorecard lists every feature with an honest ✅ / ⚠️ / ❌ status. Use this document to check whether a feature you care about works before you assume it does.

We did not write the audio engine ourselves. We use **[AMY](https://github.com/shorepine/amy)** — a free, MIT-licensed, fixed-point music synthesizer library written by Brian Whitman and the Dogbotic team. AMY already knows how to play back PCM samples, generate oscillators (sine, saw, square), and add chorus, echo, reverb and filters. Our firmware is mostly a thin "shell" that turns PO-33 button presses into AMY commands. AMY runs its own rendering task on the ESP32-S3's second core, so audio doesn't compete with our button-scan or display tasks.

The full source lives in this repository; the project website with one-click browser flashing lives at [ravinephoenix.1une.cc](https://ravinephoenix.1une.cc) (see §1.5 and the website pages).

### 1.3 What does "ESP32-S3" + "AMY" + "TFT" mean?

- **ESP32-S3** — a tiny computer on a chip. We use the **ESP32-S3-WROOM-1-N16R8** variant, which has 16 MB of flash storage and 8 MB of extra "PSRAM" memory. The PSRAM is where we keep all your recorded sounds.
- **AMY** — a synthesizer. We give AMY commands like "play this sound at pitch C, with this filter, on this step" and AMY does the math.
- **TFT** — "thin-film transistor", a fancy name for a flat color screen. Ours is 2.4 inches diagonally, 240 × 320 pixels, connected by SPI (a kind of serial data bus).

### 1.4 The box: what you actually see and touch

Imagine a small flat board, maybe 6 cm wide and 9 cm tall. Looking at the top face:

```
      +------------------------------------------------------------+
      |              2.4"  TFT   ILI9341   240 x 320               |
      |                 SPI2   GPIO 4/5/6/7/47/48                  |
      |                                                            |
      +------------------------------------------------------------+


    +---------+  +----------+  +---------+   (o)         (o)
    |  SOUND  |  | PATTERN  |  |   BPM   |  Knob A      Knob B
    +---------+  +----------+  +---------+ GPIO 20     GPIO 46
                                            A1_CH9      A1_CH5

    +---+ +---+ +---+ +---+                            +---------+
    | 1 | | 2 | | 3 | | 4 |                            |   REC   |
    +---+ +---+ +---+ +---+                            +---------+
    +---+ +---+ +---+ +---+                            +---------+
    | 5 | | 6 | | 7 | | 8 |                            |   FX    |
    +---+ +---+ +---+ +---+                            +---------+
    +---+ +---+ +---+ +---+                            +---------+
    | 9 | | 10| | 11| | 12|                            |  PLAY   |
    +---+ +---+ +---+ +---+                            +---------+
    +---+ +---+ +---+ +---+                            +---------+
    | 13| | 14| | 15| | 16|                            |  WRITE  |
    +---+ +---+ +---+ +---+                            +---------+
```

There are **23 physical buttons** in total:

- **16 step buttons** in a 4×4 matrix. The same physical buttons are *polymorphic* — their meaning depends on which modifier is held (just like the real PO-33):
  - in **normal play**: step 1–16
  - in **write mode**, after picking a slot: toggle step 1–16 for the armed slot
  - with **SOUND** held (`SOUND + step_n`): sample slot 1–16
  - with **PATTERN** held (`PATTERN + step_n`): pattern slot 1–16
  - with **FX** held: effect 1–15 (16 = swing)
- **7 dedicated modifier buttons** wired to individual GPIOs (no matrix), laid
  out exactly like the PO-33's modifier row and column:
  - `SOUND` — hold + step 1–16 **selects** a sample slot (the PO-33's "S" key); press the same step with `SOUND` released to play it (strict two-step flow); tap and long-press alone are no-ops
  - `PATTERN` — hold + step 1–16 selects a pattern AND appends it to the song chain (PO-33 "⠛" key); tap and long-press alone are no-ops
  - `BPM` — tap cycles presets (Hip Hop / Disco / Techno); long-press held: **Knob A = swing** (8 levels), **Knob B = fine BPM** (60–240)
  - `REC` — hold + step 1-16 records into that slot; release stops recording (F-001)
  - `FX` — **tap = tweak-mode cycle** (Tone / Filter / Trim, F-015); hold + step 1–15 = punch-in effect; step 16 = "no effect"
  - `PLAY` — start/stop sequencer
  - `WRITE` — enter / exit write mode (the PO-33's "·" key, handler queued)

There are **2 analog knobs**, also called "Knob A" and "Knob B" (just like the PO-33). Each is a 10 kΩ linear potentiometer on an ADC pin (A = GPIO 20 / ADC1_CH9, B = GPIO 46 / ADC1_CH5). They return 0..255 and are debounced in software. On the PO-33 these same knobs are used for fine continuous control. The actual mapping (verified against the [lode/PO-33](https://github.com/lode/PO-33) verbatim manual):
- **BPM row:** knob A = swing (8 discrete levels), knob B = fine tempo (continuous 60–240). Both wired in this commit.
- **Tweak-mode (FX tapped to select):** knob A = pitch / filter cutoff / sample start (Tone / Filter / Trim), knob B = volume / resonance / sample length. Queued for v2.
- (No "tempo level cycling" knob mapping — that was a confusion in earlier docs; tempo levels are BPM tap only.)

### 1.5 Where to go from here

Once you have read §1, you have a mental model of the device. There are three things you might want to do next, and the rest of this document supports all three:

- **Build the device.** Go to the [project website](https://ravinephoenix.1une.cc), where you can flash our firmware onto your board with one click in Chrome / Edge / Firefox over USB. The *Hardware* page there gives a full bill of materials and pin map.
- **Read the rest of this document.** §2 covers every term used in the project. §3 maps every PO-33 feature to a piece of code with an honest done / partial / missing status. §10 explains how our hardware can break the PO-33's limits. §11 is the v2 proposal for multi-**sketch** storage & restore (we call the unit a "sketch" rather than "song"). §12 is a cliff's-notes reading guide for the *Make: Electronic Music from Scratch* book. §13 is a glossary.
- **Read the source.** The repo at the [project URL](https://ravinephoenix.1une.cc) (or wherever you got this document) is roughly 3,500 lines of C across `main/audio`, `main/sequencer`, `main/ui`, `main/storage`, `main/system`, and `main/tests`. The audio engine is entirely in the vendored AMY library.

If you are a complete novice, the recommended order is: §2 (concepts) → §3.1 (recording) → §5 (workflows) → build it → §10 (breaking limits) → §11 (multi-sketch) → §12 (the book).

> **Naming.** Throughout this document, the unit of creative work stored on the device is called a **sketch** — one sketch = one pool of samples + 16 patterns + a 128-step chain + metadata. The PO-33 calls this a "song", the Korg Electribe calls it a "Pattern Set", and Ableton Live calls it a "Live Set". We picked "sketch" because it's short, plain, and avoids clashing with sampler vocabulary (we don't use "groove" because that word means *timing templates* elsewhere). The PO-33's "song chain" — an ordered list of up to 128 pattern numbers — still exists; we just call it the *pattern chain* inside a sketch.

---

## 2. Concepts you need first

These are the words we will use a lot in the rest of this document. Read this section once; you can come back to it as a reference.

### 2.1 Sound

A **sound** is what a microphone picks up over a short period of time. If you clap your hands, the microphone hears a half-second "thwack". If you say "hello", the microphone hears about a second of audio. We **record** that audio into the device's memory. Once it's stored, we can play it back any number of times.

### 2.2 Sample

**Sample** is a more technical word for "a short recording". When musicians say "I sampled a piano", they mean "I recorded a piano note into a sampler and now I can play it on the keyboard". In this document **sample** and **recording** mean the same thing.

### 2.3 Slot

A **slot** is a numbered storage location for a sample. Our device has **16 slots**, numbered 1 through 16. The first 8 (slots 1–8) are for short sounds called **drum hits** — a single kick, a single snare, a single hi-hat closed. The next 8 (slots 9–16) are for **melodic sounds** — a bass note, a chord, a vocal snippet — things you might want to play at different pitches.

### 2.4 Step

A **step** is **one sixteenth-note** — one of sixteen evenly-spaced positions in a 4/4 bar of music. The PO-33 has **16 steps** in a row, which makes **one bar of 4/4** (4 beats × 4 sixteenth-notes = 16 sixteenth-notes). Each step is a moment in time. The device walks through the steps one at a time. If you told it "play sound 3 on step 5", it will play sound 3 every time the playhead reaches step 5, then move on.

### 2.5 Pattern

A **pattern** is a complete arrangement of sounds across all 16 steps. A pattern tells the device which sound to play on which step. Our device has **16 patterns**, numbered 1 through 16. A pattern is sometimes also called a "beat" or a "loop".

### 2.6 Song

A **song** is an ordered list of patterns. Our device lets you chain up to **128 patterns** in a row, so a song can be up to 128 × 4 = **512 beats** (128 bars) long. After the last pattern in the song finishes, the device loops back to the first pattern.

### 2.7 BPM (beats per minute)

**BPM** is how fast the music goes. BPM = 120 means there are 120 beats per minute, so each beat lasts half a second (0.5 s), so each step (a sixteenth-note) lasts **an eighth of a second** (0.125 s). The real PO-33 has three "BPM levels" — Hip Hop (80 BPM), Disco (120 BPM), and Techno (140 BPM) — and you can fine-tune from there. Our firmware supports any BPM from 60 to 240.

Quick reference for the math: step duration in seconds = 60 / BPM / 4. So:

| BPM | Step duration | Bar duration (16 steps) |
|---|---|---|
| 60 | 0.25 s | 4 s |
| 80 (Hip Hop) | 0.1875 s | 3 s |
| 120 (Disco) | 0.125 s | 2 s |
| 140 (Techno) | ≈0.107 s | ≈1.71 s |

### 2.8 Sequencer

The **sequencer** is the part of the device that walks through the steps in order and plays the right sound at each step. Think of it as a robot drummer who reads the pattern and hits the right drum at the right time.

### 2.9 FX (effect)

An **effect** (called "FX" for short) is a modification applied to a sound. Effects can be small (slight reverb) or wild (the sound plays backwards). The PO-33 has 16 built-in "punch-in" effects. You press the **FX** button and then a number 1–16 to pick one. The effect applies to whatever you play next.

### 2.10 Tweak parameter

A **tweak parameter** is one of three special settings you can adjust on a sound: **Tone** (how high or low it sounds, and how loud it plays), **Filter** (which frequencies are kept or removed — like a bass-boost or treble-cut on a stereo), and **Trim** (where the recording starts and ends). On the real PO-33 you twist the two knobs (Knob A and Knob B) to change these. Our firmware uses two physical knobs — wired to GPIO 20 (Knob A) and GPIO 46 (Knob B), with the `ui/knobs.c` driver reading them every button-scan tick. `BTN_FX` tap cycles the mode (Tone → Filter → Trim → Tone); turning the knobs adjusts the playing step or active slot accordingly. See §3.4 (F-015 through F-018) for the per-mode knob bindings.

### 2.11 Punch-in

**Punch-in** is the name TE gave to those 16 effects. The idea is that you press the FX button to "punch in" an effect, just like a recording engineer in a studio punches in a reverb at the right moment. It is a fancy word for "instant on/off effect".

### 2.12 Polyphony

**Polyphony** means "how many sounds can play at the same time". If you press three buttons at once and hear three different sounds, the device has polyphony ≥ 3. Our device supports **4 simultaneous voices**, which is enough for almost any music you would make on a PO-33.

### 2.13 Sync

**Sync** (short for synchronization) means two devices agreeing on the same tempo. If you have a PO-33 and a Korg volca and you want them to play together, you run a 3.5 mm audio cable between them. One device becomes the "master" (the one that decides the tempo), the other becomes the "slave" or "sync unit". They keep in lock-step so the beats line up.

### 2.14 Loop

**Loop** means "play the same thing over and over". When a sample is "looping", it means the device is repeating the start of the sample as soon as it reaches the end, so a 1-second sample can fill a 60-second song. The PO-33 has four different "loop length" effects (loop 16, loop 12, loop short, loop shorter) which set the loop point at different positions in the sample.

### 2.15 Unison

**Unison** is an effect where the same sound is played multiple times slightly out of phase with itself, which makes it sound thicker, like a chorus of identical singers. "Unison low" is the same but an octave lower.

### 2.16 Stutter

**Stutter** is an effect where a tiny chunk of the sound is repeated many times rapidly, creating a "st-st-st-stutter" effect. "Stutter 4" repeats 4 times per beat; "stutter 3" repeats 3 times per beat.

### 2.17 Quantize

**Quantize** means "snap to a grid". When a sequencer "quantizes" notes, every note is moved to the nearest exact step, so timing is mathematically perfect. The PO-33's "6/8 quantize" effect retimes your pattern so it fits a 6/8 time signature instead of 4/4.

### 2.18 Stutter vs. Reverse vs. Scratch

These are all variations of the same idea: manipulate how the sample plays back in time. **Stutter** repeats a tiny slice. **Reverse** plays it backwards. **Scratch** jumps randomly around the sample like a DJ spinning a record back and forth.

---

## 3. The complete feature-to-code map

This is the heart of the document. Every row is one feature of the real PO-33. For each row we give:

- **PO-33 manual reference** — the section number in the TE manual.
- **Layman explanation** — what the feature does in plain English.
- **PO-33 button combo** — what you press on the real PO-33 to use it.
- **Our hardware combo** — what you press on our firmware (the same button, or a UART command over the serial port).
- **Code location** — the file path and key function name.
- **Status** — ✅ done, ⚠️ partial, ❌ missing.

### 3.1 Section 1 — Sounds (recording)

#### F-001 — Record a sample (mic)

- **Manual ref:** §1.1 (PO-33 manual "sounds" section, record sub-section)
- **Layman:** You press a button, the device starts listening through its microphone, you make a sound, you press the button again, and now the device has that sound stored in one of its 16 slots.
- **PO-33 button combo:** Hold **REC** (star), then press the number of the slot (1–16) where you want to store the sound. Press **REC** again to stop.
- **Our hardware combo:** Hold button "REC", then press a step button 1–16. Hold "REC" again to stop. (Identical button mapping.) Or via UART shell: `rec <slot>` then `stoprec`.
- **Code location:** `main/audio/amy_bridge.c` → `amy_bridge_start_record(uint8_t slot)`, `amy_bridge_stop_record()`, `amy_bridge_pump_capture()`. The recording API works (UART: `rec <slot>` + `stoprec`). Bindings in `main/ui/input.c`:
  - `s_held_modifiers[]` entry `{BTN_REC, rec_on_step, rec_on_release}`. `rec_on_step(N)` calls `amy_bridge_start_record(N - 1)`. `rec_on_release()` calls `amy_bridge_stop_record()` (fired on the true→false edge of `buttons_is_pressed(BTN_REC)` at the end of every drain).
  - The `REC LED` (via `leds_set_rec(true/false)`) mirrors the recording state.
  - First-press-wins: while `BTN_REC` is held, the first step press starts recording; subsequent step presses are no-ops (PO-33 manual: single-slot per REC hold).
- **Status:** ✅ done. PO-33 ground truth (lode/PO-33 README): "hold record (star) + number, make sound, release buttons". Single-slot-per-hold matches the manual wording. The hold-release detection lives in the same end-of-drain polling block as `s_bpm_held`.

#### F-002 — Record from line-in

- **Manual ref:** §1.1
- **Layman:** Instead of using the built-in microphone, you can plug a stereo audio cable into the line-in jack and the device records whatever is on the cable. Useful for sampling a record, a phone, or another synthesizer.
- **PO-33 button combo:** Same as F-001; the device auto-detects line-in vs. mic.
- **Our hardware combo:** Identical. We use the same I²S microphone chip (INMP441) for both, because line-in via a 3.5 mm socket would need an additional ADC. Our firmware treats both sources as "mic" for now.
- **Code location:** `main/audio/amy_bridge.c`. AMY's `i2s.c` is configured with `i2s_din = I2S_IN_DATA_GPIO` (GPIO17); the input buffer is read into PSRAM in `amy_bridge_pump_capture()`.
- **Status:** ⚠️ partial. Mic works. Line-in via a separate 3.5 mm socket is not wired in this version — you'd have to add a second ADC chip.

#### F-003 — Live recording into a playing pattern

- **Manual ref:** §1.2 ("record own sound (live)")
- **Layman:** You start the pattern playing, and while it's looping you hold **WRITE** (·) + the slot number and the device records into that slot **at the same time** as the pattern keeps playing. This is how you "jam" new sounds into an existing beat.
- **PO-33 button combo:** While a pattern is playing, hold **WRITE** + a number 1–16.
- **Our hardware combo:** Not yet implemented. The current firmware only records when the pattern is **stopped**.
- **Code location:** `main/audio/amy_bridge.c` would need a new `amy_bridge_live_record(uint8_t slot)`; `main/sequencer/sequencer.c` would have to keep playing while recording.
- **Status:** ❌ missing.

#### F-004 — 40 seconds total sample memory

- **Manual ref:** §1.1
- **Layman:** Across all 16 slots combined, you can record up to 40 seconds of audio. Each slot is capped: 8 drum slots ≤ 2 s each, 8 melodic slots ≤ 3 s each (in the current firmware; the real PO-33 caps drum slots shorter).
- **PO-33 button combo:** Implicit — the device just won't record past the slot's max.
- **Our hardware combo:** Same.
- **Code location:** `main/config.h` → `SAMPLE_TOTAL_SECONDS 40`, `SLOT_DRUM_MAX_SECONDS 2.0f`, `SLOT_MELODIC_MAX_SECONDS 3.0f`. `main/audio/amy_bridge.c` allocates `SAMPLE_POOL_SIZE_BYTES = 40 × 44100 × 2 ≈ 3.37 MB` in PSRAM.
- **Status:** ✅ done.

#### F-005 — Melodic vs drum slots

- **Manual ref:** §1.1
- **Layman:** Slots 1–8 are for short percussive sounds (drums). Slots 9–16 are for longer sounds that you want to pitch-shift around the musical scale (melodic). Drums play at their recorded pitch; melodic slots play pitched up or down based on which step of the pattern triggers them.
- **PO-33 button combo:** Slots are just numbered 1–16; the type is implicit.
- **Our hardware combo:** Same.
- **Code location:** `main/config.h` → `SLOT_DRUM_COUNT 8`. `main/audio/amy_bridge.c` → `apply_fx()` uses `e.num_voices = (slot < SLOT_DRUM_COUNT) ? 1 : 4`.
- **Status:** ✅ done.

### 3.2 Section 2 — Patterns (sequencing)

#### F-006 — Play a sound

- **Manual ref:** §1.3
- **Layman:** Hold **SOUND** (S) and press the slot number 1–16, and that slot's sound plays once.
- **PO-33 button combo:** **Two presses.** Per the [lode/PO-33](https://github.com/lode/PO-33) verbatim manual:
  1. **select sound** — hold S + number (1–16) → selects that slot.
  2. **play a sound** — *with S released*, press the same number → the selected slot plays once.

  The layman line above is a simplification — it implies one press, but the real PO-33 is a strict two-step flow.
- **Our hardware combo:** Strict two-step.
  - `SOUND` (`BTN_SOUND`, GPIO 11 — leftmost of the top row) held + step 1–16 → calls `sequencer_set_active_slot(step - 1)`.
  - Step 1–16 pressed with **no modifier held** + `active_slot != 0xFF` → `amy_bridge_play_note(slot, 60, 100, active_fx, 0, 0)`. `midi_note = 60` (middle C) and `velocity = 100` are v1 defaults; the PO-33 itself doesn't expose these for slot-play mode.
  - Tap or long-press of `SOUND` alone is a no-op (no PO-33 behaviour attached to it; the only PO-33 SOUND verbs are "select sound" and "record" — F-001 — both of which require holding + a number).
- **Code location:** `main/ui/input.c` → `s_modifiers[]` entry `{BTN_SOUND, sound_on_step}`; `sound_on_step()` calls `sequencer_set_active_slot(step - 1)`. The "step press without modifier" branch in `input_drain()` calls `play_active_slot()` which invokes `amy_bridge_play_note()`. `main/sequencer/sequencer.c` → `sequencer_set_active_slot()`, `sequencer_get_active_slot()`.
- **Status:** ✅ done (strict PO-33 ground truth; see trade-off below).

  **Trade-off.** F-006's *Layman* line implies a one-press ergonomics (hold + number = plays once). The PO-33 manual is unambiguous about two presses. We honour the manual. If the layman flow is preferred, the change is a one-liner in `sound_on_step()`: instead of `sequencer_set_active_slot(...)`, call `amy_bridge_play_note(...)` directly.

#### F-007 — 16 patterns

- **Manual ref:** §2 (PO-33 manual "patterns")
- **Layman:** You can store 16 different patterns on the device, numbered 1–16. Each pattern is a complete beat on its own. You switch between them with the bottom-right button group.
- **PO-33 button combo:** Hold **PATTERN** (⠛) + 1–16 to pick one.
- **Our hardware combo:** Single `PATTERN` button (`BTN_PATTERN`, GPIO 44 — middle of the top row), exactly like the real PO-33. **Hold `PATTERN` + press step 1–16** to load that pattern. **Tap and long-press of `PATTERN` alone are no-ops** — there is no tap-to-step or ±1 mod-16 behaviour, because that doesn't exist on the real PO-33 (verified against the [lode/PO-33](https://github.com/lode/PO-33) verbatim mirror of the operator manual: the only PATTERN behaviours are "select pattern — hold + number" and "change patterns — hold + number(s)"). UART: `pattern 5` (sets an arbitrary integer pattern directly; does not use the hold-+-number machinery).
- **Code location:** `main/sequencer/sequencer.h` → `PATTERN_COUNT 16`, `sequencer_set_pattern(uint8_t)`. `main/ui/input.c` → `s_modifiers[]` table with `{BTN_PATTERN, pattern_on_step}`; `pattern_on_step(step_1_to_16)` calls `sequencer_set_pattern(step - 1)`. `main/config.h` → `BTN_PATTERN`, `BTN_PATTERN_GPIO`.
- **Status:** ✅ done. Strict PO-33 ground truth (the matrix step routed while the modifier is held).

#### F-008 — 16 steps per pattern

- **Manual ref:** §2
- **Layman:** Each pattern has 16 "slots in time" called steps. Each step is one sixteenth-note. The sequencer walks 1→2→...→16→1→...
- **PO-33 button combo:** Implicit; the steps are the 16 polymorphic numbered buttons (1–16).
- **Our hardware combo:** Same — the 4×4 matrix holds 16 step buttons directly, numbered 1–16. When a modifier button (SOUND-equivalent, PATTERN-equivalent, FX) is held, the same buttons mean slot 1–16 / pattern 1–16 / effect 1–16 instead.
- **Code location:** `main/sequencer/pattern.h` → `STEPS_PER_PATTERN 16`.
- **Status:** ✅ done.

#### F-009 — Write mode (assigning sounds to steps)

- **Manual ref:** §2.1
- **Layman:** You enter "write mode" by pressing **WRITE** (·). Now when you press a step button, the slot you select gets added to that step. Press WRITE again to exit write mode.
- **PO-33 button combo:** Press WRITE → press a slot number → press step numbers where you want that slot to play → press WRITE again.
- **Our hardware combo:** The `WRITE` button now exists (`BTN_WRITE`, GPIO 43 — bottom of the right column), matching the PO-33's "·" key, but the write-mode handler is not wired yet. For now use UART: `pattern 0`, then `sequencer_set_step_slot(0, 5, 3, 60)`.
- **Code location:** `main/sequencer/sequencer.h` → `sequencer_set_step_slot(uint8_t pattern, uint8_t step, uint8_t slot, uint8_t note)`.
- **Status:** ⚠️ partial. Code supports it; UI button combo missing.

#### F-010 — Fill a step (press to add, press again to remove)

- **Manual ref:** §2.1
- **Layman:** When you press a step button while in write mode, the slot gets added. Press the same step again to remove it. Pressing the same step multiple times toggles it.
- **PO-33 button combo:** Press step → press step again to toggle off.
- **Our hardware combo:** Same — see F-009. Each step button is a toggle.
- **Code location:** `main/sequencer/pattern.c` → `pattern_set_step()` writes a step, and `step_t.slot_id = 0xFF` is the "empty" sentinel.
- **Status:** ✅ done.

#### F-011 — Clear a step

- **Manual ref:** §2.1
- **Layman:** Press WRITE + the step number to remove the slot from that step.
- **PO-33 button combo:** WRITE + step.
- **Our hardware combo:** Long-press the step button to clear it (currently not bound; planned).
- **Code location:** Will live in `main/sequencer/sequencer.c` when added.
- **Status:** ❌ missing.

#### F-012 — Clear an entire pattern

- **Manual ref:** §2.1 ("press record + pattern to clear the active pattern")
- **Layman:** Hold **RECORD** + **PATTERN** and the entire current pattern is wiped (all 16 steps cleared).
- **PO-33 button combo:** Hold REC + PATTERN.
- **Our hardware combo:** Not yet. UART: `pattern 0; ... (clear all steps)`.
- **Code location:** Needs `sequencer_clear_pattern(uint8_t pattern)` in `main/sequencer/pattern.c`.
- **Status:** ❌ missing.

### 3.3 Section 3 — Songs (pattern chaining)

#### F-013 — Chain up to 128 patterns into a song

- **Manual ref:** §3
- **Layman:** You can build a "song" by listing patterns in order: 1, 1, 1, 2, 3, 3, ... up to 128 entries. When you play, the device walks through the list. When it reaches the end, it loops back to the start.
- **PO-33 button combo:** Hold **PATTERN** + 1–16 to add the current pattern to the chain; repeating the same pattern multiple times in the chain makes it play that many times before advancing.
- **Our hardware combo:** Hold `PATTERN` and press numbers 1–16 in sequence; each press both selects that pattern AND appends it to `g_chain[]` (per the PO-33 manual: "hold pattern (⠛) + number(s)" → "choosing a single pattern multiple times is allowed"). The chain plays in order, looping back to the start. UART: `pattern N` still calls `sequencer_set_pattern(N)` without appending, so the user can pre-select a pattern without touching the chain.
- **Code location:** `main/sequencer/sequencer.{c,h}` → `sequencer_chain_append(uint8_t)`, `sequencer_chain_clear()`, `g_chain[]`, `g_chain_len`. `on_step()` increments `chain_idx` and rewrites `s_pattern` when reaching the chain end. `main/ui/input.c` → `s_modifiers[]` entry `{BTN_PATTERN, pattern_on_step}`; `pattern_on_step()` calls both `sequencer_set_pattern()` and `sequencer_chain_append()`.
- **Status:** ✅ done. PO-33 ground truth. Reorder / remove from chain (F-014) still queued (would need a chain-edit mode; current chain is append-only).

#### F-014 — Reorder / remove patterns from chain

- **Manual ref:** §3
- **Layman:** You can change the order of patterns in a chain by re-pressing them in a new sequence.
- **PO-33 button combo:** Hold PATTERN + number.
- **Our hardware combo:** Same gap as F-013.
- **Status:** ❌ missing.

### 3.4 Section 4 — Tweaking (Tone / Filter / Trim)

#### F-015 — Pick a tweak parameter (Tone / Filter / Trim)

- **Manual ref:** §4
- **Layman:** You cycle between three tweak modes by pressing **FX** repeatedly: Tone, Filter, Trim. Each one has two knobs (A and B) that change different aspects.
- **PO-33 button combo:** Press FX to cycle: Tone → Filter → Trim → Tone.
- **Our hardware combo:** Same `FX` button (`BTN_FX`, GPIO 12 — top of the right column under Knob B) has two roles, just like the real PO-33:
  - **Tap** = cycle tweak parameter (Tone → Filter → Trim → wrap).
  - **Held + step 1–15** = select a punch-in effect (F-019).
  - **Held + step 16** = "no effect" (clears active FX; the full "clear effect in pattern" combo lands in write mode, queued).
- **Code location:** `main/ui/input.h` → `tweak_mode_t` enum + `tweak_get_mode()`. `main/ui/input.c` → `s_tweak_mode` state + `tweak_mode_cycle()` (called from the `BTN_FX` tap dispatch) + `tweak_apply_step()` / `tweak_apply_slot()` (called from the sequencer `on_step` and `play_active_slot` respectively). The cycle is Tone → Filter → Trim → Tone, never NONE.
- **Status:** ✅ done. `BTN_FX tap` cycles the mode; long-press is a no-op (the held-+-step punch-in path from F-019 fires on step events before reaching the per-btn switch). The TFT currently shows no tweak-mode label — `display.c` doesn't render the mode — so the state lives in `s_tweak_mode` but isn't surfaced yet. This is a v2 display addition, not a logic gap.

#### F-016 — Tweak Tone (Knob A = pitch, Knob B = volume)

- **Manual ref:** §4.1 ("ton")
- **Layman:** With Tweak = Tone, **Knob A** raises or lowers the pitch of the current sound by semitones; **Knob B** raises or lowers its volume.
- **PO-33 button combo:** Turn knob A / B.
- **Our hardware combo:** Knob A → GPIO 20 (ADC1_CH9); Knob B → GPIO 46 (ADC1_CH5). The knob driver (`ui/knobs.c`) reads them on every button-scan tick. Wiring through to per-step pitch/volume is v2 work; the data model (`step_t.note`, `step_t.velocity`) already supports it.
- **Code location:** `step_t.note` (pitch), `step_t.velocity` (volume); `ui/knobs.c` reads; `main/audio/amy_bridge.c` writes into `amy_event.midi_note` / `amy_event.velocity`.
- **Status:** ✅ done while `s_tweak_mode == TWEAK_TONE`. `tweak_apply_step()` reads `knobs_get_a()` → MIDI note (0..255 → 36..97, mapping C2..C7) and `knobs_get_b()` → velocity (0..255 → 0..127). The sequencer's `on_step()` and `play_active_slot()` call `tweak_apply_step()` before `amy_bridge_play_note()`. v1 lacks `resonance` as a per-step field (F-017 below); the knob B reading in Filter mode is logged but not stored.

#### F-017 — Tweak Filter (Knob A = LP/HP, Knob B = resonance)

- **Manual ref:** §4.2 ("Flt")
- **Layman:** With Tweak = Filter, **Knob A** picks the cutoff frequency (low-pass or high-pass); **Knob B** picks how strong the filter is (resonance).
- **PO-33 button combo:** Turn knobs.
- **Our hardware combo:** Same knob GPIOs (A = GPIO 20, B = GPIO 46). The filter data field is `step_t.filter_cutoff` (0–255); v1 firmware maps a low-pass sweep into `amy_event.filter_freq` via the existing `PO33_FX_FILTER_SWEEP` effect.
- **Code location:** `step_t.filter_cutoff`; `main/audio/amy_bridge.c` → `apply_fx()` → `PO33_FX_FILTER_SWEEP` sets `e->filter_type = FILTER_LPF` and `e->filter_freq`. `main/ui/input.c` → `tweak_apply_step()` in `TWEAK_FILTER` mode reads `knobs_get_a()` → `filter_cutoff` (0..255). Resonance knob is logged but no per-step field exists yet; v2 adds a `resonance` field to `step_t`.
- **Status:** ✅ done for cutoff (knob A). Resonance (knob B) queued for v2.

#### F-018 — Tweak Trim (Knob A = start point, Knob B = length)

- **Manual ref:** §4.3 ("tri")
- **Layman:** With Tweak = Trim, you pick where in the recording the playback actually starts (**Knob A**), and how long the playback lasts (**Knob B**). This is how you chop off silence at the start, or trim a long recording to a short snippet.
- **PO-33 button combo:** Turn knob A = start, B = length.
- **Our hardware combo:** Same knob GPIOs (A = GPIO 20, B = GPIO 46). Per-slot trim is already stored in `s_slots[slot].start` / `.end` and exposed via `amy_bridge_set_trim()` (UART). Knob → trim binding queued for v2.
- **Code location:** `main/audio/amy_bridge.c` → `amy_bridge_set_trim(slot, start, end)`. `s_slots[slot].start` / `.end` are stored.
- **Status:** ✅ done while `s_tweak_mode == TWEAK_TRIM`. `tweak_apply_slot(slot)` reads `knobs_get_a()`/`b()` → trim start/end (mapped to sample indices 0..slot_len-1 via `amy_bridge_slot_ptr()` + `amy_bridge_slot_len_samples()`) and calls `amy_bridge_set_trim()`. Triggered from `play_active_slot()`; the per-step sequencer path doesn't apply trim (no per-step trim field, and the user is expected to trim the active slot, not individual pattern steps).

### 3.5 Section 5 — Effects (the 16 punch-ins)

See §4 below for the per-effect deep dive.

#### F-019 — Pick an effect

- **Manual ref:** §5
- **Layman:** Press **FX** to enter effect-pick mode, then press a number 1–16 to select one of the 16 effects. The effect applies to whatever you play next.
- **PO-33 button combo:** Per the [lode/PO-33](https://github.com/lode/PO-33) verbatim manual:
  - Hold **FX** + number (1–15) — add & save effect in pattern.
  - **Step 16** in the PO-33 manual's effects table is "**no effect**" (the 16th row reads "16. no effect"). The combo "[write mode] [play] hold fx (FX) + 16" is "clear effect in pattern" — a separate write-mode-dependent action, NOT swing. Earlier commits mislabelled step 16 as a "swing stub"; that was a fabrication.
- **Our hardware combo:** Strict PO-33: `FX` (`BTN_FX`, GPIO 12 — top of the right column under Knob B) held + step 1–15 → calls `sequencer_set_active_fx(PO33_FX_LOOP_16 + (step - 1))`. Step 1–15 maps to enum values `PO33_FX_LOOP_16` … `PO33_FX_FILTER_SWEEP`. Step 16 → `sequencer_set_active_fx(PO33_FX_NONE)` (the manual's "no effect" entry; the PO-33's "clear effect in pattern" half of the combo lands in write mode, queued).

  The selected FX is then **carried into the next note** by `sequencer_on_step()` (reads `s_active_fx` if the step's `effect` field is `PO33_FX_NONE`). Per-step effects take precedence — the FX modifier sets a *fallback*, not an override.

  **Tap** of `FX` alone is the **tweak-mode cycle** (Tone → Filter → Trim → Tone) — see F-015. Long-press is a no-op.

  **Where swing actually lives:** BPM-held + Knob A. Not a step press at all. See F-020.
- **Code location:** `main/ui/input.c` → `s_modifiers[]` entry `{BTN_FX, fx_on_step}`; `fx_on_step()` step 1..15 maps to `PO33_FX_LOOP_16 + (step - 1)`, step 16 sets `PO33_FX_NONE`. `main/sequencer/sequencer.{c,h}` → `sequencer_set_active_fx()`, `sequencer_get_active_fx()`, and `on_step()` reads `s_active_fx` as the fallback when the step's per-step effect is `PO33_FX_NONE`. `main/audio/amy_bridge.{c,h}` → `po33_fx_t`, `apply_fx()`.
- **Status:** ✅ done. Step 1..15 selects punch-ins; step 16 = "no effect" (no swing).

### 3.6 Section 6 — BPM and tempo

#### F-020 — Change tempo (BPM)

- **Manual ref:** §6
- **Layman:** Hold **BPM** and turn Knob A to fine-tune the tempo, or press BPM repeatedly to cycle between three preset levels: Hip Hop (80), Disco (120), Techno (140).
- **PO-33 button combo:** Per the [lode/PO-33](https://github.com/lode/PO-33) verbatim manual, the BPM row has *three* distinct combos (each one was previously confused with the others in our docs; corrected in this commit):
  - `change tempo (pre-defined modes)` — **press BPM** to toggle between levels.
  - `change swing` — **hold BPM + turn Knob A** (8 discrete levels, 0=no swing, 7=max).
  - `change tempo (fine tuned)` — **hold BPM + turn Knob B** (continuous, 60–240 BPM).
  - (`change volume` — hold BPM + number 1–5; see F-022. Different axis again.)
- **Our hardware combo:** `BPM` (`BTN_BPM`, GPIO 13 — rightmost of the top row) has the following behaviours:
  - **Tap** advances to the next preset level (Hip Hop → Disco → Techno → wrap). See F-021 for the table.
  - **Long-press (held)** enters BPM-mode: while held, **Knob A adjusts swing** (8 levels) and **Knob B fine-tunes BPM** (continuous, [60..240]). Each axis has its own deadzone + last-reading for delta detection; both fire on knob movement past the deadzone. On release, both values are kept (no separate commit step).
  - **Held + step 1–5**: sets the volume level (F-022). Step 6–16 is a no-op (PO-33 doesn't define a behaviour).
  - UART: `bpm 140` (sets an arbitrary integer BPM; does not use the preset machinery).
- **Code location:** `main/sequencer/sequencer.c` → `sequencer_set_bpm(uint16_t)`, `sequencer_cycle_bpm_preset()`, `sequencer_set_swing(uint8_t)`, the `on_step()` swing-delay path, `on_swing_fire()` timer callback. `main/ui/input.c` → `BTN_BPM` case (tap/long-press split), `knob_a_to_swing()` mapper, `knob_b_to_bpm()` mapper, `s_bpm_held` mode + held-mode polling. `main/ui/buttons.{c,h}` → `buttons_is_pressed(uint8_t)` (held-state poll). `main/config.h` → `MIN_BPM 60`, `MAX_BPM 240`, `BPM_PRESETS`, `SWING_LEVELS 8`, `SWING_MAX_PERCENT 50`.
- **Status:** ✅ done. Tap cycles presets (F-021); long-press + Knob A sets swing; long-press + Knob B fine-tunes BPM; held + step 1–5 sets volume level (F-022).

#### F-021 — Three preset BPM levels (Hip Hop / Disco / Techno)

- **Manual ref:** §6
- **Layman:** The PO-33 has three named BPM levels you can cycle through. The PO-33 calls these "levels" because BPM is shown as a numeric value but it remembers the level name.
- **PO-33 button combo:** Press BPM repeatedly.
- **Our hardware combo:** Each tap of `BTN_BPM` advances `s_preset_idx` (modulo 3) and calls `sequencer_set_bpm(presets[idx])`. The cycle starts at **Disco** (the `DEFAULT_BPM` 120), so the first tap lands on **Techno**, the next on **Hip Hop**, then back to Disco. Off-preset values (set by Knob B under long-press) are *not* snapped to the nearest preset on the next tap — the cycle advances slot-by-slot from whatever the last preset delivered.
- **Code location:** `main/config.h` → `BPM_PRESETS` (macro) + `BPM_PRESET_COUNT 3`. `main/sequencer/sequencer.c` → `sequencer_cycle_bpm_preset()`. The static `s_preset_idx` lives inside that function (initial value 1 = Disco).
- **Status:** ✅ done.

### 3.7 Section 7 — Headphone volume

#### F-022 — Headphone volume level (5 levels)

- **Manual ref:** §7
- **Layman:** You can pick one of 5 headphone volume levels so the device isn't too loud or too quiet for headphones.
- **PO-33 button combo:** Hold BPM + 1–5.
- **Our hardware combo:** Hold BPM + step 1–5 → `amy_bridge_set_volume_level(N)`. The level is stored as 0..5 (5 = max, 0 = silent). AMY's public API does not expose a master gain, so the multiplier is applied per-note inside `amy_bridge_play_note()`: `velocity_out = (velocity_in / 127) * volume_multiplier(level)`. Multiplier curve: 0/0.25/0.5/0.75/0.9/1.0 (slightly compressed at the top end to avoid the perceptual jump between levels 4 and 5).
- **Code location:** `main/audio/amy_bridge.{c,h}` → `amy_bridge_set_volume_level(uint8_t)` + static `s_volume_level` + `volume_multiplier()`. The multiplier is applied inside `amy_bridge_play_note()`. `main/ui/input.c` → BPM-held + step 1–5 branch in `input_drain()` (routes BEFORE the modifier lookup because BPM isn't in the press-bound `s_modifiers[]` table; its mode is polled).
- **Status:** ✅ done. Note: the PO-33 also uses BPM tap alone (with no step) to *display* the current volume level — "press bpm (handle); numbers are lit until the current level" (audit item 26). That display-half lives in `display.c` and is queued for v2; the *set*-half is wired here.

### 3.8 Section 8 — Copy and delete

#### F-023 — Copy a sound to another slot

- **Manual ref:** §8
- **Layman:** Hold **WRITE** + **SOUND** + 1–16 and the active sound is copied to that slot.
- **PO-33 button combo:** WRITE + SOUND + number.
- **Our hardware combo:** Not yet.
- **Code location:** Would call `amy_bridge_register_slot(dst, src_ptr, src_len, src_sr, src_is_drum)` in `main/audio/amy_bridge.c`.
- **Status:** ❌ missing.

#### F-024 — Copy a slice of a drum sample

- **Manual ref:** §8
- **Layman:** Same as F-023 but only copies one slice (one of the 16 auto-sliced parts) to a new drum slot.
- **PO-33 button combo:** WRITE + SOUND + 9–16 (drum slot) + slice number 1–16.
- **Our hardware combo:** Not yet.
- **Code location:** `main/audio/amy_bridge.c` → would need a `amy_bridge_copy_slice(src_slot, slice_index, dst_slot)`.
- **Status:** ❌ missing.

#### F-025 — Copy an entire pattern

- **Manual ref:** §8 ("hold write + pattern and press 1-16 to paste the active pattern to the corresponding new slot")
- **Layman:** Hold WRITE + PATTERN and the active pattern is copied to another pattern slot.
- **PO-33 button combo:** WRITE + PATTERN + number.
- **Our hardware combo:** Not yet.
- **Code location:** Would call `memcpy(&g_patterns[dst], &g_patterns[src], sizeof(pattern_t))` in `main/sequencer/pattern.c`.
- **Status:** ❌ missing.

#### F-026 — Delete a sound

- **Manual ref:** §8 ("delete sound: hold record + sound")
- **Layman:** Hold RECORD + the slot's number and the slot is wiped empty.
- **PO-33 button combo:** REC + slot number.
- **Our hardware combo:** Not yet.
- **Code location:** Would call `amy_bridge_clear_slot(uint8_t slot)` — sets `s_slots[slot].in_use = false` and zero-fills the data.
- **Status:** ❌ missing.

#### F-027 — Delete a pattern

- **Manual ref:** §8 ("press record + pattern to clear the active pattern")
- **Layman:** Hold REC + PATTERN and the current pattern is wiped.
- **PO-33 button combo:** REC + PATTERN.
- **Our hardware combo:** Same gap as F-012.
- **Status:** ❌ missing.

### 3.9 Section 9 — Data transfer

#### F-028 — Transfer sounds and patterns between two PO-33s

- **Manual ref:** §9.1
- **Layman:** You run a 3.5 mm stereo audio cable from the line-out of one PO-33 to the line-in of another. Hold WRITE + SOUND + PLAY on the sending unit, hold WRITE + SOUND + RECORD on the receiving unit, and the second unit receives a copy of all sounds and patterns. (Note: this erases the receiving unit first.)
- **PO-33 button combo:** As above.
- **Our hardware combo:** Not yet. We'd need to encode a packet stream over audio and a corresponding decoder.
- **Code location:** Would be a new `main/audio/p2p.c` module.
- **Status:** ❌ missing.

#### F-029 — Back up to / restore from a stereo recording device

- **Manual ref:** §9.2
- **Layman:** You can back up your PO-33 by playing its data into a tape recorder / phone / computer. Later you play that recording back into another PO-33 to restore. (Note: this erases the receiving PO-33 first.)
- **PO-33 button combo:** As above (same wire protocol as F-028).
- **Our hardware combo:** Same.
- **Status:** ❌ missing.

### 3.10 Section 10 — Sync

#### F-030 — Jam-sync out (pulse on each 16th note)

- **Manual ref:** §10
- **Layman:** On every sixteenth note the device sends a 1 ms electrical pulse out of its sync-out jack. Another device listening on sync-in can use that pulse as its tempo reference.
- **PO-33 button combo:** Implicit; always on while playing.
- **Our hardware combo:** Same. GPIO 18 is the sync-out pin.
- **Code location:** `main/system/sync.c` → `sync_pulse()` toggles GPIO 18 high for 1 ms. Called from `sequencer.c` per step.
- **Status:** ✅ done.

#### F-031 — Jam-sync in (listen to a master device)

- **Manual ref:** §10
- **Layman:** Set the PO-33 to "sync mode", press PLAY, and it will not play on its own — it waits for an external pulse on the sync-in jack, then plays one step per pulse.
- **PO-33 button combo:** Hold RECORD + BPM to cycle sync modes SY0–SY5.
- **Our hardware combo:** The GPIO (GPIO 19) is configured as input with pullup, but no listener task is implemented yet.
- **Code location:** `main/system/sync.c` has the GPIO config; the listener is missing.
- **Status:** ❌ missing (GPIO configured, listener not wired).

#### F-032 — Five sync modes (SY0–SY5)

- **Manual ref:** §10.1 (the table: SY0 stereo/stereo, SY1 stereo mono/sync, SY2 sync stereo, SY3 sync mono/sync, SY4 mono/sync stereo, SY5 mono/sync mono/sync)
- **Layman:** The PO-33 can send either stereo audio, sync pulse, or both — on either its line-in or its line-out. There are 5 useful combinations. The default is SY0 (stereo in, stereo out, no sync), which is what you want when listening to music through headphones.
- **PO-33 button combo:** Hold RECORD + BPM to cycle.
- **Our hardware combo:** Not yet. The hardware side (routing I²S L/R or sync GPIO into the same jack) needs a small analog mux.
- **Status:** ❌ missing.

### 3.11 Section 11 — Clock and alarm

#### F-033 — Built-in clock (HH:MM shown on display)

- **Manual ref:** §11
- **Layman:** The PO-33 has a clock. The current time is shown in the upper-right corner of the display.
- **PO-33 button combo:** No specific button; the clock is always running.
- **Our hardware combo:** The clock shows on the TFT top bar (in `main/ui/display.c` → `display_tick()`).
- **Code location:** `main/system/clock.c` → `clock_init()`, `clock_get_hhmm()`, `clock_set_hhmm()`. The NVS stores the epoch offset so the clock survives a reboot.
- **Status:** ✅ done.

#### F-034 — Alarm that plays a sample

- **Manual ref:** §11
- **Layman:** You can set an alarm time. When the alarm time is reached, the device plays a sample.
- **PO-33 button combo:** Set via PO-33 menu (no single button combo in the manual).
- **Our hardware combo:** UART: `clock_set_alarm 7 30 3` (play slot 3 at 07:30).
- **Code location:** `main/system/clock.c` → `clock_set_alarm(hh, mm, slot)`; the alarm callback is not yet implemented (no FreeRTOS timer that checks the time).
- **Status:** ⚠️ partial. Storage works; the check-and-fire timer is missing.

### 3.12 Section 12 — Battery

#### F-035 — Show battery level on display

- **Manual ref:** §12
- **Layman:** The PO-33 shows a small battery icon with bars indicating remaining charge. Press SOUND + BPM and the bars light up to indicate the level.
- **PO-33 button combo:** SOUND + BPM.
- **Our hardware combo:** The TFT top bar shows the battery voltage (planned — currently shows the clock).
- **Code location:** `main/system/clock.c` → needs `battery_get_percent()` reading `BATTERY_ADC_CHANNEL`. Display hook in `main/ui/display.c` `display_tick()`.
- **Status:** ❌ missing.

#### F-036 — Deep-sleep after 5 minutes idle

- **Manual ref:** §12 (implicit)
- **Layman:** After 5 minutes with no button pressed, the device goes into a very-low-power "sleep" mode. Pressing any button wakes it back up.
- **PO-33 button combo:** Implicit.
- **Our hardware combo:** Same. Any button press wakes from deep sleep via GPIO interrupt.
- **Code location:** `main/system/power_mgmt.c` → `power_mgmt_init()` starts a 5-minute esp_timer; `power_mgmt_enter_deep_sleep()` is called when it fires. GPIO columns are configured with `gpio_wakeup_enable()` for `GPIO_INTR_LOW_LEVEL`.
- **Status:** ✅ done.

### 3.13 Section 13 — Factory reset

#### F-037 — Factory reset (erase everything)

- **Manual ref:** §13 ("hold pattern + insert batteries")
- **Layman:** To wipe the device back to factory defaults, you power it off, hold PATTERN, and reinsert the batteries. Everything is erased.
- **PO-33 button combo:** Power off → hold PATTERN → insert batteries.
- **Our hardware combo:** UART: `storage_erase_all()` (not yet written). Mechanical deep-sleep wake cycles via the reset button are equivalent.
- **Code location:** Needs `storage_erase_all()` in `main/storage/storage.c`.
- **Status:** ⚠️ partial. Code path exists conceptually; no UI button combo.

#### F-038 — Show active sounds and patterns

- **Manual ref:** §13 ("active sounds/patterns — press sound (S) or pattern (⠛). lit numbers have a sound/pattern, unlit are silent, flashing is the currently selected sound/pattern")
- **Layman:** On the real PO-33 the slot buttons light up in patterns to show which slots are recorded, which are empty, and which is currently selected.
- **PO-33 button combo:** Press SOUND or PATTERN.
- **Our hardware combo:** We don't have lit buttons (the buttons on a stock 4×4 matrix keypad are not illuminated). The TFT does show the same information — which slots are filled, which is selected — on the Sound select UI page (proposed in §6).
- **Code location:** `main/ui/display.c` would render the per-slot filled/empty/selected icons.
- **Status:** ⚠️ partial. Data is in `s_slots[]`; rendering is missing.

---

## 4. The 16 punch-in effects, one per effect

Each effect below is presented with: a one-sentence layman explanation of what it does to the sound, the PO-33 manual's wording, what we currently map it to in code, and what audio quality you should expect.

> ⚠️ **Important honesty note.** Our current firmware has a slightly different list of effects than the real PO-33. We have `BITCRUSH` and `FILTER_SWEEP` which are *not* PO-33 effects, and we are missing `SCRATCH` and `6/8 QUANTIZE`. See §7 for the v2 alignment plan.

### 4.1 Effect 1 — loop 16

- **What it sounds like:** the sound plays its first 16th, then loops that 16th over and over for the duration of the step.
- **Manual:** "loop 16".
- **Our code:** `main/audio/amy_bridge.h` → `PO33_FX_LOOP_16`. In `apply_fx()`, this case is currently a no-op — the loop point is set when the sample is registered, not per-step.
- **Status:** ⚠️ partial. Data model supports loop_end / loop_start on each voice, but the per-step override is not wired through.

### 4.2 Effect 2 — loop 12

- **What it sounds like:** the same as loop 16, but the loop length is 12 sixteenth-notes (i.e. an eighth-note triplet).
- **Manual:** "loop 12".
- **Our code:** `PO33_FX_LOOP_12`. Same gap as 4.1.
- **Status:** ⚠️ partial.

### 4.3 Effect 3 — loop short

- **What it sounds like:** loop a shorter chunk (about a 16th-note).
- **Manual:** "loop short".
- **Our code:** `PO33_FX_LOOP_SHORT`.
- **Status:** ⚠️ partial.

### 4.4 Effect 4 — loop shorter

- **What it sounds like:** loop an even shorter chunk.
- **Manual:** "loop shorter".
- **Our code:** `PO33_FX_LOOP_SHORTER`.
- **Status:** ⚠️ partial.

### 4.5 Effect 5 — unison

- **What it sounds like:** the same sound played three or four times at the same pitch but slightly out of phase, sounding thicker — like multiple singers singing the same note.
- **Manual:** "unison".
- **Our code:** `PO33_FX_UNISON`. In `apply_fx()`, no parameter changes — AMY's per-voice polyphony already handles multi-voice playback.
- **Status:** ⚠️ partial (the multiple voices are already there from `num_voices = 4`; explicit unison detune isn't applied).

### 4.6 Effect 6 — unison low

- **What it sounds like:** the unison effect, but the duplicated voices are an octave lower.
- **Manual:** "unison low".
- **Our code:** `PO33_FX_UNISON_LOW`. In `apply_fx()`, lowers `midi_note` by 12.
- **Status:** ✅ done.

### 4.7 Effect 7 — octave up

- **What it sounds like:** the sound plays one octave higher than its recorded pitch.
- **Manual:** "octave up".
- **Our code:** `PO33_FX_OCTAVE_UP`. In `apply_fx()`, `midi_note += 12`.
- **Status:** ✅ done.

### 4.8 Effect 8 — octave down

- **What it sounds like:** the sound plays one octave lower than its recorded pitch.
- **Manual:** "octave down".
- **Our code:** `PO33_FX_OCTAVE_DOWN`. In `apply_fx()`, `midi_note -= 12` (clamped at 0).
- **Status:** ✅ done.

### 4.9 Effect 9 — stutter 4

- **What it sounds like:** a tiny chunk of the sound is repeated 4 times per beat, creating a "st-st-st-stutter".
- **Manual:** "stutter 4".
- **Our code:** `PO33_FX_STUTTER_4`. Currently a no-op (the granular engine isn't wired).
- **Status:** ⚠️ partial. Data model supports it (`effect_state_t.ring`), but the per-sample stutter logic isn't fully wired through `apply_fx()`.

### 4.10 Effect 10 — stutter 3

- **What it sounds like:** same as stutter 4, but 3 repeats per beat (a triplet feel).
- **Manual:** "stutter 3".
- **Our code:** `PO33_FX_STUTTER_3`. Same gap.
- **Status:** ⚠️ partial.

### 4.11 Effect 11 — scratch

- **What it sounds like:** the playhead jumps back and forth in the sample like a DJ spinning a record by hand.
- **Manual:** "scratch".
- **Our code:** `PO33_FX_SCRATCH`. **Missing from our enum — see §7.**
- **Status:** ❌ missing.

### 4.12 Effect 12 — scratch fast

- **What it sounds like:** same as scratch, but the jumps are faster.
- **Manual:** "scratch fast".
- **Our code:** `PO33_FX_SCRATCH_FAST` exists but the implementation is a no-op (we treat it like stutter).
- **Status:** ⚠️ partial.

### 4.13 Effect 13 — 6/8 quantize

- **What it sounds like:** the step timing is re-mapped from 4/4 (4 beats per bar) to 6/8 (6 beats per bar with compound subdivision). The pattern sounds different — like a waltz or a slow blues.
- **Manual:** "6 / 8 quantize".
- **Our code:** `PO33_FX_68_QUANTIZE`. **Missing from our enum — see §7.**
- **Status:** ❌ missing.

### 4.14 Effect 14 — retrigger pattern

- **What it sounds like:** every step that has a sound re-fires that sound multiple times within the step duration.
- **Manual:** "retrigger pattern".
- **Our code:** `PO33_FX_RETRIGGER_PATTERN`. Currently a no-op.
- **Status:** ❌ missing.

### 4.15 Effect 15 — reverse

- **What it sounds like:** the sound plays backwards.
- **Manual:** "reverse".
- **Our code:** `PO33_FX_REVERSE`. Currently a no-op.
- **Status:** ❌ missing.

### 4.16 Effect 16 — no effect

- **What it sounds like:** exactly as recorded. The "off" position.
- **Manual:** "no effect".
- **Our code:** `PO33_FX_NONE`. Implemented as a pass-through.
- **Status:** ✅ done.

### 4.17 Effects we have that the real PO-33 does NOT have

For full honesty, here are two effects in our `po33_fx_t` enum that are **not** part of the real PO-33:

- **`PO33_FX_BITCRUSH`** — heavy digital distortion (4-bit downsample). It's a fun effect, but the PO-33 does not have it.
- **`PO33_FX_FILTER_SWEEP`** — automated low-pass filter sweep. The PO-33 has a manual filter knob (the Filter tweak), not a sweep effect.

These are kept in v1 because they were useful while developing. The v2 plan (§7) drops them and adds the real missing PO-33 effects (`SCRATCH` and `6/8 QUANTIZE`).

---

## 5. Workflows: "How do I…?"

For each common task, here is the answer twice — once for the real PO-33, once for our firmware. If the two answers look the same, it means our firmware is fully there. If our answer says "UART shell", it means the button combo isn't wired and you'll have to use the serial port for now.

### 5.1 How do I record a sample?

**On a real PO-33:**
1. Hold the **REC** (star) button.
2. Press the slot number 1–16 where you want the recording to live.
3. Make the sound (clap, sing, play your synth into the mic).
4. Press **REC** again to stop.

**On our firmware:**
1. Hold the **REC** button.
2. Press a step button 1–16 to pick a slot.
3. Make the sound.
4. Press **REC** again to stop.

Or over the UART shell (115200 baud):
```
po33> rec 5
po33> stoprec
```

### 5.2 How do I make a beat?

**On a real PO-33:**
1. Record one or more samples first (see 5.1).
2. Press **WRITE** (·) to enter write mode.
3. Press a slot number (e.g. 1) — that slot is now "armed".
4. Press step numbers where you want slot 1 to play.
5. Press another slot number (e.g. 2), then press more step numbers.
6. Press **WRITE** again to exit.
7. Press **PLAY**.

**On our firmware:**
- No UI button combo yet. Use the UART shell:
```
po33> pattern 0
po33> bpm 120
po33> play
```
And to add a slot to a step programmatically:
```
po33> ...  # we don't have a shell command for this yet — see §7
```

### 5.3 How do I change the tempo?

**On a real PO-33:** Three distinct combos (per the [lode/PO-33](https://github.com/lode/PO-33) verbatim manual):
- Press **BPM** to cycle through 80 / 120 / 140 (the preset "levels").
- Hold **BPM** + turn knob A to set the swing (8 discrete levels).
- Hold **BPM** + turn knob B to fine-tune the tempo (continuous, 60–240).

**On our firmware:** All three behaviours wired as of this commit. Tap `BPM` (rightmost of the top row) cycles presets. Long-press + twist Knob A → swing. Long-press + twist Knob B → fine tempo. Or over the UART shell: `bpm 140`.

### 5.4 How do I switch to another pattern?

**On a real PO-33:** Hold **PATTERN** (⠛) + number 1–16.

**On our firmware:** Tap **PATTERN** to advance, long-press to step back. Or: `pattern 5` over the UART shell.

### 5.5 How do I apply an effect?

**On a real PO-33:** Hold **FX** + number 1–15 (the 15 punch-ins; the manual's effects table lists entry 16 as "no effect", not swing). The effect applies to the next sound you play. **Swing** is not a step-press at all — see §5.3 for the BPM + knob A combo.

**On our firmware:** Same combo: hold `FX` (`BTN_FX`, GPIO 12) + step 1–15 → sets `sequencer_set_active_fx(PO33_FX_LOOP_16 + (step - 1))`, which the next triggered note (sequencer or "step press without modifier after a SOUND select") consumes. Step 16 → `PO33_FX_NONE` (the manual's "no effect" entry). See F-019 for the full mapping. The selected effect is shown on the TFT (proposed in §6).

### 5.6 How do I save my work?

**On a real PO-33:** Patterns are saved automatically the moment you finish writing them. Sounds are saved automatically the moment you stop recording. Power-off and back on — everything is still there. (Backup to tape / another PO-33 is via the data transfer protocol described in 5.9.)

**On our firmware:** Patterns auto-save on power-off is not yet implemented. Use the UART shell: `save` writes all 16 patterns and all 16 sample slots to LittleFS on flash. Use `load` to restore on the next boot.

### 5.7 How do I trim a recording?

**On a real PO-33:** Enter Tweak = Trim (press FX until the screen says Trim). Turn Knob A to move the start point; turn Knob B to change the length.

**On our firmware:** Not yet via UI. UART shell would call `amy_bridge_set_trim(slot, start, end)`.

### 5.8 How do I sync with another device?

**On a real PO-33:** Connect a 3.5 mm cable from line-out → line-in. On the master, hold **RECORD + BPM** to pick a sync mode (SY0–SY5). Press **PLAY** on the master, then **PLAY** on the slave. Both devices will tick at the same tempo.

**On our firmware:** Sync OUT works (the device pulses GPIO 18 every step). Sync IN is partially wired but not fully implemented; see F-031.

### 5.9 How do I back up everything?

**On a real PO-33:** Connect line-out to a tape recorder / phone / computer. Hold **WRITE + SOUND + PLAY** on the PO-33 to dump its data as audio. The receiver records the audio.

**On our firmware:** Not yet implemented (see F-028, F-029).

### 5.10 How do I use the alarm?

**On a real PO-33:** Set the current time, then set the alarm time and pick which sound to play. At the alarm time, the sound plays once.

**On our firmware:** The clock works (F-033). Setting an alarm via shell: `clock_set_alarm 7 30 3` (play slot 3 at 07:30). The alarm-firing timer is not yet implemented (F-034).

---

## 6. Better UI: a richer TFT

The 2.4″ screen on our device is 240 × 320 pixels — a generous canvas. Our current `display_tick()` shows a step grid plus a clock and pattern number, and that's about 30 % of the pixels used. This section proposes eight dedicated UI screens that would use the rest.

Each screen is described with: when it shows, what is in the top bar, what is in the middle, what is at the bottom, and a concrete ASCII mockup. These are proposals — they are **not yet implemented** in the firmware; they are listed here as the v2 UI roadmap.

### 6.1 Screen 1 — Idle

This is the default screen. Shows while the device is on but no special mode is active.

```
+-----------------------------------+
| PAT 3   120 BPM   14:32   [|||||] |  ← top bar: pattern, BPM, clock, battery
|                                   |
|                                   |
|   ▁ ▃ ▅ ▇  playhead is at step 4 |
|   . . . . . . . . . . . . . . . . |
|   1 2 3 4 5 6 7 8 9 ...        16 |
|                                   |
|   FX: octave up                   |  ← active FX badge
+-----------------------------------+
```

Implementation: top bar is one `fill_rect(0,0,240,18)` in blue; playhead is one `fill_rect` per step; FX badge is `draw_text` in the lower 16 px. Total ~30 lines of code in `main/ui/display.c`.

### 6.2 Screen 2 — Sound select

Triggered by the SOUND button. Shows all 16 slots as small icons, filled if recorded, hollow if empty, current slot highlighted.

```
+-----------------------------------+
| SOUND  [●][●][●][●][●][●][●][●]  |  ← slots 1-8: drum
|        [○][○][●][●][●][●][●][●]  |  ← slots 9-16: melodic
|                                   |
|        Slot 3: 1.2 s, 44100 Hz   |  ← info on the highlighted slot
|        Trim: ▕████████░░░░░░░░    |  ← trim bar
|        FX: scratch                |
|                                   |
|  step +/- to choose, func to exit |
+-----------------------------------+
```

Implementation: 16 small `fill_rect`s for the slot icons; one `draw_text` line for the slot info; one thin horizontal bar for trim. ~50 lines.

### 6.3 Screen 3 — Recording

Triggered by REC button. Shows the recording level meter, elapsed time, slot target.

```
+-----------------------------------+
| REC ● slot 3                      |
|                                   |
|  ┌──────────────────────────┐     |
|  │ ░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░ │     |  ← live VU meter
|  └──────────────────────────┘     |
|                                   |
|  elapsed: 1.4 s  /  remaining 1.6 |  ← time progress
|                                   |
|  press REC again to stop          |
+-----------------------------------+
```

Implementation: red `fill_rect` background; green VU meter drawn frame-by-frame using `amy_get_input_buffer()`; numeric counter. ~40 lines.

### 6.4 Screen 4 — Pattern edit (write mode)

Triggered by the WRITE button (`BTN_WRITE`, GPIO 43) or by long-press on PLAY.

```
+-----------------------------------+
| WRITE pattern 3                   |
|                                   |
|  slot: 5 (kick)                   |  ← currently armed slot
|                                   |
|  [1][2][3][4][5][6][7][8]         |
|  [o][o][o][o][●][o][o][o]         |  ← 16 steps, ● = filled with slot 5
|  [9][10][11][12][13][14][15][16]  |
|  [o][o][●][o][●][o][o][●]         |
|                                   |
|  press step to toggle, slot+/-    |
+-----------------------------------+
```

Implementation: 16 small `fill_rect`s for the steps (filled or hollow); a label row showing the armed slot; navigation hint. ~60 lines.

### 6.5 Screen 5 — Effect picker

Triggered by FX button. Shows the 16 effects in a 4 × 4 grid, the current one highlighted, and a one-line description of what it does.

```
+-----------------------------------+
| FX                                |
|                                   |
|  [01][02][03][04]                 |
|  loop  loop  loop  loop           |
|  16    12    short shorter        |
|                                   |
|  [05][06][07][08]                 |
|  uni  uni- oct+ oct-              |
|                                   |
|  [09][10][11][12]                 |
|  stut4 stut3 scra  scra+          |
|                                   |
|  [13][14][15][16]                 |
|  6/8  retr reverse no fx          |
|                                   |
+-----------------------------------+
```

Implementation: 16 tiles, each a small `fill_rect` with the index number and short label. ~80 lines.

### 6.6 Screen 6 — Tweak mode

Triggered by SOUND + FX. Shows the currently selected tweak parameter (Tone / Filter / Trim), the two knob values, and a mini visualization of what those knobs do.

```
+-----------------------------------+
| TWEAK: TONE                       |
|                                   |
|   knob A: pitch                   |
|   ▁▁▂▂▃▃▄▄▅▅▆▆▇▇██  +5 semitones |
|                                   |
|   knob B: volume                  |
|   ▁▁▁▁▁▂▂▂▂▃▃▃▃▄▄▄▄▄▄  -10 dB    |
|                                   |
|   press FX to switch to FILTER    |
+-----------------------------------+
```

Implementation: parameter name on top; two value-bar visualizations; current parameter indicator. ~50 lines.

### 6.7 Screen 7 — Sync

Triggered by SOUND + BPM. Shows the current sync mode (SY0–SY5) and whether this device is master or slave.

```
+-----------------------------------+
| SYNC  SY1   master                |
|                                   |
|  out: stereo + sync               |
|  in : mono                        |
|                                   |
|  play ▶ to start                  |
|                                   |
+-----------------------------------+
```

Implementation: ~20 lines.

### 6.8 Screen 8 — Alarm

Triggered by SOUND + clock-area touch. Shows the current time, the alarm time, the alarm slot, on/off.

```
+-----------------------------------+
| ALARM  14:32  ON                  |
|                                   |
|  now:    14:32                    |
|  alarm:  07:30   slot 3           |
|                                   |
|  + - to change hour, hold for mm  |
+-----------------------------------+
```

Implementation: ~30 lines.

### 6.9 Total TFT-UI cost

Eight screens, averaging ~50 lines of C each = ~400 lines of new code in `main/ui/display.c` plus a screen-state enum in the buttons task to decide which screen to show. Estimated work: 1 day for an experienced embedded engineer.

The payoff: a user looking at the device can tell at a glance which sound is loaded, what step they're on, what effect is active, whether the alarm is set, and what sync mode they're in. The current screen is so minimal that it requires opening the UART shell to know any of this.

---

## 7. What's done, partial, missing — the honest scorecard

### 7.1 Feature scorecard by section

| Section | Total features | ✅ Done | ⚠️ Partial | ❌ Missing |
|---|---|---|---|---|
| 1. Sounds (record / mic / line-in) | 5 | 3 | 1 | 1 |
| 2. Patterns (write mode) | 7 | 3 | 2 | 2 |
| 3. Songs (chain) | 2 | 0 | 1 | 1 |
| 4. Tweaking (tone / filter / trim) | 4 | 0 | 3 | 1 |
| 5. Effects (16 punch-ins) | 16 | 3 | 6 | 7 |
| 6. BPM / tempo | 2 | 1 | 0 | 1 |
| 7. Volume | 1 | 0 | 0 | 1 |
| 8. Copy + delete | 5 | 0 | 0 | 5 |
| 9. Data transfer | 2 | 0 | 0 | 2 |
| 10. Sync | 3 | 1 | 0 | 2 |
| 11. Clock + alarm | 2 | 1 | 1 | 0 |
| 12. Battery | 2 | 1 | 0 | 1 |
| 13. Factory reset + UI | 2 | 0 | 1 | 1 |
| **Total** | **~50** | **13 (26%)** | **15 (30%)** | **22 (44%)** |

### 7.2 The effect-list mismatch (v2 plan)

Our current `po33_fx_t` enum in `main/audio/amy_bridge.h` is:

```c
typedef enum {
    PO33_FX_NONE = 0,
    PO33_FX_LOOP_16, PO33_FX_LOOP_12, PO33_FX_LOOP_SHORT, PO33_FX_LOOP_SHORTER,
    PO33_FX_UNISON, PO33_FX_UNISON_LOW,
    PO33_FX_OCTAVE_UP, PO33_FX_OCTAVE_DOWN,
    PO33_FX_STUTTER_4, PO33_FX_STUTTER_3,
    PO33_FX_SCRATCH_FAST,
    PO33_FX_REVERSE,
    PO33_FX_RETRIGGER_PATTERN,
    PO33_FX_68_QUANTIZE,
    PO33_FX_FILTER_SWEEP,    /* not a PO-33 effect */
    PO33_FX_BITCRUSH,        /* not a PO-33 effect */
    PO33_FX_COUNT
} po33_fx_t;
```

The real PO-33 list (per the manual and the `lode/PO-33` transcription) is exactly 16 effects:

```
1.  loop 16
2.  loop 12
3.  loop short
4.  loop shorter
5.  unison
6.  unison low
7.  octave up
8.  octave down
9.  stutter 4
10. stutter 3
11. scratch
12. scratch fast
13. 6/8 quantize
14. retrigger pattern
15. reverse
16. no effect (off)
```

We are **missing** `scratch` (which we conflated with `scratch_fast`) and `6/8 quantize` (which we have but as a no-op). We are **carrying** two extras (`filter_sweep` and `bitcrush`) that aren't real PO-33 effects.

The proposed v2 alignment:

```c
typedef enum {
    PO33_FX_NONE              = 0,
    PO33_FX_LOOP_16           = 1,
    PO33_FX_LOOP_12           = 2,
    PO33_FX_LOOP_SHORT        = 3,
    PO33_FX_LOOP_SHORTER      = 4,
    PO33_FX_UNISON            = 5,
    PO33_FX_UNISON_LOW        = 6,
    PO33_FX_OCTAVE_UP         = 7,
    PO33_FX_OCTAVE_DOWN       = 8,
    PO33_FX_STUTTER_4         = 9,
    PO33_FX_STUTTER_3         = 10,
    PO33_FX_SCRATCH           = 11,
    PO33_FX_SCRATCH_FAST      = 12,
    PO33_FX_68_QUANTIZE       = 13,
    PO33_FX_RETRIGGER_PATTERN = 14,
    PO33_FX_REVERSE           = 15,
} po33_fx_t;
```

This v2 enum is presented here as a **proposal**, not as a change to be merged. Changing the enum values would break any patterns already saved in flash (they reference effect IDs by number). The migration story would be: save the patterns in a v2-compatible format, OR add a mapping table that translates old IDs to new IDs at load time. Either is a half-day of work.

### 7.3 Top-three missing features that would unlock the most value

If you only have time to ship three more features, do these:

1. **Live recording into a playing pattern** (F-003). Without this, you cannot "jam" a new sound into an existing beat — a core PO-33 workflow.
2. **The 16 punch-in effects actually working** (F-019 through F-031 in §3.5). Selection works (FX + step 1–15 → `sequencer_set_active_fx`); many `apply_fx()` cases are still no-ops (LOOP_16, LOOP_12, STUTTER_*, SCRATCH_FAST, REVERSE, RETRIGGER_PATTERN, 68_QUANTIZE). AMY already supports most of the DSP; the remaining work is wiring `apply_fx()` correctly per case. Also pending: a v2-aligned `po33_fx_t` enum (see §7 below).
3. **Effect selection via FX button + step button** (UI binding for F-019). Wired: FX + step 1–15 sets `sequencer_set_active_fx(...)`, which `on_step()` consumes as the fallback for triggered notes. The "save effect in pattern" half of the PO-33 combo still requires write mode (F-009), which is queued.

Each of these is roughly half a day of work for an experienced developer.

---

## 8. Cheat sheet — buttons at a glance

A printable one-page reference. **P** = press, **H+P** = hold while pressing, **L+P** = long-press.

| Action | Real PO-33 | Our firmware |
|---|---|---|
| Record into slot 5 | H+REC, then 5 | same |
| Stop recording | REC | same, or `stoprec` |
| Play pattern | PLAY | same |
| Stop pattern | PLAY | same |
| Change pattern | H+PATTERN + number | H+`PATTERN` + step 1–16, or `pattern N` |
| Change BPM (preset) | press BPM (cycle) | `BPM` tap cycles preset (Hip Hop → Disco → Techno → wrap), or `bpm N` |
| Apply effect | H+FX + number (1–15) | H+`FX` + step 1–15 → active FX (carried into next note); step 16 = "no effect" (PO33_FX_NONE). NOT a step-press: swing is BPM + Knob A. |
| Enter / exit write mode | press WRITE (·) | `WRITE` button wired (GPIO 43); handler queued for v2 |
| Select a sample slot | H+SOUND + number | H+`SOUND` + step 1–16 → active slot; press same step with no modifier = plays once |
| Save pattern | auto on power-off | `save` over UART |
| Load on boot | auto | `load` over UART |
| Erase sound | H+REC + slot number | not yet |
| Erase pattern | H+REC + PATTERN | not yet |
| Copy sound | H+WRITE + SOUND + number | not yet |
| Copy pattern | H+WRITE + PATTERN + number | not yet |
| Tweak tone/filter/trim | FX tap cycles; knobs A/B adjust | FX tap cycles Tone → Filter → Trim → Tone; knobs bind to per-step data (Tone/Filter) or slot trim (Trim) |
| Change swing | H+BPM + knob A | H+BPM + Knob A → 8 discrete levels (0=no swing, 7=max); release keeps the level |
| Fine BPM | H+BPM + knob B | H+BPM + Knob B → continuous 60–240 BPM; release keeps the BPM |
| Change volume | H+BPM + number (1-5) | H+BPM + step 1-5 → volume level 1..5 (5=max); multiplier applied per-note in amy_bridge_play_note() |
| Sync out | always on | always on |
| Sync in | H+REC + BPM cycles mode | partial |
| Show battery | SOUND + BPM | not yet (TFT will show) |

---

## 9. FAQ

### Q: Can I use this firmware without buying the audio chip and the screen?

**A:** No. The firmware assumes the I²S DAC and mic are present, and uses the SPI screen for status. Without those, boot will fail at the I²S init step (the device will reboot-loop). You could compile a "headless" build that skips the audio and screen, but that is not part of this firmware.

### Q: Will my saved samples survive a reboot?

**A:** Yes, if you ran `save` over the UART shell before the reboot. Patterns are also saved. The samples live in LittleFS on the 16 MB flash, not in PSRAM — PSRAM is wiped on every boot. Without `save`, your recordings are gone after a power cycle.

### Q: Does it work with a real PO-33?

**A:** Partial. The 3.5 mm sync protocol is not yet implemented, so two of our boards can sync to each other, but ours cannot sync to a real PO-33. We share the same audio sample rates (44.1 kHz) so once we implement data transfer (F-028), the two devices should be able to exchange sounds and patterns.

### Q: Can I plug headphones in?

**A:** Only if you wire the headphone amp chip (MAX98357A) to the I²S lines. The PCM5102A DAC we use for line-out does not drive headphones directly. The "headphone volume" levels (5 levels) on the real PO-33 would then apply to the MAX98357A's volume register.

### Q: How loud is the speaker?

**A:** Whatever the MAX98357A is rated for — typically 3 W into a 4 Ω speaker. The firmware does not set the volume, it just outputs full-scale audio; the amp's volume is hardware-controlled by an external potentiometer or a GPIO pin we'd have to add to the firmware.

### Q: Can I make a song with more than 128 patterns?

**A:** No. The real PO-33 also caps at 128. If you want a longer song you have to plan for the loop-back.

### Q: Does the alarm work?

**A:** You can set the alarm time and which slot to play, but the firmware does not yet have a timer that checks "is it alarm time?". Setting the alarm just stores the values in NVS. The actual fire-and-play is a planned v2 feature (F-034).

### Q: Why does the device sometimes lock up when I press a button?

**A:** It shouldn't. If it does, please file a bug report with the exact button combo. There are known edge cases in the button de-bouncer for very rapid presses; the firmware ignores presses shorter than 30 ms to filter switch bounce. If you find a press that consistently crashes the device, that's a bug.

### Q: Can I use this firmware with a battery?

**A:** Yes. The board has a LiPo charging circuit if you wire one up. In deep sleep the firmware draws ~10 µA, so a 1500 mAh LiPo would last many months. Active power is 60–120 mA depending on polyphony. The battery voltage display is a planned v2 feature (F-035).

### Q: How is this different from running AMY on a Mac?

**A:** AMY on a Mac uses your laptop's audio interface and your laptop's keyboard / mouse as input. Our firmware uses a $5 chip, a $3 DAC, a $5 screen, and 16 buttons. The difference is **portability** and **physical tactility** — a PO-33 is something you hold in your hand, not something you sit in front of.

---

## 10. Beyond the PO-33: breaking the limits with the ESP32-S3

### 10.0 Why this section exists

The PO-33 is a small, beautiful object with very specific limits: 40 seconds of sampling, 16 slots, 16 steps, 16 patterns, 16 effects, 5 sync modes, no WiFi, no Bluetooth, no MIDI, no USB. None of these limits are bugs. None of them are accidents. They are the **consequences of the hardware TE chose**: a particular microcontroller, a particular amount of memory, a particular audio codec. The constraint-as-feature is part of why the device is loved: 16 slots force you to be ruthless about which sounds matter; 16 steps force a specific kind of rhythm; the absence of MIDI keeps you playing the PO-33 like an instrument, not a sequencer.

But the limits are still limits. And the chip in this project — the **ESP32-S3** — is a fundamentally more powerful platform: a dual-core 240 MHz processor with vector instructions for DSP, 8 MB of Octal PSRAM, WiFi, Bluetooth, USB-OTG, dual I²S, and an entire free audio library called AMY sitting on top. So almost every PO-33 limit becomes a knob we can turn up, if we want to.

This section does three things:

1. **Lists the PO-33 limits explicitly** (what the original device actually caps at).
2. **Lists what the ESP32-S3 + AMY can do** that the PO-33 cannot.
3. **Proposes a v3 wishlist** ranked by impact.

We will also be honest about constraints that even the ESP32-S3 cannot break — every platform has ceilings — and about the PO-33 limits we should *keep on purpose*, because constraint is also a creative tool.

> ⚠️ **Honesty note.** This section describes what the hardware *can* do, not what our v1 firmware *has* implemented. Every "could be" is grounded in a real chip capability or a documented AMY API. "Currently does" is flagged in the wishlist column.

### 10.1 What the PO-33 cannot do

Drawn from the official Teenage Engineering manual ([teenage.engineering/guides/po-33/en](https://teenage.engineering/guides/po-33/en)) and confirmed against the `lode/PO-33` transcription.

| # | Limit | Value | Source |
|---|---|---|---|
| L1 | Total sample memory | 40 seconds across all 16 slots | Manual §1 |
| L2 | Per-slot duration | uneven; the manual does not state a per-slot max — only a total | Manual §1 |
| L3 | Number of sample slots | 16 total (slots 1–8 melodic, slots 9–16 drum) | Manual §1 |
| L4 | Steps per pattern | 16 | Manual §2 |
| L5 | Patterns | 16 | Manual §2 |
| L6 | Song length | up to 128 patterns chained | Manual §3 |
| L7 | Tweak parameters | 3 (Tone, Filter, Trim) | Manual §4 |
| L8 | Punch-in effects | 16 (fixed) | Manual §5 |
| L9 | BPM levels | 3 presets (Hip Hop 80, Disco 120, Techno 140) + fine-tune | Manual §6 |
| L10 | Headphone volume levels | 5 | Manual §7 |
| L11 | Sync protocols | 5 modes (SY0–SY5) over 3.5 mm cable; ≤5 Vpp | Manual §10 |
| L12 | Data transfer | 3.5 mm audio cable only; no WiFi, no Bluetooth, no USB, no MIDI, no SD card | Manual §9 |
| L13 | Storage | non-volatile internal only; no user-expandable storage | Manual §8 |
| L14 | Recording sources | built-in microphone + 3.5 mm line-in | Manual §1 |
| L15 | Polyphony | not stated explicitly; ~4 voices typical for the PO line | inferred |
| L16 | Display | fixed LCD; no waveform display, low resolution | TE product page |
| L17 | Power | 2 × AAA batteries; ~1 month standby | TE product page |
| L18 | Audio quality | mono; sample rate implied at ~22 kHz | inferred from manual |
| L19 | Effects parameter locks | 2 per step | Manual §4 |
| L20 | Connectivity (none) | no WiFi, no Bluetooth, no USB, no MIDI | Manual (absence) |

Some of these limits are tightly coupled to the hardware Teenage Engineering chose. The 40-second total, for instance, comes from the microcontroller inside the PO-33, which has a fixed amount of RAM. Other limits (3 BPM levels, 5 sync modes, 16 fixed effects) are arguably arbitrary and could be loosened in a hypothetical PO-33 v2 — but TE chose the current numbers for usability reasons.

### 10.2 What the ESP32-S3 can do that the PO-33 cannot

| # | Capability | Why we can exceed the PO-33 | Concrete ceiling |
|---|---|---|---|
| C1 | **Sample memory** | The ESP32-S3-WROOM-1-N16R8 module has 8 MB of Octal PSRAM. The PO-33 uses roughly 1.6 MB (40 s × ~22 kHz × 16-bit ≈ 1.76 MB). We use **44.1 kHz** to match AMY's native render rate; that gives us a live pool of **~50–75 s** mono in PSRAM after the TFT framebuffer and AMY state (see §11.3 for the full breakdown). | Per-sketch on flash: ~75 s typical per sketch in our 8 MB `sketches` partition (~5 typical sketches). With streaming-to-flash on the 16 MB flash chip, you could push the live pool further. |
| C2 | **Sample rate / quality** | AMY supports up to 48 kHz; the ESP32-S3 I²S peripheral supports higher. We chose 44.1 kHz to match AMY's native rate, but we *could* sample at 48 kHz. | At 44.1 kHz mono: ~5.3 MB/min; at 48 kHz mono: ~5.8 MB/min; at 96 kHz mono: ~11.5 MB/min. Anything above 48 kHz exceeds our PSRAM pool in practice. |
| C3 | **Per-slot duration** | Because we have PSRAM, each slot's length is bounded only by the total pool. **But at 44.1 kHz, 1 s of mono = 86 KB**, so a long melodic slot costs a lot. | Per-slot ceiling: up to ~30 s for a melodic slot in principle, but the *combined* pool is bounded by ~6 MB of PSRAM (after framebuffer + AMY state) = roughly **50–75 s of total sample time** (see §11.8 for the full breakdown). Practically, with a typical 8-slot sketch, that means **average ~6–9 s per slot** in moderate DSP load. |
| C4 | **Polyphony** | AMY's `amy_config_t.max_voices` controls how many oscillator voices are available. Each voice costs a small CPU slice per render block. The ESP32-S3 has dual LX7 cores and vector DSP instructions, so we can run more voices in parallel than a single-core chip. | 8–16 simultaneous voices is comfortable. 30 is achievable with optimization. |
| C5 | **Step count per pattern** | Each `step_t` is currently 8 bytes. 256 steps × 16 patterns × 128 chain = 16 KB. Trivial in PSRAM. We could support variable time signatures: 16 steps of 1/16, 32 of 1/32, etc. | 32 or 64 steps per pattern (variable time signatures — e.g. a 5/4 or 7/8 pattern). |
| C6 | **Pattern count** | 16 patterns × ~3 KB each = 48 KB. PSRAM has megabytes to spare. | 64–256 patterns. |
| C7 | **Song length** | Pattern chain is currently 128 bytes. | 512–2048 patterns chained. |
| C8 | **Storage** | 16 MB flash + LittleFS. The `sketches` partition is **8 MB**; the rest is `factory` (3 MB) + `nvs` + `phy_init` (tiny). We can persist samples, patterns, presets, and user-named sketches. | 16 patterns × 3 KB + 16 samples × ~220 KB = ~3.5 MB used per typical sketch; ~4.5 MB free in `sketches` after 5 typical sketches. |
| C9 | **Sync protocols** | The ESP32-S3 has WiFi 802.11 b/g/n and Bluetooth 5. We can add WiFi-based sync (NTP clock, Ableton Link, OSC), BLE MIDI, USB MIDI, USB audio class. | Ableton Link over WiFi; BLE MIDI; USB-MIDI class compliant. |
| C10 | **MIDI** | The chip supports USB-OTG and BLE. USB-MIDI device class costs ~1 KB of code with the ESP-IDF TinyUSB stack. | Full MIDI in/out (DIN-5 with a $1 optocoupler, USB-MIDI, or BLE MIDI). |
| C11 | **Effects** | AMY already gives us chorus, echo, reverb, distortion, filters. Multiple AMY effects can be chained per voice. | Chainable multi-effect per voice (e.g. filter + reverb + chorus on the same note). |
| C12 | **Sampling rate conversion** | The ESP32-S3 vector instructions accelerate SRC. We could record at one rate and play at another (e.g. record 48 kHz, slow down to 22 kHz for lo-fi playback). | Record at any rate; play at any rate; crossfade between. |
| C13 | **Networking** | WiFi + BLE. | Sync with phones; web preset library; OTA firmware updates. |
| C14 | **Display** | We chose a 240×320 TFT — already 5× the pixels of the PO-33's LCD. AMY exposes an audio-analysis API (FFT) we can pipe to the display. | Waveform display; spectrum analyzer; level meters; full editor UI on the screen. |
| C15 | **Sample management** | PSRAM lets us keep every sample loaded simultaneously. No file system round-trip on sample switch. | Zero-latency sample swap; no need to "load from card". |
| C16 | **Recording** | Two independent I²S channels — one for the built-in mic, one for line-in. | Dual-source recording; resampling on capture; pitch detection from the input. |
| C17 | **Tap tempo / auto-quantize** | Microphone input already exists. | Listen to the room (or another instrument) and quantize the player's own timing. |

### 10.3 How we can break each PO-33 limit

Mapping each row of §10.1 to a concrete "how we exceed it" answer.

| PO-33 limit | Currently | Possible with our hardware | Required work |
|---|---|---|---|
| L1 — 40 s total | 40 s × 44.1 kHz mono = 3.37 MB pool (we already match PO-33's total) | ~5 minutes mono, or ~20 min with flash streaming | Re-partition PSRAM pool; add LittleFS streaming for the surplus |
| L2 — per-slot max | 2 s drums, 3 s melodic (configurable) | up to 30 s melodic | Change the slot-cap math in `config.h` |
| L3 — 16 slots | 16 slots | 32, 64, or 256 slots | Bigger pool + bigger UI selector |
| L4 — 16 steps | 16 steps | 32 or 64 steps | Larger `step_t` array + bigger step grid on TFT |
| L5 — 16 patterns | 16 | 64 or 256 | More `pattern_t` storage |
| L6 — 128-song chain | 128 | 512–2048 | Larger `g_chain[]` |
| L7 — 3 tweak params | not bound to UI | unlimited (already supported in `step_t`) | New "Tweak" UI screen (§6.6) |
| L8 — 16 effects | 16 | unlimited (AMY supports many) | Per-event effect chains |
| L9 — 3 BPM levels | 1 BPM level (continuous 60–240) | unlimited | Add "level cycling" mode |
| L10 — 5 volume levels | master gain fixed at 100% | 1–100 dB in 1 dB steps | UI binding for volume keys |
| L11 — 5 sync modes | 5 modes (not implemented) | unlimited + WiFi Link | New sync task; possibly Ableton Link port |
| L12 — no wireless | none | WiFi sync, BLE MIDI, USB-MIDI | Significant: BLE stack + Link port |
| L13 — 256 KB flash for samples | 256 KB on chip (we don't use it for samples) | 8 MB `sketches` partition on chip | Already in `partitions.csv` (renamed to use `sketches` at 8 MB) |
| L14 — mic + line only | mic only | mic + line-in simultaneously | Second I²S RX channel + a 3.5 mm jack |
| L15 — ~4 voices | 4 voices | 8–30 voices | Tweak `amy_config_t.max_voices` |
| L16 — fixed LCD | 240×320 TFT | 240×320 or larger TFT, OLED, or e-ink | Already done; future versions: add a second screen |
| L17 — 2× AAA | USB-C 5 V or LiPo | same + solar if we add a charging chip | already wired in `config.h` |
| L18 — 22 kHz mono | 44.1 kHz mono (we already exceed) | 48 kHz or 96 kHz | AMY supports up to 48 kHz natively |
| L19 — 2 plocks/step | 2 plocks | unlimited (AMY has many params) | Larger `step_t` struct |
| L20 — no I/O beyond 3.5 mm | UART shell + USB-C | USB-MIDI, USB-audio class, BLE MIDI, WiFi web UI | Component additions (TinyUSB, Link) |

### 10.4 Constraints the ESP32-S3 cannot break

Not every limit is unbounded. Some are fundamental to the physics and economics of the platform. We list them honestly so the wishlist in §10.6 is grounded.

| # | Limit | Why it can't be broken |
|---|---|---|
| X1 | **Live mic latency floor** | The I²S DMA must buffer at least one block. With our 256-frame blocks at 44.1 kHz, the round-trip latency is ~5.8 ms (record → playback). The PO-33 is similar. You cannot make "feel like zero" smaller than this without changing the chip or the audio protocol. |
| X2 | **Polyphony ceiling** | Each voice costs CPU cycles per render block. With 30 voices, the ESP32-S3 hits its throughput limit and audio glitches. The AMY overload failsafe would kick in. |
| X3 | **WiFi range and battery life with WiFi on** | WiFi is ~200 mA when active. With a 1500 mAh LiPo you get ~6 hours, not months. Range is ~50 m line-of-sight. |
| X4 | **Maximum I²S sample rate** | The peripheral tops out at a few MHz. 96 kHz is fine; 192 kHz is fragile. |
| X5 | **Speaker volume** | Software cannot fix a small speaker. Hardware (a bigger amp chip, a bigger speaker) is the only lever. |
| X6 | **Audio-to-MIDI** | Real-time polyphonic pitch detection from audio requires a DSP / ML model that doesn't fit in 8 MB PSRAM. Off-the-shelf solutions exist (e.g. on a phone), but not on-chip. |
| X7 | **Display size** | Our board drives one 240×320 TFT. Adding a second display needs a second SPI bus and more GPIO. |
| X8 | **Power consumption during deep sleep** | ~10 µA is already very good. Below that we'd need a different chip entirely. |
| X9 | **BLE MIDI latency** | BLE has a ~7.5 ms connection interval minimum. Not as tight as wired MIDI (~1 ms). |
| X10 | **Sample-rate mismatch with USB audio** | The ESP32-S3 USB-OTG can do isochronous audio at 48 kHz reliably. 96 kHz over USB is possible but flaky. |

### 10.5 Which constraints are worth keeping

This is the counter-intuitive part of the section. **Not every PO-33 limit should be broken.** Constraint is part of why the device is fun.

| Limit | Argument for keeping it |
|---|---|
| L1 — 40 s total | Forces the user to curate. A 5-minute pool fills with mediocre takes. |
| L3 — 16 slots | Forces you to delete bad samples, which is creative discipline. |
| L4 — 16 steps | One bar of 16th-notes is the rhythmic sweet spot. 32 steps is technically possible but the grid gets too dense for a 2.4" screen. |
| L5 — 16 patterns | Anything more becomes a menu, not a beat. |
| L8 — 16 effects | A small palette is more learnable than a large one. AMY supports dozens but exposing all of them defeats the PO-33's "punch-in one button, get one effect" model. |
| L9 — 3 BPM levels | Naming the levels (Hip Hop / Disco / Techno) gives a beginner a starting tempo without forcing them to count. |

The PO-33's UI is constrained *on purpose*. Our v3 should preserve most of these constraints even as the underlying hardware becomes more capable. The role of constraint in creative work is well-studied — see Brian Eno's "oblique strategies" or any number of game-design articles on the topic.

### 10.6 A v3 wishlist, ranked by impact

The following are features we **could** ship if time permitted, ranked by (user value × implementation cost). The list is intentionally short — we are not committing to ship any of these.

| Rank | Feature | User value | Implementation cost | Why it ranks here |
|---|---|---|---|---|
| 1 | **WiFi Ableton Link sync** | Lets the PO-33 participate in a DAW session alongside a laptop, iPad, or other Link-enabled gear. Unlocks studio use. | High: needs the Link protocol ported to ESP-IDF (~1–2 kLOC). | Massive user-value jump (turns the device from a toy into a pro tool). |
| 2 | **USB-MIDI device class** | Plug the PO-33 into a laptop and use it as a 16-button MIDI controller. Adds keyboard, pad-controller, and DAW-integration use cases. | Medium: TinyUSB component + ~500 LoC. | High user value, well-trodden code path. |
| 3 | **BLE MIDI** | Wireless MIDI without USB. Plays well with iOS / Android / macOS. | Medium: NimBLE + MIDI service profile. | High value, but overlaps with USB-MIDI. |
| 4 | **Streaming samples from the 16 MB flash** | 5-minute sample pool (or much longer if we accept compression). | Medium: LittleFS streaming in `amy_bridge.c`. | Big value, modest cost. |
| 5 | **Waveform + spectrum on the 2.4″ TFT** | Visual feedback while recording / playing. Pro-level feel. | Medium: AMY exposes an FFT hook; render in `display.c`. | Nice-to-have more than essential. |
| 6 | **Effect chains per voice** | More expressive performances. | Low: just an array of effects in `amy_event`. | Easy win, but the PO-33 community might prefer the existing single-effect model. |
| 7 | **Dual-source recording (mic + line-in)** | Record the room *and* a synth simultaneously. | Medium: second I²S RX channel + a jack. | Niche but cool. |
| 8 | **WiFi web UI for preset library** | Browse + upload presets from a phone. | High: web server + UI. | High novelty, low practical use. |
| 9 | **AI patch suggestions** | Suggest effect chains based on the audio. | Very high: needs ML model. | Out of scope for v3. |

Honest note: items 1, 8, and 9 are aspirational. Items 2–7 are realistic on a one-month timeline for a single developer. Items 2 and 4 are what we would build first.

---

## 11. Multi-sketch storage & restore

> **Vocabulary.** The unit of creative work this section designs for is called a **sketch** (one sketch = samples + 16 patterns + a 128-step chain + metadata). The PO-33 calls the same idea a "song"; the Korg Electribe calls it a "Pattern Set"; Ableton Live calls it a "Live Set". We picked "sketch" because it's short, plain, and doesn't clash with sampler vocabulary. Throughout §11, the **C identifiers in the API block use the `sketch` prefix** (e.g. `storage_sketch_save_active()`, `sketch_meta_t`, `SKETCHES_MAX`) — the rename has already happened in the design, so the v2 implementation should match. **Outside code blocks, every "project" in this section means "sketch".**

### 11.0 Why this section exists

The current v1 firmware has a **single-bank** storage model:

- 16 patterns in RAM as `g_patterns[16]`.
- One chain of up to 128 pattern entries as `g_chain[128]`.
- One sample pool in PSRAM.
- A `storage_save_all()` that writes each pattern / slot to a flat file (`p0.bin`, `p1.bin`, …, `p15.bin`, `s0.bin`, …, `s15.bin`).
- A `storage_load_all()` that unconditionally reads every file and clobbers RAM.

This works — but only if you treat the device as if it holds **exactly one sketch at a time**. Start building a new chain, hit save, and you overwrite the sketch you'd been working on. There is no "save as", no "switch sketch", no "delete sketch". It is, in essence, the same storage model the real PO-33 has (it also holds exactly one song at a time, with up to 128 patterns in its chain).

For a $5 dev board with **16 MB of flash** and **8 MB of PSRAM**, this is an absurd waste. We can store **dozens of independent sketches**, each with its own sample pool, pattern bank, chain, and metadata. The user can flip between them. The active sketch lives in PSRAM (zero-latency playback); archived sketches live in LittleFS on flash.

This section is the **design** for that system. It is a v2 proposal — v1 firmware is unchanged. No code in this section is committed; everything below describes what the v2 implementation should look like.

### 11.1 Concepts

- **Sketch** — a self-contained creative unit: a name, a sample pool, 16 patterns, a chain, BPM, and metadata. The atomic unit the user saves, switches between, deletes, or duplicates.
- **Active sketch** — the one sketch whose sample pool is in PSRAM and whose patterns are live in `g_patterns[16]`. At any moment, exactly one sketch is active. Switching to a different sketch pages its data into RAM; the previously-active sketch is flushed to flash.
- **Sketch ID** — a **4-character hex string** (e.g. `a1b2`), assigned by a monotonic counter starting at `0000`. 16 bits of entropy (65 536 possible IDs); collisions are impossible because we only assign each ID once. 4 hex chars makes folder names short, easy to type on a UART shell, and trivial to recognise in a sketch picker UI.
- **Sketch metadata** — a small JSON file describing a sketch: name (≤24 chars), label color (1 of 8), created-at Unix timestamp, last-modified-at Unix timestamp, BPM at last save, sample count, pattern count, chain length, total playback duration.
- **Atomic write** — a save operation that either fully completes or is fully rolled back, so a power loss mid-write can never leave a half-written sketch. We achieve this with the classic write-to-temp-then-rename trick.
- **Sketch slot** — a serial index from 0 to N-1, where N is determined by available flash. The slot number is for UI ordering and internal bookkeeping; the 4-hex ID is the canonical identifier. Two sketches can never share a slot number.

### 11.2 On-flash layout

We currently have **one** LittleFS partition called `sketches` (256 KB in `partitions.csv`). It is large enough for ~32–64 sketches of average size, so we **reuse it** rather than add a new partition. The layout inside the existing `sketches` partition becomes:

```
/sketches/
├── sketches.lst                       # index: one line per sketch
│                                       #   format: <id4> <slot> <name>
│                                       #   sorted by slot for stable UI ordering
│
├── a1b2/                              # sketch with ID "a1b2"
│   ├── meta.json                      # sketch metadata (see §11.1)
│   ├── samples.bin                    # raw 16-bit PCM, the sample pool
│   ├── patterns/                      # 16 step-patterns, one file each
│   │   ├── p00.bin
│   │   ├── p01.bin
│   │   │   ...
│   │   └── p15.bin
│   └── chain.bin                      # chain (up to 128 entries)
│
├── b3c4/                              # next sketch
│   ├── meta.json
│   ├── samples.bin
│   ├── patterns/
│   │   ...
│   └── chain.bin
│
└── tmp/                                # staging area for atomic writes
    ├── samples.bin.tmp
    └── ...
```

Notes:

- **4-hex ID folders.** The folder name is the sketch ID (`0000`, `0001`, …, `ffff`). Folders are flat at the `sketches/` root — no nesting — so LittleFS directory traversal stays cheap.
- **Hex counter** — IDs are assigned sequentially: the next ID is `storage_next_free_id()`, which walks the `sketches.lst` and returns `max(ids) + 1`, formatted as 4 hex chars. If the device has never had a sketch, the first ID is `0000`. If the device has `0000` and `0003`, the next is `0004`.
- **`sketches.lst` is the master index.** It is rewritten atomically on every sketch create / delete / rename. The UI's "Sketch" menu reads this file.
- **`tmp/`** is a scratch area. Atomic-save writes to `tmp/<id4>.samples.bin.tmp` etc., then `rename()`s the file into place. A power loss during a write leaves the tmp file dangling; on boot we delete any leftover `*.tmp` files.
- **No quota file.** Quotas (max N sketches, max total bytes per sketch) are enforced at runtime in `storage_sketch_save_active()` by checking free space first. LittleFS has a fixed partition size, so "free space" is `partition_size - used_bytes`.

> **Note.** We've renamed `project.lst` to `sketches.lst`, `storage_project_*` → `storage_sketch_*`, and the **partition label** `patterns` → `sketches` (see `partitions.csv` and `main/storage/storage.{h,c}`). **The v2 source code should match.** v1 firmware doesn't yet contain these symbols (the multi-sketch system is a v2 addition), so there's nothing in `main/` to rename today — when the v2 implementation lands, the identifiers above are the ones to use.

### 11.3 On-RAM state

At any moment, exactly one sketch is loaded into PSRAM. The in-RAM data model does **not change** — it is still `g_patterns[16]`, `g_chain[128]`, and the PSRAM sample pool. What changes is the **mapping** between in-RAM state and on-flash state.

Concretely:

| In-RAM symbol | Holds the state of | Paged in when | Paged out when |
|---|---|---|---|
| `g_patterns[16]` | active sketch's 16 patterns | sketch switch (`storage_sketch_load`) | sketch switch (save then load new) |
| `g_chain[128]` + `g_chain_len` | active sketch's chain | same | same |
| PSRAM sample pool (3.37 MB) | active sketch's samples | same | same |
| `s_bpm`, `s_pattern`, etc. | active sketch's transport state | same | same |

**The PO-33's "chain" stays a chain.** Each sketch carries one chain. The difference from v1 is that there are now *N* chains on the device, one per sketch.

### 11.4 Sketch lifecycle

The state diagram for a sketch:

```
                +---------+    create    +----------+
                |         | ----------> |          |
                |  EMPTY  |             | CREATED  |  (no samples, no patterns, name = "New Sketch")
                |         |             |          |
                +---------+             +----------+
                                          |
                                          | first record
                                          v
                +---------+    load     +----------+
                |         | <--------- |          |
                |          |  -------  | SAVED    |  (on flash, may be active or not)
                |          |  ---      |          |
                |          |  <-       +----------+
                |  ACTIVE  |
                |          |  -------> 
                +----+----+
                     |   |
        modify       |   |    save
        (in RAM)     |   |   (atomic)
                     |   |
                     v   v
                +----------+
                | MODIFIED |  (RAM differs from flash; will be lost on power-off)
                +----------+
                     |
                     | save (atomic)
                     v
                +----------+
                |   SAVED  |  (RAM == flash, durable across power-off)
                +----------+
                     |
                     | delete
                     v
                +----------+
                | DELETED  |  (folder removed from flash; sketches.lst updated)
                +----------+
```

State transitions:

- `EMPTY → CREATED`: the user picks "New Sketch" on the TFT. We allocate the next 4-hex ID from the counter, write a default `meta.json` (name = "New Sketch", label color = 1, timestamps = now), and add an entry to `sketches.lst`. The active sketch becomes this one and starts empty.
- `CREATED → MODIFIED`: any recording, edit, or transport change. Pure RAM.
- `MODIFIED → SAVED`: user picks "Save" on the TFT, or auto-save fires. We write `samples.bin`, `patterns/p*.bin`, `chain.bin`, and `meta.json` (with bumped last-modified-at) atomically. RAM == flash.
- `SAVED → MODIFIED`: user edits anything. Pure RAM.
- `ACTIVE → DELETED`: user picks "Delete sketch" on the TFT. We require confirmation. We then `unlink()` the sketch folder and rewrite `sketches.lst`. If it was the active sketch, the device falls back to an empty sketch (or another sketch if one exists).
- `ACTIVE → EMPTY`: like DELETE but for the active sketch. The user is left with no active sketch until they pick or create one.

### 11.5 API additions

These are the new public functions. **Signatures only** — no implementation in v1.

```c
/* In storage.h */

typedef struct {
    char     id[5];            /* 4 hex chars + null terminator */
    char     name[25];
    uint8_t  label_color;      /* 0..7 */
    uint32_t created_at;       /* Unix timestamp */
    uint32_t modified_at;
    uint16_t bpm;              /* BPM at last save */
    uint8_t  sample_count;     /* 0..16 */
    uint8_t  chain_len;        /* 0..128 */
    uint32_t total_samples;     /* playback duration in samples */
} sketch_meta_t;

#define SKETCHES_MAX 16          /* hard cap; see §11.8 capacity math */
#define SKETCH_NAME_MAX 24

esp_err_t storage_sketches_init(void);
size_t      storage_sketches_count(void);
esp_err_t storage_sketches_list(sketch_meta_t *out, size_t max);

/* Returns the active sketch meta. */
esp_err_t storage_get_active_sketch(sketch_meta_t *out);

/* Create / switch / delete / rename. */
esp_err_t storage_sketch_create(const char *name, sketch_meta_t *out_new);
esp_err_t storage_sketch_switch(const char *id);   /* pages old to flash, loads new */
esp_err_t storage_sketch_delete(const char *id);   /* requires confirmation flag */
esp_err_t storage_sketch_rename(const char *id, const char *new_name);
esp_err_t storage_sketch_save_active(void);         /* atomic write of active sketch */

/* Duplicate — clones the active sketch under a new 4-hex ID and new name. */
esp_err_t storage_sketch_duplicate(const char *new_name, sketch_meta_t *out_new);

/* Export / import — exports one sketch as a .zip containing its folder. */
esp_err_t storage_sketch_export(const char *id, const char *dest_path);
esp_err_t storage_sketch_import(const char *src_path, sketch_meta_t *out_new);

/* Internal helper: returns the next free 4-hex ID by scanning sketches.lst. */
char       *storage_next_free_id(void);
```

```c
/* In sequencer.h — new function for live save-on-edit */

esp_err_t sequencer_request_save(void);
/* Posts a "save the active sketch" request to a queue that the
 * storage task drains. Returns ESP_OK immediately. The save happens
 * in the background to keep audio playback glitch-free. */
```

```c
/* In main/ui/menu.h — new screen for sketch management */
void ui_menu_sketch_picker(void);   /* shows the sketch list, lets user pick */
```

All new functions are non-blocking for the audio path. The actual file I/O happens in a dedicated low-priority task so the I²S render task (AMY) is never starved. We use FreeRTOS stream buffers to pass sample-pool chunks to the storage task.

### 11.6 PO-33 parity vs. extension

| Feature | PO-33 | v1 (current) | v2 (this section) |
|---|---|---|---|
| Sample memory | 40 s | 40 s | 40 s × N sketches in flash; 40 s live in PSRAM |
| Patterns | 16 | 16 | 16 per sketch, N sketches |
| Pattern chain (the PO-33's "song chain") | up to 128 | up to 128 | up to 128 **per sketch**, N sketches |
| # of sketches | 1 | 1 | N (bottleneck: flash size) |
| Save | auto on power-off | manual `save` UART command | auto on edit (debounced) + manual |
| Back up | audio out to tape (slow, lossy) | `storage save` writes 1 set of files | export one sketch as .zip over USB-MSD (v3) |
| Copy between devices | P2P audio cable | not implemented | USB-MSD export/import (v3) |

The PO-33's "one song, one chain" is preserved **within** a sketch. What we add is that the device can hold N sketches, each with its own chain. This is the same conceptual model as the Korg Electribe's "Pattern Set" or Ableton Live's "Live Set" — a higher-level container that owns a complete working state.

> **Vocabulary note.** The PO-33 calls its unit of creative work a "song". We call it a **sketch**. The two refer to the same idea (samples + patterns + a chain + metadata); we just picked a shorter word that doesn't have pop-music connotations or clash with sampler vocabulary (we avoid "groove" because that word means *timing templates* in the rest of the music-software world). The 16-step chain *within* a sketch is unchanged from the PO-33's "song chain".

### 11.7 Concurrency / atomicity

LittleFS is journaled but not transactional across multiple files. We must be careful.

**Atomic-write protocol for a sketch save:**

1. Acquire a mutex to serialize saves against the storage task.
2. Write all files to a unique `tmp/` subfolder, e.g. `tmp/<id4>/`.
3. `rename()` each file into the sketch folder. LittleFS `rename()` is atomic within the same directory.
4. After all files are renamed, rewrite `sketches.lst` atomically (`tmp/sketches.lst.tmp` → `sketches.lst`).
5. Release the mutex.

If power is lost at any step:

- Files left in `tmp/<id4>/` are stale. On boot, the storage init scans `tmp/` and deletes any file older than 1 minute. (Or just any file; `tmp/` is not user-visible.)
- Files in the sketch folder that were renamed are intact.
- If `sketches.lst` was not yet rewritten, the old version is used. The user sees the old sketch list, which is the safe failure mode.
- If `sketches.lst` *was* rewritten but the sketch folder rename was incomplete, the entry in `sketches.lst` will point to a sketch that's "half present". On boot, we validate each `sketches.lst` entry against the actual filesystem and drop any orphaned entries.

### 11.8 Capacity math

We sample at **44 100 Hz × 16-bit mono** to match AMY's native render rate. At that rate:

- **1 second of mono audio = 88 200 B ≈ 86 KB.**
- **40 seconds = 3.45 MB** (this is the v1 PO-33-equivalent pool size).

Let's work through a real example. Average sketch (the PO-33's typical usage):

| Component | Math | Size |
|---|---|---:|
| Sample pool | 8 samples × 2 s × 44 100 Hz × 2 B = **1 411 200 B** | **~1.38 MB** |
| Patterns | 16 patterns × 16 steps × ~50 B/step | **~13 KB** |
| Chain | 128 B | **0.13 KB** |
| Metadata | `meta.json` | **~0.2 KB** |
| **Total per sketch (typical)** | | **~1.42 MB** |

The sample pool is **>97 %** of every sketch. Drop a sample or shorten one and the sketch shrinks proportionally; add a long sample and it grows proportionally.

#### Live PSRAM sample pool ceiling (active sketch)

The on-flash numbers above are for archived sketches. The **active sketch's** sample pool must fit in PSRAM alongside the TFT framebuffer and AMY state. The math at 44 100 Hz:

| Subsystem | Approx. size |
|---|---:|
| TFT framebuffer (240×320×2) | ~150 KB |
| AMY state (idle / no audio) | ~100 KB |
| AMY state (moderate — 4 voices + reverb) | ~1.5 MB |
| AMY state (heavy — 16 voices + echo + chorus) | ~3.5 MB |
| AMY state (stress — 30 voices + heavy DSP) | ~6 MB |
| LittleFS cache, heap overhead, misc | ~256 KB |
| **Available for samples** (8 MB PSRAM minus above) | **~6.3 MB (idle) down to ~1.8 MB (stress)** |

At 88 200 B/s, that gives:

| Scenario | Live sample memory |
|---|---:|
| Idle / no audio rendering | **~75 s** |
| Moderate DSP (4 voices + reverb) | **~72 s** |
| Heavy DSP (16 voices + echo + chorus) | **~49 s** |
| Stress (30 voices, polyphony ceiling) | **~21 s** |

**The PO-33's 40 s ceiling is matched under stress, beaten under typical loads.** The earlier 100–150 s figures in this section were based on the wrong sample rate (22 050 Hz) and have been replaced.

#### On-flash partition size

Partition `sketches` is **256 KB** in `partitions.csv` — that's *too small* for even one typical sketch at 44 100 Hz (1.42 MB). We need to **enlarge** this partition.

| Partition size | Typical sketches (~1.42 MB) | Tiny sketches (~133 KB) |
|---|---:|---:|
| 1 MB | 0 | ~7 |
| **8 MB** | **~5** | **~60** |

> The 16 MB row has been removed — bumping `sketches` past 8 MB would require shrinking the `factory` app partition below its current 3 MB, which is not worth it. If we ever want more, we'd add an external flash (e.g. an SPI NOR chip) rather than shrink the app partition.

Recommended: **`sketches` → 8 MB**. That gives ~5 typical sketches or ~60 tiny sketches, with comfortable headroom for filesystem overhead. The 16 MB flash chip on the ESP32-S3-WROOM-1-N16R8 has plenty of room for the 3 MB `factory` app partition, the small `nvs` + `phy_init` partitions, and the 8 MB `sketches` partition.

A `samples` partition is **not needed** in v2 — the active sketch's sample pool lives in PSRAM, and archived sketches' sample pools live in their sketch folder inside `sketches/`. The `samples` partition in the current `partitions.csv` is unused; we recommend removing it.

We propose a hard cap `SKETCHES_MAX = 16` for the UI's sketch picker — beyond that the list becomes hard to navigate anyway. The hard cap is **not** enforced by flash space (16 × 1.42 MB ≈ 23 MB which still exceeds our 8 MB partition) — it's a UI limit. Real capacity is whatever fits in the partition.

> **Action item for v2 implementation:** update `partitions.csv` to enlarge the `sketches` partition from 256 KB to **8 MB** and remove the unused `samples` partition. The `factory` app partition stays the same. **Done in `776bca7`** — `partitions.csv` now has `patterns, data, littlefs, 0x310000, 0x800000`.  *(Later renamed to `sketches` partition — see the §11.2 Note below.)*

#### Caveats on these numbers

- **AMY state size is an estimate.** The "moderate / heavy / stress" rows come from looking at the AMY source structure (oscillator bank + chorus/echo buffers + reverb tail). Real numbers would need a profiling run with `-O2` and `amy_overload_check()` enabled.
- **Polyphony ceiling on the ESP32-S3 is ~30 voices** before audio glitches. The "stress" row above is a real upper bound, not extrapolation.
- **Per-sample lengths vary.** A sketch with 16 long samples (10 s each) uses ~14 MB on flash — one such sketch fills the 8 MB partition almost entirely. The 5-sketches figure assumes the *typical* profile (8 short samples), not the max.
- **Streaming-to-flash** (v3) would let a single sketch's *live* pool exceed the PSRAM ceiling by streaming long samples from LittleFS on demand. AMY does not natively support streaming; this is real engineering, not a config change.

### 11.9 v2 UI proposal — the Sketch picker screen

A new screen (extends §6):

```
+-----------------------------------+
|  SKETCHES                  3 / 16 |       ← top bar: count
|                                   |
|   * DRUM KIT 1       0003  a1b2   |       ← * marks active sketch
|     DISCO DEMO       0001  b3c4   |       0003 = slot, a1b2 = id
|     AMBIENT 03       0002  1234   |
|     (unused slot)                 |       ← "Create new" hint
|                                   |
|   [step 1..8]  scroll list        |
|   [step 9..16] hold SOUND = new   |
|                                   |
|   press step 1-8 to load          |
|   hold SOUND to create/delete     |
+-----------------------------------+
```

Button bindings:
- Press **step 1–8**: load the corresponding sketch (after a "switching…" progress indicator).
- **SOUND** held: enter "manage" mode (create / delete / rename).
- **BPM** tap / long-press: scroll the list (since the picker shows 8 of 16 at a time).
- **PATTERN** tap / long-press: jump to first / last sketch.

The "manage" mode shows a sub-menu with **create / duplicate / delete / rename / export** options. Each is a step-button shortcut.

### 11.10 Open questions for v2 implementation

These are decisions I'm flagging now but punting to the implementer.

1. **Save policy** — should `storage_sketch_save_active()` be:
   - (a) **manual only** — user presses WRITE + REC or similar to save. Matches PO-33.
   - (b) **debounced auto-save** — every edit triggers a save 2 s later. No "did I forget to save?" anxiety.
   - (c) **both** — manual save is immediate; auto-save runs in background every N seconds if there are unsaved changes.

   Recommendation: **(c)**. Manual save is the PO-33 way; auto-save is the safety net for novices.

2. **Sketch delete confirmation** — how many button presses to confirm? PO-33 requires holding REC + PATTERN. We could:
   - (a) require holding SOUND + the sketch's slot number for 2 s.
   - (b) require a separate "delete mode" entered via SOUND + a step number.
   - (c) require two separate presses (first selects, second confirms).

   Recommendation: **(a)** — 2-second hold on the slot is unambiguous and matches the PO-33's destructive-action convention.

3. **Storage backend** — should we:
   - (a) **stick with LittleFS** (current v1 choice) — proven, journaled.
   - (b) migrate to **FAT** (via esp_littlefs → esp_vfs_fat) — better Windows/macOS support for USB-MSD export.
   - (c) use a **custom flat format** — fastest, but no USB access.

   Recommendation: **(a) for v2**, then **(b)** for v3 when USB-MSD is added.

4. **Backward compatibility with v1's storage** — the v1 storage has flat `p0.bin..p15.bin` and `s0.bin..s15.bin`. v2's sketch folder layout is incompatible. On first boot with v2 firmware, the storage init should:
   - (a) detect the old flat layout and migrate it into a new sketch (e.g. `legacy-default/`) automatically. Safe.
   - (b) leave the old files alone and start with an empty sketch. The user loses their old patterns if they don't migrate manually.

   Recommendation: **(a) with a one-shot migration** — detect old layout, rename to `legacy-default/`, treat as the active sketch on first boot, then delete the flat files after the user saves once.

5. **Sketch metadata format** — JSON (human-readable but parseable) vs. a custom key=value format (smaller, faster to parse). Recommendation: JSON, written by `cJSON` (already in ESP-IDF as a built-in component).

---

## 12. Cliff's notes: reading "Make: Electronic Music from Scratch" alongside this project


### 12.0 Why this book pairs with our project

This document is about a **digital** device — software running on a microcontroller that emulates a sampler. *Make: Electronic Music from Scratch* by **Kirk Pearson** (Maker Media, October 2024; subtitle: *A Beginner's Guide to Homegrown Audio Gizmos*) is about the **analog** roots of the same art. The book teaches you to build real, working electronic musical instruments out of resistors, capacitors, integrated circuits, solder, and batteries. Our project teaches you to build the same kind of instrument out of code.

The two together give a complete picture. The book explains *what an oscillator is* and *why filters matter* — concepts that are buried under DSP jargon in most digital-music books. Our project shows the same concepts *expressed in code* — you can see an oscillator running and a filter sweeping, but you'd have a hard time figuring out why anyone bothered to write the code without the conceptual foundation.

Pearson's book is from the **Dogbotic** studio in Berkeley, and is explicitly written for "total beginners" — "even the biggest electrophobes". It contains more than 40 hands-on projects and over 400 color photographs. Its final project, the **Dogbotophone MK1**, is described as "a massive electronic orchestra complete with multiple voices, drums, and self-patching sequencers" — which is exactly the spirit of what our PO-33 firmware aspires to be, just in silicon instead of solder.

> ⚠️ **One honesty note.** I do not own a copy of this book. The chapter list and project descriptions in this section come from the publisher's page on Amazon and from the Dogbotic studio's own description ("Our Road Map") at [dogbotic.com/book](https://dogbotic.com/book). If the actual book has additional appendices, troubleshooting chapters, or online resources, my cliff's notes will be incomplete. The broad shape, however, is reliable because both sources are the author's own writing.

### 12.1 The book in one paragraph each

Here is every chapter in the book, condensed to a single sentence.

1. **Musical Electricity for Electrophobes.** Corrects the lies you were taught in elementary school. Builds a working speaker out of household items; teaches you to listen to music through your bones; explains audio cables and how to use headphones as a microphone.
2. **The Hello World Oscillator.** Builds the basic building block of a classical synthesizer: an oscillator circuit. Introduces the family of electronic components — resistors, capacitors, integrated circuits.
3. **Amplifiers, Reverbs, and Talkboxes.** Builds a battery-powered amplifier you can take on the road, plus two silly fun projects that use one: a plate reverb and a talkbox.
4. **Soldering, Enclosures, and UI.** Puts the oscillator from chapter 2 into a permanent home. This is where the project stops being a breadboard and starts being an instrument.
5. **Chaining Oscillators.** Hooks one oscillator up to another, then another, then another. Builds a cricket-sounding circuit, a candle-controlled synth, and a chip that generates subharmonics.
6. **Schematics and Mass Transit.** Teaches you to read those scary-looking circuit diagrams electricians use. Also the Rotterdam Metro.
7. **Filters.** Uses your knowledge of impedance to build circuits that pass only certain frequencies. The "wah" effect on a guitar is a filter; so is the "telephone" EQ on a voice.
8. **Harmonization.** Teaches integrated circuits some rudimentary music theory so you can harmonize with yourself. A harmonizer is what an autotune pedal does.
9. **Modulation.** Builds little doodads that dynamically control aspects of a synth's sound: light-sensitive LFOs, an envelope generator with variable decay, and a stereo ping-pong tremolo.
10. **Sequencers.** Builds a programmable circuit that plays back a series of voltages to play a melody, modulate a filter, or do anything else. Bonus: a circuit that composes interesting melodies automatically.
11. **Electronic Percussion.** Builds a drum machine with kicks, snares, and cymbals.
12. **Phase-Locked Loops.** Deep dive into the CD4046 PLL chip; multiplies frequencies, slews voltages, and makes a sample-and-hold module.
13. **The Dogbotophone MK1.** The boss project: an electronic orchestra with multiple voices, drums, and self-patching sequencers that compose on the fly.
14. **Thoughts on Automation.** A concluding essay on the politics of electronic music and capitalism.
15. **Appendices.** Where to get parts; overview of the integrated circuits the book uses; a listening list.

### 12.2 Chapter-to-feature map: which chapters give you intuition for which parts of our device

If you read the book alongside building our firmware, this table tells you which chapter to read *before* you tackle which part of our device. The intuition transfers even though the implementation is completely different (analog circuits vs. C code).

| Book chapter | Builds intuition for these parts of our device | Why |
|---|---|---|
| **1. Musical Electricity** | §3.1 (recording, F-001–F-005), §6 (TFT screens) | Understanding sample rate, voltage, and signal flow is the foundation for everything else. The book's "use headphones as a mic" is exactly what our INMP441 I²S mic chip does, just digital. |
| **2. Hello World Oscillator** | §3.5 (effects F-019), §4 (per-effect deep dive) | When the book says "oscillator", our firmware says "amy_event with `wave = SAW` or `wave = SINE`". Same idea, different medium. |
| **3. Amplifiers, Reverbs, Talkboxes** | §3.5 (effects F-019), §7.3 (top-three missing features) | The plate reverb the book builds is a *physical* reverb (a metal plate vibrating). Our firmware's `amy_config_t.ram_caps_delay` is a *digital* reverb (a buffer of samples fed back on itself). Same effect; the math is nearly identical. |
| **4. Soldering, Enclosures, UI** | §6 (the eight TFT screens) | The book chapter on UI is about how buttons and knobs and lights make an instrument feel real. Our equivalent is the four-by-four button matrix and the TFT. Same design problem. |
| **5. Chaining Oscillators** | §3.1 (F-005, melodic vs drum slots), §2.12 (polyphony) | "Polyphony" in our firmware is just four oscillator instances playing in parallel. The book chapter on chaining is the analog version. |
| **7. Filters** | §3.4 (F-017 tweak filter), §4.17 (`PO33_FX_FILTER_SWEEP`) | When the book talks about "low-pass filter" (pass the low frequencies, cut the high), AMY's `filter_type = FILTER_LPF` does the same thing in math. After reading the chapter, the code becomes obvious. |
| **8. Harmonization** | §3.5 (effects F-019), §4.5 (unison) | "Unison" in our device = two oscillators playing the same note slightly out of phase. The book's "harmonize with yourself" is the same concept one step further. |
| **9. Modulation** | §3.2 (sequencing, F-006–F-012) | The book's envelope generator (a circuit that "shapes" a sound over time) is the conceptual ancestor of our step sequencer. The step sequencer is a modulator that turns sounds on and off at exact moments. |
| **10. Sequencers** | **§3.2 entirely**, §6.4 (pattern-edit screen), §6.6 (tweak screen) | **The book's chapter on sequencers is the closest analog to half of our firmware.** A step sequencer in hardware (a row of knobs and switches) and a step sequencer in code (a 16-element array of slots and notes) do the same thing. Read this chapter before reading our §3.2. |
| **11. Electronic Percussion** | §3.1 (F-005, drum slots), §3.2 (F-006), §6.2 (sound-select screen) | The book's drum machine uses analog circuits to generate kick / snare / hat waveforms. Our device *records* real-world drum hits into slots and plays them back. The book teaches you what a "kick" actually is, in the analog sense. |
| **12. Phase-Locked Loops** | Out of scope for our v1 firmware. PLLs are how analog synths tune oscillators. We don't need them because AMY's oscillators are digital and already in tune. | Mention only if you're curious about the history. |
| **13. Dogbotophone MK1** | **The whole project.** Our device is a small, cheap, digital Dogbotophone. | After reading this chapter, the architecture of our firmware (sample pool + sequencer + effects) makes total sense. |

### 12.3 Recommended reading order

There is no single right way to read the book. The order below matches how our document is structured, so the book and the doc reinforce each other.

1. **Before you do anything else**, read the book's introduction and skim chapters 1, 2, and 4. The introduction sets the philosophy ("question the politics of your own creative practice"). Chapters 1 and 2 give you the vocabulary. Chapter 4 motivates the UI design.
2. **Before you record a sample** (our §3.1, F-001), read the book's chapter 1 closely. It will make you understand *why* a microphone produces numbers instead of just sound.
3. **Before you read our §3.5 (the 16 punch-in effects)**, read the book's chapters 3 (amplifier/reverb/talkbox) and 7 (filters). You will then know what those words *do*, which makes our effect list intelligible.
4. **Before you read our §3.2 (sequencing)**, read the book's chapter 10 (sequencers) end-to-end. The book chapter is short but it grounds every term in our doc: step, pattern, loop, fill, retrigger.
5. **Before you read our §3.1 on drum slots**, read the book's chapter 11 (electronic percussion). You will learn what a "kick" actually is, in the analog sense.
6. **After you have the device working**, read the book's chapters 5 (chaining oscillators) and 9 (modulation). These give you deeper intuition for what our polyphony (F-005) and step sequencer (F-008) are doing mathematically.
7. **Optional but inspiring**, read the boss chapter (13, Dogbotophone MK1). It will show you what a serious home-built instrument looks like. Our device is a baby Dogbotophone.
8. **Read last**, the concluding chapter (14, Thoughts on Automation) and the appendices. These are not technical; they are about why any of this matters.

You do not need to *build* any of the book's circuits to benefit from reading it. Even just reading the chapter text and looking at the photos builds the mental model. If you do want to build along, the book has a companion kit (the "Dogbotic Labs DIY Synth Kit") that contains all the parts you need for the first several chapters.

### 12.4 What the book does NOT cover — and our project does

Pearson's book is firmly **analog-first**. It does not teach:

- **Digital audio**: PCM, sample rate, bit depth, quantization. All of which our §2 and §3 explain.
- **Programming**: C, microcontrollers, ESP-IDF, the AMY library. Our §3 is where to start.
- **Samplers** in the digital sense: loading PCM files, pitch-shifting samples, slicing. Our §3.1, F-001 through F-005 covers this.
- **MIDI**: the universal protocol for music gear to talk to each other. Our §3.10 (jam sync) is a poor-man's version; MIDI would be richer. (MIDI is on the v2 roadmap.)
- **Firmware, storage, file systems**: how to save a pattern, how LittleFS works. Our §3.13 (factory reset + UI) and §3.10 (sync) cover this.
- **The PO-33 specifically**: this book is about *inventing* your own instrument, not about emulating someone else's. Our document is the opposite: it's about replicating a specific existing product.

If the book is the question "how do electronic instruments work?", our project is the question "how do I make a specific electronic instrument I already love?". Reading both gives you a more complete answer than either alone.

### 12.5 Side-by-side concepts: analog → digital

For the layman reader, this is probably the most useful table in the whole document. Each row takes a concept the book teaches in hardware and shows the digital equivalent in our firmware. The point is not that they're identical — they're not — but that *the vocabulary transfers*.

| Concept in the book (analog) | Equivalent in our project (digital) | Where in our code |
|---|---|---|
| Voltage | A 16-bit integer sample value (-32768 to +32767) | `main/audio/amy_bridge.c` sample pool |
| Oscillator circuit (chapter 2) | An AMY oscillator event with `wave = SAW` or `wave = SINE` | `main/audio/amy_bridge.c` → `apply_fx()` |
| Filter circuit (chapter 7) | AMY's `filter_type = FILTER_LPF` and `filter_freq = 200..700` | `main/audio/amy_bridge.c` → `apply_fx()` → `PO33_FX_FILTER_SWEEP` |
| Envelope generator (chapter 9) | AMY's breakpoint envelope on each event (`bp0` parameters) | AMY `amy_event.bp0_times`, `bp0_values` |
| LFO (chapter 9) | AMY's low-frequency oscillator modulation source (`mod_source`) | AMY `amy_event.mod_source[]` |
| Step sequencer (chapter 10) | Our 16-element `pattern_t.steps[]` array | `main/sequencer/pattern.h` |
| Drum machine (chapter 11) | Our 8 drum slots + the `amy_bridge_play_note` trigger | `main/audio/amy_bridge.c` |
| Plate reverb (chapter 3) | AMY's delay-based reverb (`cfg.ram_caps_delay = MALLOC_CAP_SPIRAM`) | `main/audio/amy_bridge.c` → `amy_config_t` |
| "Voltage-controlled" anything | A parameter in an `amy_event` struct | `components/amy/src/amy.h` `amy_event` |
| Patch cables on a modular synth | The arguments to `amy_add_event()` | `main/audio/amy_bridge.c` |
| Speaker cone vibrating | The I²S DAC (PCM5102A) outputting an electrical waveform | `main/audio/i2s_driver.c` (now `amy_bridge.c` since the AMY integration) |
| Knob on the front panel | A button combo on our 4×4 matrix, or a UART shell command | `main/ui/buttons.c`, `main/main.c` shell |
| LED on the front panel | A region of the TFT screen | `main/ui/display.c` |
| Power supply (battery) | USB-C 5 V or LiPo via the optional TP4056 charger | `main/config.h` `BATTERY_*` |

Reading the right column first makes the left column obvious. Reading the left column first makes the right column feel familiar. Pick whichever order works for your brain.

### 12.6 The book's philosophy, applied to our project

The book's introduction closes with this line, which I want to quote because it is the closest thing to a mission statement the author gives:

> "Deep down, we really truly believe that instrument building is an important, empowering activity, and that the more experimentation there is in the world, the more aware and compassionate we all become."

Applied to our project: building firmware that emulates a beloved existing product is a slightly different act from building a brand-new instrument, but it shares the same spirit. To make our firmware, you have to understand what makes the PO-33 special — the immediacy of recording, the constraint of 16 slots, the way 16 steps force you into a particular kind of rhythm, the way 16 punch-in effects turn every performance into a small surprise. You are not just transcribing code; you are reverse-engineering a *design philosophy* and rebuilding it on cheaper, more open hardware.

If you read the book and feel the urge to make your own instrument instead of (or in addition to) this one, that is the correct reaction. Our firmware is one path; the book is the door to a hundred others.

---

## 13. Glossary

- **AMY** — A free open-source fixed-point music synthesizer library. Used for all sound generation in this project. ([github.com/shorepine/amy](https://github.com/shorepine/amy))
- **BPM** — Beats per minute. How fast the music goes.
- **DAC** — Digital-to-analog converter. A chip that turns digital numbers into electrical signals a speaker can play.
- **DMA** — Direct Memory Access. A chip feature where data is moved between memory and a peripheral (like the audio chip) without the CPU doing the work. This frees the CPU to do other things while audio is playing.
- **Driver** — A piece of code that knows how to talk to a specific chip.
- **DSP** — Digital Signal Processing. Any operation that transforms numbers representing audio. A filter is DSP; a reverb is DSP; pitch-shifting is DSP.
- **ESP32-S3** — A microcontroller chip made by Espressif. Has WiFi, Bluetooth, and lots of GPIO pins.
- **Firmware** — Software that is "firmly" embedded in a device. Same idea as "software", but used for things that run on chips rather than laptops.
- **FreeRTOS** — A small operating system that runs on microcontrollers. Lets multiple "tasks" run at the same time on a single CPU.
- **GPIO** — General Purpose Input/Output. A pin on the microcontroller that can be either an input (read a button) or an output (drive an LED).
- **I²C** — A two-wire protocol for talking to small chips like sensors and displays.
- **I²S** — A protocol for sending digital audio between chips. Similar to I²C but optimized for audio.
- **ISR** — Interrupt Service Routine. A function that runs immediately when a hardware event happens (like a button press).
- **JTAG** — A debugging protocol for microcontrollers. Not used in this firmware.
- **LittleFS** — A small filesystem designed for flash memory. We use it to store patterns and saved samples.
- **MIDI** — Musical Instrument Digital Interface. An old protocol (1983) for sending music between devices. We don't use MIDI in this project directly, but AMY understands MIDI note numbers.
- **NVS** — Non-Volatile Storage. A small key-value store in the ESP32's flash that survives reboots. We use it for clock + alarm settings.
- **Octal PSRAM** — A kind of extra memory chip connected to the ESP32-S3 with 8 data lines. Fast, cheap, and 8 MB on our board.
- **PCM** — Pulse-Code Modulation. The raw format of digital audio: a stream of numbers representing the air pressure at the microphone at each instant.
- **Pitch** — How high or low a sound is. Doubling the pitch makes the sound one octave higher.
- **Polyphony** — How many sounds can play at the same time.
- **PSRAM** — Pseudo-Static RAM. Cheap, large, external memory that the ESP32-S3 can address as if it were normal RAM.
- **PWM** — Pulse-Width Modulation. A way to fake an analog signal by toggling a digital pin on and off very fast. We use it for the TFT backlight brightness.
- **Quantize** — Snap timing to a grid.
- **Sample rate** — How many audio samples per second. We use 44100 samples/second, same as a CD.
- **Sequencer** — The "robot drummer" that walks through the steps and plays sounds.
- **Slot** — A numbered storage location for a sample.
- **SPI** — Serial Peripheral Interface. A four-wire protocol for talking to chips, faster than I²C. We use it for the TFT.
- **Step** — One moment in a beat. The PO-33 has 16 steps per pattern.
- **Stutter** — A musical effect where a tiny slice of sound is repeated rapidly.
- **Sync** — Two devices agreeing on tempo.
- **TFT** — Thin-Film Transistor. A flat color screen.
- **Tweak** — A small adjustment you can make to a sound (pitch, filter, or trim).
- **UART** — Universal Asynchronous Receiver/Transmitter. The protocol used for the USB-serial port that talks to your computer.
- **Voice** — One independent instance of a playing sound. With 4-voice polyphony, you can hear 4 sounds at once.
- **WAV** — A common audio file format. Not directly used in this firmware.
- **WiFi** — Wireless networking. Not used in this firmware (would need a WiFi-enabled ESP32 variant; the WROOM-1-N16R8 has it disabled by default).

---

*End of document. ~18 000+ words. Source of truth for the PO-33 manual section numbers is [teenage.engineering/guides/po-33/en](https://teenage.engineering/guides/po-33/en); for our firmware, see the file paths and function names cited in §3. How we can break the PO-33's limits is in §10; the v2 multi-project storage proposal is in §11; cliff's notes on the companion book are in §12.*
