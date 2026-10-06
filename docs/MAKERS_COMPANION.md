# The Maker's Companion to the PO-33 ESP32-S3 Firmware

## A study guide for *Make: Electronic Music from Scratch* × RavinePhoenix.1une.cc

> **Audience:** complete novices. Every technical term is defined the first
> time it appears. If you've never written a line of code or soldered a
> single joint, you are the intended reader.
>
> **How to use this book.** Each chapter pairs one chapter of Kirk Pearson's
> *Make: Electronic Music from Scratch* (Dogbotic, 2024) with one slice of our
> PO-33 firmware. The first half of each chapter is a friendly recap of
> Pearson's chapter in plain language. The second half shows you how to make
> our firmware *do the same thing* — usually in a UART shell session, since
> that's how a headless developer tests the firmware. There are no math
> prerequisites and no C-code-reading prerequisites; the few lines of code we
> do quote are heavily commented.

---

## Table of contents

1. [How to use this book](#how-to-use-this-book)
2. [Setup: from zero to a beeping ESP32 in fifteen minutes](#setup)
3. [Companion to Pearson Chapter 1 — A People's History of Electronic Music](#chapter-1)
4. [Companion to Pearson Chapter 2 — Musical Electricity for Electrophobes](#chapter-2)
5. [Companion to Pearson Chapter 3 — The Hello World Oscillator](#chapter-3)
6. [Companion to Pearson Chapter 4 — Amps, Reverbs, and Talkboxes](#chapter-4)
7. [Companion to Pearson Chapter 5 — Soldering, Enclosures, and UI](#chapter-5)
8. [Companion to Pearson Chapter 6 — Chaining Oscillators](#chapter-6)
9. [Companion to Pearson Chapter 7 — Schematics and Mass Transit](#chapter-7)
10. [Companion to Pearson Chapter 8 — Filters](#chapter-8)
11. [Companion to Pearson Chapter 9 — Harmonization](#chapter-9)
12. [Companion to Pearson Chapter 10 — Modulation](#chapter-10)
13. [Companion to Pearson Chapter 11 — Sequencers](#chapter-11)
14. [Companion to Pearson Chapter 12 — Electronic Percussion](#chapter-12)
15. [Companion to Pearson Chapter 13 — Phase-Locked Loops](#chapter-13)
16. [Companion to Pearson Chapter 14 — The Dogbotophone MK1](#chapter-14)
17. [Companion to Pearson Chapter 15 — Thoughts on Automation](#chapter-15)
18. [Companion to the Appendices](#appendices)
19. [Glossary](#glossary)
20. [Index](#index)

---

<a id="how-to-use-this-book"></a>
## How to use this book

This book assumes you have either:

- a flashed ESP32-S3 board with our firmware running, *or*
- a copy of the firmware source you can build and flash yourself.

If neither is true yet, jump straight to the **Setup** chapter below. It walks
you through getting a board talking to your computer over USB.

Once you're set up, you have two reasonable ways to read this book:

- **Sequential, in lock-step with Pearson.** Read Pearson chapter N, then our
  chapter N. The two are designed to be read together; the conceptual debt
  from one chapter is paid by the next.
- **Topical, dipping in as you need it.** If you already know Pearson and
  just want to see how a particular concept maps to our firmware, the
  [Glossary](#glossary) lists every concept Pearson introduces and points to
  the chapter of ours that explains the digital equivalent.

There are three kinds of paragraphs in this book, and they're marked so you
can skip the parts that don't interest you:

- **🎓 Background.** Conceptual explanation. Read these if you're new to the
  topic. Skip them if you've already read Pearson.
- **🔧 Try it.** A concrete exercise using the UART shell on the ESP32. These
  are short — usually under five commands — and they're the most valuable
  part of the book. Do them, even if you read nothing else.
- **🛠 Code reference.** A small pointer into our firmware source. These
  exist so an engineer reading this book can find the implementation
  quickly. Skip them if you're not an engineer.

A note about honesty: this book is not a marketing document. Where our
firmware can simulate a Pearson project, we'll show you. Where it can't,
we'll tell you and link to the relevant section of our design document so
you know why.

---

<a id="setup"></a>
## Setup

This chapter assumes you've never touched an ESP32 before. If you have,
skim to the **Quick start** at the end.

### What you'll need

1. **An ESP32-S3-WROOM-1-N16R8 dev board.** This is the specific variant
   we support. It has 16 MB of flash and 8 MB of PSRAM, both of which we
   need. The board costs about $5 from any major electronics supplier
   (Mouser, Digikey, AliExpress). Look for the words "WROOM-1-N16R8"
   silkscreened on the metal shield on the back of the module.
2. **A USB-C cable.** The board uses USB-C for both power and serial
   communication. Any data cable will do; some charge-only cables will
   silently fail to enumerate as a serial device, so try a different
   cable if you can't connect.
3. **A computer with a serial terminal.** Any of:
   - **Linux or macOS:** `minicom`, `screen`, or just `cat` over
     `/dev/ttyUSB0` (Linux) or `/dev/tty.usbserial-*` (macOS).
   - **Windows:** PuTTY, Tera Term, or the built-in Windows Terminal.
   - **Browser-based:** the one-click web flasher on
     [ravinephoenix.1une.cc](https://ravinephoenix.1une.cc) can open a
     serial terminal in your browser. This is the easiest option.
4. **(Optional) Headphones or an external speaker.** The ESP32-S3 dev
   board has no built-in speaker; the firmware outputs audio via the I²S
   pins. Our hardware guide (`hardware/HARDWARE.md`) shows how to wire a
   $2 PCM5102A DAC chip to the board and connect it to a 3.5 mm jack.

If you bought a fully assembled **RavinePhoenix.1une.cc device** rather
than building your own, you can skip the hardware steps entirely. The
firmware already works on the off-the-shelf hardware.

### Plug it in

1. Plug the USB-C cable into the board and your computer.
2. If a red or blue LED lights up on the board, congratulations — the
   power section is working. If nothing lights up, try a different cable
   or USB port.
3. The board will enumerate as a serial device. The exact name depends
   on your operating system:
   - **Linux:** `/dev/ttyUSB0` or `/dev/ttyACM0`. You may need to add
     yourself to the `dialout` group: `sudo usermod -a -G dialout $USER`
     and then log out and back in.
   - **macOS:** `/dev/tty.usbserial-*`. No driver install needed.
   - **Windows:** `COM3`, `COM4`, etc. You may need to install the
     Silicon Labs CP210x driver from
     [silabs.com/developers/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).

### Open a serial terminal

Set your terminal to **115 200 baud, 8 data bits, no parity, 1 stop bit
(115200-8-N-1)**. No flow control. No line ending translation.

In `minicom`:
```
minicom -D /dev/ttyUSB0 -b 115200
```

In `screen`:
```
screen /dev/ttyUSB0 115200
```

In `cat` (read-only — for testing):
```
cat /dev/ttyUSB0
```

Press the reset button on the board (or send `Ctrl+T Ctrl+R` via the
ESP-IDF monitor, if you're using `idf.py monitor`). You should see
something like this scroll by:

```
I (312) boot:  ESP-IDF v6.1.2 2nd stage bootloader
I (421) cpu_start: Pro cpu start user code
...
I (1024) main: RavinePhoenix PO-33 firmware v0.6.0 starting.
I (1025) amy: AMY 1.0 ready.
I (1026) main: >
```

The `>` is the shell prompt. You are now talking to the firmware. Type
`help` and press Enter:

```
> help
```

The firmware prints a list of every shell command it understands. There
are about 30. This book will only use a handful of them, and we'll
introduce each one the first time we need it.

### Quick start

If you already have a flashed board and a working serial connection:

1. Type `help` and read the output.
2. Type `status` to see what the firmware currently knows about
   (active pattern, BPM, slot count, etc.).
3. Type `play` to start the empty pattern. Type `stop` to stop it.
4. You're ready to begin Chapter 1.

If anything goes wrong, the firmware has a `panic` log printed to the
serial port. Copy-pasting the last 30 lines into a search engine or a
GitHub issue is almost always enough to diagnose the problem.

### The shell verbs this book uses

Here's the cheat sheet for every UART command this book will use. We
introduce them one at a time in the chapters where they matter, but if
you skim ahead, this table tells you what's available.

| Verb | Args | What it does | Pearson analog |
|---|---|---|---|
| `play` | (none) | Start the active pattern | Chapter 11 sequencers |
| `stop` | (none) | Stop playback | Chapter 11 |
| `record_mic` | `<slot> <seconds>` | Record from the I²S mic into a slot | Chapter 2 (speaker as mic) |
| `slot_play` | `<slot>` | Play a recorded slot | Chapter 11 (sequencer steps) |
| `slot_clear` | `<slot>` | Erase a slot, free the audio buffer | Chapter 12 (drum sample delete) |
| `slot_copy` | `<dst> <src>` | Copy a slot's audio into another slot | Chapter 4 (amp + reverb = layered sample) |
| `slot_info` | (none) | List all slots: empty vs recorded, length in ms | Chapter 5 (UI: see what you have) |
| `pattern_info` | (none) | Show the active pattern's 16 steps | Chapter 11 (sequencer readout) |
| `pattern_copy` | `<dst> <src>` | Copy one pattern into another slot | Chapter 14 (Dogbotophone "pattern-changing sequencer") |
| `chain_show` | (none) | Print the chain (which patterns play in order) | Chapter 11 (sequencer) |
| `chain_append` | `<pattern>` | Add a pattern to the end of the chain | Chapter 11 |
| `chain_swap` | `<i> <j>` | Swap two entries in the chain | Chapter 11 (live re-order) |
| `chain_insert` | `<at> <pattern>` | Insert a pattern into the chain at a given index | Chapter 11 |
| `chain_remove` | `<i>` | Remove a chain entry | Chapter 11 |
| `chain_clear` | (none) | Wipe the entire chain | Chapter 11 |
| `bpm` | `<value>` | Set the sequencer tempo (60–240) | Chapter 11 (clock) |
| `fx` | `<name>` | Pick the next-step effect (e.g. `LOOP_16`, `STUTTER_4`) | Chapter 6 (vactrol arpeggiator), Chapter 10 (modulation) |
| `tweak_filter` | `<cutoff> <resonance>` | Apply a low-pass filter on the next triggered note (knob units, 0–255) | Chapter 8 (filters) |
| `note` | `<slot> <midi_note>` | Play a slot at a specific pitch (MIDI note number, 0–127) | Chapter 9 (octave harmonizer) |
| `alarm_set` | `<HH> <MM> <slot>` | Schedule a sample to fire at a wall-clock time | Chapter 6 (timing circuits), bonus feature |
| `sync_in_toggle` | (none) | Toggle whether incoming sync pulses drive the sequencer | Chapter 13 (PLL — same idea, different medium) |
| `status` | (none) | Dump everything the firmware knows | "Chapter 0" |

Two of these verbs — `fx` and `tweak_filter` — are the most-used in this
book. They directly correspond to Pearson's chapters 6, 8, 10, and 12.

---

<a id="chapter-1"></a>
## Chapter 1 — A People's History of Electronic Music

### 📖 What Pearson does

Chapter 1 is a survey of how electronic music came to be. Pearson ranges
from Thaddeus Cahill's 200-ton Telharmonium (1896), through Leon Theremin's
touch-free theremin (1920), Robert Moog's garage-startup synthesizers
(1960s), to the Ondioline, the first synthesizer that "imitated" traditional
instruments (1940s). He argues — convincingly — that the synthesizer is a
**folk instrument**: for most of its history it was homemade, garage-built, and
shared among curious amateurs. Companies like Moog, Korg, and Roland only
later industrialised it. He calls this out as a positive: making your own
instrument expands the definition of music, and reclaims the act of
construction from the corporations that own the supply chain.

The chapter is mostly prose, with three short essays:

- **Nobody trusts electronic music.** Synth players used to be portrayed in
  films as klutzy, talentless dorks; guitar players were the suave ones.
- **Synthesizers as products.** How Moog, Korg, and Roland re-branded the
  synth as a consumer good rather than a folk instrument.
- **The bane of learning electronics.** Why getting started is so hard.
- **Our road map.** Pearson's four-part plan for the rest of the book.

### 🎓 Background: why our project exists

The reason this firmware exists at all is Chapter 1. Pearson's whole
project is a vote of confidence in the homemade-instrument tradition.
Ours is the same vote, cast in C code instead of solder.

Two specific arguments from Pearson's chapter are worth carrying forward,
because they explain why our firmware makes the design choices it makes:

**Argument 1: a synthesizer does not need to be made in a factory by
professionals to be a synthesizer.** Our firmware runs on a $5
microcontroller. The Teenage Engineering PO-33 K.O! — the device we are
emulating — sells for about $80 and is a tiny plastic box with a custom
LCD. The DSP inside the PO-33 is in many ways less capable than the
AMY library we use, which is a free open-source fixed-point synth. By
running AMY on the ESP32-S3 and giving it a PO-33-shaped UI on top, we
are doing what the first synth-builders did: combining commodity parts
into something that has its own personality.

**Argument 2: products ship with their own code of conduct, which may or
may not suit the end user.** The real PO-33 is locked down. You cannot
read its firmware. You cannot add features. You cannot change the
sequencer's resolution, or the FX order, or the chain length. You can
only do what the manufacturer lets you do. Our firmware, by contrast,
is MIT-licensed. Every feature you can see on the device is something
you can read in the source, change, recompile, and reflash. If you
think the FX order is wrong, you can change it in
`main/audio/amy_bridge.c`. If you want 32 steps instead of 16, you can
change it in `main/sequencer/pattern.h`.

### 🔧 Try it on the device

This chapter has no exercises, because there are no circuits to build.
There is, however, one thing worth doing: take inventory of what you
have.

Open a serial terminal and type:

```
> status
```

The firmware prints something like:

```
Firmware:    RavinePhoenix PO-33 v0.6.0
Free heap:   92 KB
Free PSRAM:  6.4 MB
Active pat:  1
BPM:         120
Volume:      3/5
Battery:     87%
Time:        14:32
```

That is the entire state of the device in nine lines. It is the
digital equivalent of looking at a synthesizer and counting the knobs.
Compare to Pearson's how open the synthesizer used to look in 1975:
no menus, no presets, no screens — just a knob and a switch per
function. Our firmware has many more "functions" but the same
honesty: everything you see is a knob somewhere in the source code.

Now type:

```
> help
```

and read the list. About half the verbs you'll see in this book are
listed. The other half you'll type without prompting.

### 🛠 Code reference

- **The firmware that prints `status`** — `main/main.c`, the shell
  command `cmd_status()`. About 40 lines; reads every global variable
  in the project and formats them as a single human-readable string.
- **The hardware the firmware runs on** — `hardware/HARDWARE.md` and
  `main/config.h`. The ESP32-S3 pin map, the I²S microphone wiring,
  the PCM5102A DAC wiring, the TFT wiring.
- **The bootstrap story** — the comment block at the top of
  `main/main.c` describes the boot sequence: hardware init →
  LittleFS mount → NVS open → clock init → amy_init → button scan
  task → display render task → sequencer timer.

### 🚫 What we can't simulate

There is nothing in Pearson's Chapter 1 that requires simulation. It
is a manifesto, not a workshop. Read it; let it set the tone for the
chapters that follow; then move on.

---

<a id="chapter-2"></a>
## Chapter 2 — Musical Electricity for Electrophobes

### 📖 What Pearson does

Chapter 2 is a tour of the very simplest things electronic circuits can
do. It teaches you that electricity is just moving charges, that
conductors and insulators are materials with different degrees of
"willingness to move electrons", and that speakers and microphones are
the same device run in reverse. There are four small experiments:

- **The world's simplest speaker.** Wind wire around a magnet, hold
  the magnet near a paper cone, drive alternating current through
  the wire, watch the cone shake and make sound.
- **Speaker-as-microphone identity crises.** Take two speakers, hook
  them to each other through an amplifier. Speak into one. The other
  makes your voice louder. (The first half of this is in Chapter 2;
  the second half — using a speaker as a real microphone — is in
  Chapter 4.)
- **Piezomania.** Press a piezo buzzer (a small flat disc that flexes
  when voltage is applied), listen to the click. Then *tap* the
  piezo, listen to the small voltage it makes. Piezos are both
  speakers and microphones.
- **Sound through your skull.** Hold a vibrating tuning fork against
  your skull. The sound gets louder, because your bones conduct
  vibration better than air.

The chapter ends with a section called "A moment of clarity" that
distils everything: voltage is a difference in charge between two
points; current is the rate at which charge moves; resistance is how
much the material pushes back; and "audio" is just voltage waving
around at a frequency the ear can hear (20 Hz to 20 000 Hz).

### 🎓 Background: what "voltage" is in our firmware

Pearson spends most of this chapter explaining that **voltage is a
number**, not a thing. It is a measurement of "potential energy per
unit charge" — i.e., a measurement of how much oomph the electrons have.
When the book's "Hello World oscillator" later outputs a wave of
voltage, the voltage is a number that swings between, say, +0.5 V and
-0.5 V at some frequency.

In our firmware, **the equivalent of a voltage sample is a 16-bit
signed integer**. The AMY library (the audio engine we use) represents
each audio sample as a number between **-32 768 and +32 767**. Zero is
"silence". +32 767 is "as loud as possible, speaker pushed fully out".
-32 768 is "as loud as possible, speaker pushed fully in". Anything
in between is some position of the speaker cone. The DAC chip
(PCM5102A) is the part that turns the number into actual voltage at
44 100 times per second — that 44 100 is the **sample rate**, and it
is exactly Pearson's "44 100 voltage readings per second".

So when our firmware plays a recording, what is actually happening is:
a long array of 16-bit integers (one per centisecond of audio) gets
streamed to the DAC, which turns each integer into a voltage. The
electromagnet inside the speaker responds to the voltage by wiggling.
The air in front of the speaker wiggles. Your eardrum wiggles. You
hear sound.

**This is the single most important sentence in this whole book:**
*audio is a sequence of numbers that some hardware eventually turns into
wiggling air*. Every audio effect, every filter, every reverb, every
pitch-shifter in our project — and every circuit in Pearson's book —
is a way of transforming that sequence.

The I²S microphone we use (the INMP441) is exactly the reverse: it
listens to the air pressure and emits a stream of 16-bit integers at
44 100 samples/second. That stream goes into a buffer. The buffer
becomes a slot. The slot gets played back through the DAC. That is
the entire pipeline of F-001 (record a sample) — Pearson's
"speaker-as-microphone" project, in software.

### 🔧 Try it on the device

This is the first hands-on chapter in our book. Make sure you have
headphones or a speaker plugged in.

**Exercise 1: confirm the firmware can produce sound.**

```
> slot_play 1
```

If slot 1 has nothing in it, you'll get silence (or possibly an AMY
synth tone if the slot is configured as a melodic slot and has a
default patch). That is fine. The point is the verb works — the
firmware is talking to the DAC, and the DAC is talking to your
earphones.

**Exercise 2: record 2 seconds of ambient sound from the microphone.**

```
> record_mic 1 2
```

Hold the device near a sound source — your mouth, a fan, a
favourite song on your phone's speaker. After two seconds the firmware
prints:

```
Recorded 2.00 s -> slot 1 (88200 samples)
```

Now play it back:

```
> slot_play 1
```

You should hear what you just recorded. The 88 200 samples is
exactly 2.0 seconds × 44 100 samples/second. The 16-bit integers are
sitting in PSRAM, waiting to be streamed out.

**Exercise 3: see what you recorded.**

```
> slot_info
```

The firmware prints a 16-line table. Slot 1's row shows
`RECORDED 2000 ms` (or similar). The other slots show `empty`.

**Exercise 4: do Pearson's "microphone identity crisis" experiment
in software.**

Record your voice into slot 1, then immediately play it back through
slot 1. The firmware doesn't know the difference between the input
side (INMP441 mic, I²S RX) and the playback side (PCM5102A DAC, I²S
TX). To the firmware, both are 16-bit integers at 44 100 samples/sec.
Just as Pearson showed that a speaker is a microphone run in reverse,
our firmware shows that the same buffer can be the input or the
output of the audio system. There is no fundamental difference.

### 🛠 Code reference

- **Record from the I²S mic** — `main/audio/amy_bridge.c`,
  `amy_bridge_record_mic()`. The function opens an I²S RX channel,
  fills a 16-bit sample buffer, and calls `pcm_load_external()` to
  register the buffer with AMY as a playable preset.
- **Play a recorded slot** — same file, `amy_bridge_play_note()`,
  with `e.wave = PCM` and `e.patch_number = PO33_PRESET_BASE + slot`.
- **The 44 100 sample rate** — `main/config.h`, `AMY_SAMPLE_RATE`.
  Changing it would make the firmware sound like a chipmunk (up) or
  like a foghorn (down).
- **The PSRAM pool** — same file, `s_pcm_pool[]`. This is where
  recorded audio lives. The pool is 40 seconds long (40 s × 44 100
  samples/s × 2 bytes/sample = 3.5 MB).

### 🚫 What we can't simulate

- **Piezo buzzers.** We don't expose a piezo GPIO in this firmware's
  default hardware. The INMP441 is a MEMS mic, which works on a
  different principle (capacitive sensing, not piezo). The piezo
  project — "tap the piezo, hear the click, then tap again and watch
  the voltage on a multimeter" — has no digital analog in our
  current firmware.
- **Sound through your skull.** Pearson's tuning-fork experiment is
  about your skeleton conducting vibration better than air. There
  is no firmware analog because there is no "skull" in the
  firmware. The amplifier (the PCM5102A and the LM4853 or similar
  headphone amp) is the closest analog: it does for the audio
  signal what your skull does for the tuning fork.

---

<a id="chapter-3"></a>
## Chapter 3 — The Hello World Oscillator

### 📖 What Pearson does

Chapter 3 is the centrepiece of the book. Pearson introduces the
breadboard, the resistor, the LED, the potentiometer, the capacitor,
and finally the **integrated circuit** — specifically the **555 timer
chip** in **astable mode**, which is the classical "first oscillator"
circuit. With a 555, two resistors, and a capacitor, you can build a
circuit that outputs a square wave (voltage jumping between +V and 0)
at any frequency you want. The frequency is set by the resistor and
capacitor values.

The chapter has these sub-sections, in order:

- **The breadboard.** How the rows of holes are connected, how to use
  jumper wires.
- **How jumper wires work.** (Basically, copper conducts.)
- **Resistors.** Pushing back against current. Measured in ohms.
  10 kΩ means 10 000 ohms.
- **LEDs and other diodes.** One-way valves for current.
- **Potentiometers.** Variable resistors. Twist the knob, change the
  resistance, change the circuit's behaviour.
- **Capacitors.** Tiny batteries that fill up and empty out. The
  combination of a resistor and a capacitor makes an RC time constant,
  which sets the timing of the 555 oscillator.
- **Integrated circuits.** Tiny pre-built circuits in a plastic
  package. The 555 is one. The 40106 is another (a hex inverter).
- **My-First-Square-Wave-Oscillator™.** The main project. A 555,
  two resistors, a capacitor, a 9 V battery, and a potentiometer.
  Output: a square wave you can hear as a pip pip pip.
- **Variations:** The Pinch-O-Matic, Playing with Your Food,
  The Pencil-Pusher, A Photo Theremin.

This chapter is where the book transitions from "this is what a
component is" to "this is what a component does when you put it in a
circuit". If you read one chapter of Pearson's book, this is the one.

### 🎓 Background: what an oscillator is, in code

An **oscillator** is a thing that produces a repeating wave. In
hardware, a 555 oscillator produces a square wave: the voltage is +5 V
for some duration, then 0 V for some duration, then +5 V again, etc.
The frequency is how many times per second this happens. A 440 Hz
oscillator is the **A above middle C** — every musician's tuning
reference.

In our firmware, an oscillator is **a struct in C** called
`amy_event`. The relevant fields are:

```
wave = SAW          /* SAW, SINE, SQUARE, PULSE, PCM, NOISE, ... */
frequency = 440.0   /* Hz */
amplitude = 1.0     /* 0..1 */
```

When you call `amy_add_event()` with this struct, AMY's render task
(notices the FPU, notices the second core) starts producing 16-bit
samples 44 100 times per second that describe the chosen waveform at
the chosen frequency. The DAC turns those samples into voltage. The
speaker wiggles.

That is the entire translation. Pearson's "555 timer outputting a
square wave at 440 Hz" and our firmware's "amy_event with wave=SQUARE
and frequency=440" are doing the same thing. They differ only in the
material — electrons vs. floating-point math.

### 🔧 Try it on the device

The PO-33 has 16 sample slots. 8 are **drum slots** (slots 1–8) and
8 are **melodic slots** (slots 9–16). Drum slots are monophonic — they
play one note at a time. Melodic slots are 4-voice polyphonic and
respond to pitch.

**Exercise 1: play the firmware's built-in oscillator.**

A melodic slot with no recording falls through to AMY's default
synth. Pick slot 9 and play it:

```
> slot_play 9
```

You'll hear a tone. Which tone depends on what AMY's default patch
is. By default it's a saw-wave at some pitch; the firmware knows the
slot is empty, so it doesn't try to play a sample.

**Exercise 2: trigger an oscillator at a specific pitch.**

```
> note 9 69
```

This plays slot 9 at MIDI note 69, which is **A4 = 440 Hz**. If you
have a music background, that note should sound familiar — it's
concert pitch. Hit it again:

```
> note 9 81
```

That's MIDI note 81, which is **A5 = 880 Hz**. One octave up.

This is Pearson's 555 oscillator, with the potentiometer controlling
pitch. In our firmware the "potentiometer" is a MIDI note number.

**Exercise 3: change the waveform.**

The PO-33's punch-in effects let you change the timbre. The
equivalent of changing the 555's capacitor (which would change the
*shape* of the wave in a more involved analog sub-circuit) is to pick
a different FX. Try:

```
> fx STUTTER_4
> note 9 69
```

That's a stuttering repeat of the A note. Not exactly an oscillator
modification, but it's the closest punch-in we have to "modulate the
carrier".

**Exercise 4: register a "Pin-O-Matic" (Pearson's pinch-resistor
variation).**

The Pinch-O-Matic is a potentiometer replaced by a squishy
conductive material. The harder you squeeze, the more current. In
our firmware the analog is **velocity** — a per-note parameter
between 0 and 127 that scales the amplitude:

```
> note 9 69     (firmware uses default velocity)
```

There is no UART verb to set velocity directly in v0.6 — the PO-33
itself derives velocity from the step-trigger button press. But the
concept is there: every `note` event has a velocity field, and louder
velocities make louder notes. Pearson's "squeeze harder" is our
"press the step button harder".

**Exercise 5: do the Photo Theremin (Pearson variation 4).**

A photo theremin uses a light-dependent resistor instead of a
potentiometer. Shine a torch on the LDR, the pitch goes up. Cover it,
the pitch goes down.

Our firmware has no light sensor. But the *idea* — a continuous
input parameter that drives pitch in real time — is exactly what
**Knob A** does on our device. Knob A maps to the "tweak Tone"
parameter, which adjusts the pitch of the next triggered note. With
Knob A turned up, the firmware plays the slot one or two semitones
sharp. With Knob A turned down, flat.

If you have a RavinePhoenix device in front of you, try:

1. Hold `FX` and tap it (cycles tweak mode to **TONE**).
2. Press `SOUND + step 9` to pick slot 9 (this makes it the *active*
   slot). Note: a bare `step 9` press plays the *active* slot at slice 9 —
   it does **not** pick slot 9. Selection is always `SOUND + pad`.
3. Press `PLAY`.
4. Twist **Knob A** while slot 9 plays. The pitch glides up and
   down.

That is Pearson's photo theremin, with a knob instead of an LDR.

### 🛠 Code reference

- **The default synth patch** — `components/amy/src/amy.c`, function
  `amy_default_event()`. Returns a fully-zeroed event struct; the
  caller fills in `wave`, `frequency`, `amplitude`, `velocity`, etc.
- **The `note` verb** — `main/main.c`, `cmd_note()`. Calls
  `amy_bridge_play_note(slot, midi_note, ...)`. Internally that
  function populates an `amy_event` and calls `amy_add_event()`.
- **The 4-voice polyphony** — `main/audio/amy_bridge.c`,
  `amy_bridge_play_note()`. Sets `e.num_voices = (slot <
  SLOT_DRUM_COUNT) ? 1 : VOICE_COUNT`, where `VOICE_COUNT` is 4.
  AMY handles the polyphony: it allocates up to 4 voices per
  melodic slot.
- **Tweak Tone (Knob A = pitch)** — `main/ui/knobs.c` and
  `main/audio/amy_bridge.c`. When tweak mode is `TWEAK_TONE`,
  Knob A adjusts the next note's `midi_note` by ±12 semitones.

### 🚫 What we can't simulate

- **The actual 555 timer chip.** Pearson spends a whole chapter
  teaching what a 555 does — discharge a capacitor through a
  resistor until a threshold, flip the output, charge it again,
  repeat forever. Our firmware skips this entirely; AMY's
  oscillator is software, not silicon.
- **The breadboard experience.** The tactile satisfaction of pushing
  components into a foam board is the most important pedagogical
  moment in Pearson's book, and we have no analog. The closest we
  have is the 4×4 button matrix on the PO-33, which has the same
  "one input, one output, no menus" quality.
- **The Photo Theremin hardware.** No light-dependent resistor on our
  board. (One could be added in v2 hardware. The firmware has a
  GPIO reserved for it.)

---

<a id="chapter-4"></a>
## Chapter 4 — Amps, Reverbs, and Talkboxes

### 📖 What Pearson does

Chapter 4 builds a battery-powered amplifier (the **LM386** chip with a
few capacitors), then uses it to build three projects:

- **A breadboard power amp.** Drives a small speaker.
- **Stereo panning.** A potentiometer splits an audio signal between
  two speakers. Twist left, more in the left speaker; twist right,
  more in the right. Pearson's implementation uses one pot and
  three resistors.
- **A plate reverb.** A metal plate vibrates. Contact microphones
  pick up the vibrations. The output is a recognisable reverb
  sound. This is the "physical reverb" — sound bounces off a
  physical surface, and the surface re-radiates it back.
- **A talkbox.** A small speaker plays audio directly into your
  mouth. You shape the audio with your mouth. A microphone picks up
  the result. The effect is the classic "robot voice" from 1970s
  funk records.

### 🎓 Background: the digital amp, reverb, and pan

In software, an amplifier is a multiplication: every sample gets
multiplied by some gain factor (0..1 for normalise, or higher for
"boost"). A pan pot is two multiplications — one for the left
channel, one for the right. A reverb is a more interesting
operation: a long delay line with feedback, possibly with several
parallel delay lines at different lengths.

AMY's **plate reverb** is the digital analog of Pearson's metal plate.
Instead of vibrating metal, AMY keeps a circular buffer of audio
samples and reads it back at different positions with feedback. The
math is similar: Pearson's metal plate has a "resonance time" of a
few seconds; AMY's reverb has a `feedback` parameter that controls
how long the echoes ring out.

In our firmware, volume is a 5-level scale (F-022: 1..5). Each level
is a float multiplier on the velocity field. Level 5 is full velocity;
level 1 is 20% velocity. This is the closest thing we have to a
volume knob — Pearson's volume pot is our 5-key `BPM + step 1..5`
combo.

### 🔧 Try it on the device

**Exercise 1: change the volume.**

```
> volume 1
> note 9 69
> note 9 72
```

That played A4 and C5 at minimum volume. Now:

```
> volume 5
> note 9 69
> note 9 72
```

Same notes, much louder. The "potentiometer" in Pearson's power amp
is the `volume` verb in our firmware.

**Exercise 2: panning.**

We don't expose a pan knob via UART in v0.6. But our hardware has
**Knob B**, which maps to "Tweak Filter" (Knob A = cutoff, Knob B =
resonance, F-017). When in tweak mode **FILTER**, twisting Knob B
adjusts the **resonance** of the low-pass filter on the next
triggered note. Pearson's stereo pan pot is doing a *bipolar
distribution* between two channels; our Knob B is doing a
*unipolar amplification* of a single filter parameter. Different
shape, same idea — "a knob that splits one signal into two with
different proportions".

**Exercise 3: reverb.**

There is no dedicated reverb verb yet, but the AMY library supports
a global reverb that all sounds run through. To turn it on, we'd
need a UART verb that calls `amy_reverb()` — which we haven't
exposed yet. The closest available approximation is **the
LOOP_16 punch-in effect**, which loops a slice of the played note
and feeds it back on itself — that is, a 1-sample delay with
feedback, which sounds similar to a short reverb tail.

```
> fx LOOP_16
> note 9 69
```

You'll hear the A note, then a soft repeating fragment of it that
fades out. Not exactly a plate reverb, but conceptually adjacent:
a delayed-and-fed-back audio signal is the building block of all
reverb algorithms, including the algorithm that builds AMY's.

### 🛠 Code reference

- **The 5-level volume scale** — `main/audio/amy_bridge.c`,
  `volume_multiplier()`. Levels: 0.20, 0.40, 0.60, 0.80, 1.00.
  This is applied per-note as a velocity scalar.
- **AMY's reverb** — `components/amy/src/amy.c`, `amy_reverb()`
  function. Sets the global reverb level. We have not yet wired
  this to a UART verb.
- **The PCM5102A DAC** — `main/audio/i2s_driver.c` (or its
  successor). Talks to the DAC over I²S, with the DMA feeding
  samples from AMY's render buffer.

### 🚫 What we can't simulate

- **The talkbox.** This requires a plastic tube, a small speaker,
  and a microphone — all hardware, none of which our firmware
  controls. The digital analog would be a **vocoder**, which
  AMY does support (`amy_event.mod_source = ...`), but we
  haven't wired it to a UART verb.
- **Stereo panning.** The PCM5102A outputs stereo, but our
  firmware mixes everything to mono internally. Panning is a
  v2 feature.

---

<a id="chapter-5"></a>
## Chapter 5 — Soldering, Enclosures, and UI

### 📖 What Pearson does

Chapter 5 is where the project stops being a breadboard and becomes an
instrument. Pearson covers:

- **Lab equipment.** What you need on your bench: soldering iron,
  solder, wire strippers, flush cutters, multimeter, helping hands,
  safety glasses.
- **Soldering safety.** Don't breathe the fumes. Don't burn
  yourself. Don't solder a live circuit.
- **The photo theremin.** (Transferred from Ch 3, finished into a
  permanent instrument with a knob and an enclosure.)

### 🎓 Background: our equivalent of "the enclosure"

In our firmware, the "enclosure" is the printed-circuit board + the
3D-printed case + the **TFT screen** + the **4×4 button matrix**.
These are the parts of the device the user sees and touches. The
soldering-iron equivalent for our project is **flashing the
firmware** — connecting the ESP32 over USB and pushing the compiled
binary to it.

Just as Pearson argues that the *enclosure* is what makes a
breadboard feel like an instrument, our project argues that the
*TFT screen* and the *button matrix* are what makes our firmware
feel like a PO-33. Both are true. A blob of code on a microcontroller
that you can only talk to over a serial port is a development kit, not
an instrument. The moment you put it in a case with a screen and
buttons, it becomes an instrument.

The TFT on our device is a 2.4-inch, 240×320-pixel colour display
driven by an ILI9341 chip over SPI. It has 8 named "screens". Only the
first two exist in the current firmware; the rest are the v2 UI
proposal (`docs/DESIGN.md` §6). They are listed so you can see the
design intent:

1. **Main / step grid** — the default screen, showing the 16 steps
   of the active pattern with the current step highlighted. *(implemented)*
2. **Sound-select** — shows 16 slots, indicating which have audio. *(implemented as the sketch picker's slot view)*
3. **Pattern-select** — shows 16 patterns, indicating which have steps. *(v2 proposal)*
4. **Tweak** — shows the current tweak mode (TONE / FILTER / TRIM). *(the tweak label is shown in the main-screen status bar)*
5. **Sketch picker** — long-press WRITE to enter this; lists all
   saved sketches. *(implemented)*
6. **FX picker** — *(not implemented in v1: `FX + step N` selects an effect directly, and `FX` long-press toggles sync IN)*
7. **Clock + alarm** — shows the wall-clock time and the alarm time. *(v2 proposal)*
8. **Battery / status** — a corner overlay always present.

These are the closest thing we have to Pearson's knobs and switches.
Each is a small UI element that maps to a single global setting.

### 🔧 Try it on the device

**Exercise 1: look at the boot screen.**

After reset, the firmware draws the main screen. You'll see:

- A status strip with `P01`, the current BPM, and a clock.
- A 4×4 grid of step buttons, most of them empty (white).
- The word `TONE` or `FILTER` or `TRIM` near the top — the current
  tweak mode.
- A small "BAT 87%" overlay somewhere.

All of these are knobs Pearson would have used a potentiometer to
express. In our firmware they are pixels drawn by
`main/ui/display.c`.

**Exercise 2: cycle through the tweak modes.**

If you have a device, tap `FX`. The mode label changes:
`TONE → FILTER → TRIM → TONE`. This is the same as Pearson's
"three knobs on the front panel" — one knob per mode.

Over UART:

```
> tweak_mode
```

prints the current mode.

**Exercise 3: see the slot screen.**

If you have a device, press `SOUND`. The screen shows the 16
slots; recorded slots have an orange dot; empty slots are dark.

Over UART:

```
> slot_info
```

prints the same data as text.

### 🛠 Code reference

- **The TFT driver** — `main/ui/display.c` and `main/ui/ili9341.c`.
  About 2 000 lines of C. The display is drawn 30 times per second
  via a FreeRTOS task.
- **The button matrix** — `main/ui/buttons.c`. A 4×4 button matrix
  scanned by an I/O expander (likely MCP23017 or PCA9555) over I²C.
- **The 8 screens** — `main/ui/display.c`, function
  `render_screen()`. Dispatches to one of 8 render functions based
  on the current `display_state_t`.

### 🚫 What we can't simulate

- **The soldering experience.** Our project doesn't require you to
  solder anything (unless you build the hardware from scratch, in
  which case you solder the DAC, the TFT, the I/O expander, the
  buttons, and the microphone). For users with a pre-built device,
  this whole chapter is moot.
- **The enclosure.** Our device ships with a 3D-printed case (or no
  case, depending on the kit). The "feel" of the device is set by
  the case design, not the firmware.

---

<a id="chapter-6"></a>
## Chapter 6 — Chaining Oscillators

### 📖 What Pearson does

Chapter 6 is about **polyphony** (multiple oscillators per note, for
richer sounds) and **chaining** (feeding one oscillator's output into
another's input). Projects:

- **Polyphonic square waves.** Mix three 555 oscillators with
  different pitches into one amp. You hear three notes at once.
- **Tremolo.** Modulate the amplitude of one oscillator with a slow
  LFO. The note gets louder and softer rhythmically.
- **The electro-cricket.** A 555 + a 40106 hex inverter + a small
  speaker. The 40106 squares up the 555's rounder waveform and
  adds harmonic richness.
- **The undertone.** A subharmonic generator. Plays notes below
  the audible range (felt more than heard).
- **Variations:** The Atari Punk Candle (a light-controlled 555),
  The Gating Oscillator (turns sound on/off rapidly), The Vactrol
  Arpeggiator (a sequence of pitches using a vactrol as the
  variable resistor).

### 🎓 Background: polyphony, modulation, and the AMY way

**Polyphony** in our firmware is the ability to play multiple
voices of the same sound at once. AMY allocates voices per-slot: 1
voice for drum slots (you can't have two kicks at once), and 4 voices
for melodic slots (you can play a four-note chord). This is set in
`amy_bridge_play_note()`:

```
e.num_voices = (slot < SLOT_DRUM_COUNT) ? 1 : VOICE_COUNT;
```

**Tremolo** in our firmware is most closely approximated by the
**STUTTER_4** punch-in effect, which rapidly triggers a single
note on and off. Or, more cleanly, by **RETRIGGER_PATTERN**, which
re-triggers the whole step sequence at a fast rate.

**The Atari Punk Console** — Pearson names this gem (a 555 + 555 +
pot + light sensor + speaker) but doesn't formally build it. Our
firmware has no direct analog. The closest is the **Knob A** (when
in tweak Tone mode) — a continuously-variable input that affects
the next note.

**The Vactrol Arpeggiator** — a sequencer that walks through a
series of pitches, where each pitch is set by a vactrol (a
light-controlled resistor). Our firmware's analog is **a pattern of
recorded slots**, where each step plays a different sample at a
different pitch. The pattern plays as a sequence. The "vactrol"
becomes "the slot chosen for step N".

**The Undertone** — a circuit that generates frequencies below
the audible range, creating a tactile "rumble" you feel rather than
hear. Our firmware has no direct analog. AMY's frequency range
extends down to 0.1 Hz, so very slow LFO rates are supported, but
they are pitched above 0, not below.

### 🔧 Try it on the device

**Exercise 1: 4-voice polyphony.**

Pick slot 9 (or any melodic slot) and trigger four notes in rapid
succession:

```
> note 9 60
> note 9 64
> note 9 67
> note 9 72
```

You should hear a C-major chord (C, E, G, C). Because each note
allocates a new voice from the 4-voice pool, you can keep adding
notes. The fifth one evicts the oldest. This is Pearson's
"polyphonic square waves", with C-major instead of random pitches.

**Exercise 2: tremolo via punch-in effect.**

```
> fx STUTTER_4
> note 9 60
```

The C note will rapidly retrigger. That is *exactly* the gate
oscillator — a square wave that turns the note on and off. Pearson
called this "the gating oscillator"; we call it STUTTER_4.

**Exercise 3: the vactrol arpeggiator via a pattern.

Write a 4-step pattern with different pitches on each step:

```
> write_on
> pattern_clear P01
> step_set P01 1 9 60
> step_set P01 2 9 64
> step_set P01 3 9 67
> step_set P01 4 9 72
> write_off
> bpm 120
> play
```

The pattern plays C, E, G, C in sequence. That is the vactrol
arpeggiator, with the "vactrol" replaced by the slot/pitch
selection on each step. Pearson's circuit walks through pitches
using hardware; our firmware walks through pitches using RAM.

### 🛠 Code reference

- **Polyphony** — `main/audio/amy_bridge.c`, `amy_bridge_play_note()`.
  `e.num_voices` controls how many AMY voices are allocated per slot.
- **STUTTER_4** — `main/audio/amy_bridge.c`, `apply_fx()`,
  `case PO33_FX_STUTTER_4:`. Triggers the note 4 times in quick
  succession.
- **Pattern step data** — `main/sequencer/pattern.h`, `pattern_step_t`.
  Each step stores a `slot` and a `note` (MIDI pitch).

### 🚫 What we can't simulate

- **Subharmonic generation (the Undertone).** Generating frequencies
  below the audible range requires either analog circuits that
  modulate the supply voltage (the literal "undertone") or digital
  techniques like downsampling with a low-pass filter. Our
  firmware does neither.
- **The 40106 hex inverter as a tone shaper.** The 40106 is a
  digital chip that, in this context, squares up a softer waveform
  into a harder one. Our firmware has no equivalent "wave squarer".

---

<a id="chapter-7"></a>
## Chapter 7 — Schematics and Mass Transit

### 📖 What Pearson does

Chapter 7 is short and pedagogical. Pearson teaches you to **read a
schematic** — the standardised diagram of a circuit. He uses the
Rotterdam Metro as an analogy: the same colour-coded lines that mean
"this train goes to Central Station" also mean "this wire carries
the audio signal". A schematic is a metro map for electrons.

This is a "no-project" chapter. It exists to teach literacy. The
reward is the next chapter's projects become readable.

### 🎓 Background: how to read our block diagrams

Our project's `design document / docs/DESIGN.md` has block diagrams
showing how firmware components talk to each other. The most useful
mental model:

```
+----------+   +---------+   +---------+
| 4x4 BTN  +-->| input.c +-->| sequencer|
+----------+   +---------+   +---------+
                                  |
                                  v
                              +--------+
                              | AMY    |
                              +--------+
                                  |
                                  v
                              +-------+
                              | I2S TX|
                              +-------+
                                  |
                                  v
                              [DAC PCM5102A]
                                  |
                                  v
                              [headphones]
```

Reading a block diagram is exactly Pearson's reading a schematic,
except instead of "resistor", "capacitor", "555", we have "input",
"sequencer", "AMY". The arrows mean "data flows this way". The
rectangles mean "subsystem". Same idea, different vocabulary.

### 🔧 Try it on the device

This chapter has no exercises. It is a literacy lesson.

If you want to test your new literacy, open `docs/DESIGN.md` §3 in a
text editor and read the first block diagram. Identify:

- One module that produces data (probably `sequencer_tick()`).
- One module that consumes data (probably `amy_add_event()`).
- One arrow that goes the "wrong way" (a callback, an event
  subscription, an ISR).

If you can identify those three things, you have read your first
schematic.

### 🛠 Code reference

- **The block diagrams** — `docs/DESIGN.md` §3, throughout.
  Hand-curated, ASCII-art.
- **The actual code that flows** — every `.c` file in `main/` has
  its dependencies at the top:
  ```c
  #include "freertos/FreeRTOS.h"
  #include "sequencer.h"
  #include "amy_bridge.h"
  ```
  The `#include` lines are the wires. The functions called inside
  the file are the signals travelling along those wires.

### 🚫 What we can't simulate

Schematics are a *literacy* lesson. There is nothing to simulate —
only to read.

---

<a id="chapter-8"></a>
## Chapter 8 — Filters

### 📖 What Pearson does

Chapter 8 is about filters — circuits that pass some frequencies and
block others. The three main types:

- **Low-pass filter (LPF).** Lets the low frequencies through,
  attenuates the highs. The "bass boost" on a hi-fi is an LPF.
- **High-pass filter (HPF).** Lets the highs through, attenuates
  the lows. The "telephone" EQ on a voice (or the wah pedal on a
  guitar) is an HPF.
- **Band-pass filter (BPF).** Lets a band of frequencies through,
  attenuates everything else. A radio tuner is a BPF.

Pearson distinguishes **passive filters** (made of resistors and
capacitors only, no power) from **active filters** (which add an
op-amp like the **LM741** to amplify the signal). Active filters are
sharper and more controllable.

Projects:

- The passive low-pass filter.
- The passive high-pass filter.
- The active high-pass filter.
- The active low-pass filter.
- The active band-pass filter.

### 🎓 Background: filters in software

A digital filter takes the incoming stream of audio samples and
transforms it. The three main types are:

- `FILTER_LPF` — low-pass.
- `FILTER_HPF` — high-pass.
- `FILTER_BPF` — band-pass.

AMY's filter, like Pearson's op-amp-based filter, has two controls:

- **`filter_freq`** — where the cutoff happens (in Hz).
- **`filter_resonance`** — how aggressive the peak is at the cutoff
  (the "Q" factor).

In our firmware, the equivalent of "twist the cutoff knob on a
Moog" is **Knob A in tweak FILTER mode** — it adjusts `filter_freq`.
The equivalent of "twist the resonance knob" is **Knob B in tweak
FILTER mode** — it adjusts `filter_resonance`.

### 🔧 Try it on the device

**Exercise 1: low-pass filter sweep.**

Pick a melodic slot and trigger a note with different cutoff values:

```
> tweak_filter 10 0
> note 9 60
> tweak_filter 100 0
> note 9 60
> tweak_filter 200 0
> note 9 60
```

The first note is filtered very low — almost no high frequencies.
The second note lets more highs through. The third is fully open.
This is Pearson's "sweep the cutoff knob" demo.

**Exercise 2: with resonance.**

```
> tweak_filter 50 100
> note 9 60
```

That sets cutoff to 50 (~1570 Hz) and resonance to 100. You'll
hear a peak in the spectrum around 1.5 kHz. This is the "wah" — a
low-pass filter with high resonance, played around a fixed cutoff.
Move `cutoff` to 30, 200, 250 to hear different "wah" sounds.

**Exercise 3: high-pass filter via FX.**

The PO-33 has a punch-in effect called **FILTER_SWEEP** that is
explicitly meant to drive the filter. With this FX active:

```
> fx FILTER_SWEEP
> note 9 60
```

the filter sweeps up over the duration of the note, opening from
closed to fully open. That is the "filter sweep" sound from
thousands of dance records.

### 🛠 Code reference

- **The tweak-filter UI** — `main/ui/knobs.c`. The two ADCs on
  ESP32-S3 GPIO 2 (Knob A) and GPIO 46 (Knob B) read 0..4095 and
  scale to 0..255.
- **The filter application** — `main/audio/amy_bridge.c`,
  `amy_bridge_play_note()`. Lines around `filter_freq`,
  `filter_resonance`, `filter_type` set the AMY filter struct.
- **The FILTER_SWEEP FX** — `main/audio/amy_bridge.c`,
  `apply_fx()`, `case PO33_FX_FILTER_SWEEP:`. Triggers an automated
  sweep over the duration of the note.

### 🚫 What we can't simulate

- **The math.** Pearson explains why filters work in terms of RC
  time constants and the Fourier transform. We don't dive into
  the math here; we trust AMY's filter to be correct. The reader
  who wants the math should read Pearson's chapter 7 (and probably
  some Wikipedia on biquad filters).
- **The active filter circuits.** The op-amp-based active filter is
  a circuit, not a math formula. Our firmware has no analog of
  "supply the op-amp with ±12 V"; AMY's filter is digital and
  operates on 16-bit integers.

---

<a id="chapter-9"></a>
## Chapter 9 — Harmonization

### 📖 What Pearson does

Chapter 9 builds circuits that "harmonize with themselves" — i.e.,
take one input signal and produce one or more additional signals
that are musically related to the input. Projects:

- **Octave harmonizer.** Plays the same note one octave up.
- **Subharmonic generator.** Plays the same note one octave down.
- **10-stage wavetable generator.** Plays a sequence of pitches
  chosen from a 10-element "wavetable". This is a digital technique
  using the 4017 decade counter.
- **The FM Yodeler.** Frequency-modulates one oscillator with
  another at audio rate, producing the "yodel" sound.

### 🎓 Background: harmonization in software

In our firmware, the closest analog of "play the same note one
octave up" is the **`note` verb with a different MIDI number**:

```
> note 9 60
> note 9 72
```

MIDI 60 is C4; MIDI 72 is C5. That is one octave up. Same idea
as Pearson's octave harmonizer; different math.

Frequency modulation in our firmware is **AMY's FM synth**:

```
amy_event e = amy_default_event();
e.synth = 1;        /* FM synth */
e.patch_number = 1; /* patch 1: bell-like FM */
e.frequency = 440.0;
e.midi_note = 60;
amy_add_event(&e);
```

That makes a bell-ish FM tone. The "yodel" character comes from
modulating frequency with another oscillator. AMY exposes this
through `mod_source` and `mod_target` fields. We have not wired
this to a UART verb yet, but the AMY primitive is there.

### 🔧 Try it on the device

**Exercise 1: octave up.**

```
> note 9 60
> note 9 72
```

You'll hear the same note one octave apart. That is the simplest
"harmonizer" in the world.

**Exercise 2: octave down.**

```
> note 9 72
> note 9 60
```

Same, but down.

**Exercise 3: full chord.**

Play the C, E, G, B notes of a Cmaj7 chord:

```
> note 9 60
> note 9 64
> note 9 67
> note 9 71
```

That's a 4-note chord. The firmware handles the polyphony.

**Exercise 4: load a different waveform via FX.**

```
> fx SCRATCH
> note 9 60
```

`SCRATCH` (when fully implemented) loads a different AMY patch.
Different patches sound different — one might be a sine, another
a saw, another a complex FM sound. This is the closest analog to
Pearson's "10-stage wavetable generator": a small set of named
sounds you can step through.

### 🛠 Code reference

- **The note verb** — `main/main.c`, `cmd_note()`. Maps MIDI
  numbers to AMY frequencies (440 × 2^((note - 69) / 12)).
- **AMY FM synth** — `components/amy/src/amy.c`, function
  `amy_event_set_osc()`. Sets up FM operators.
- **Scratch FX** — `main/audio/amy_bridge.c`, `apply_fx()`,
  `case PO33_FX_SCRATCH:`. (Note: this case is currently a no-op
  in v0.6; the AMY infrastructure is in place but the verb is
  not wired yet.)

### 🚫 What we can't simulate

- **Subharmonic generation.** Halving the playback speed of a
  recorded sample is one way; our firmware doesn't expose this.
- **Wavetable synthesis.** AMY's oscillator engine is
  subtractive/additive, not wavetable. A wavetable oscillator is
  on the AMY roadmap but not currently shipped.
- **FM Yodeler.** The AMY FM patch is usable but is not wired to
  a UART verb. The closest is the implicit direct "FUTURE" plan
  in `TODO.md`.

---

<a id="chapter-10"></a>
## Chapter 10 — Modulation

### 📖 What Pearson does

Chapter 10 builds a family of circuits that **modulate** other
circuits. Modulation means: take a control signal (often from an
LFO, envelope, or other source) and use it to vary a parameter of a
sound-generating circuit in real time. Projects:

- **The Vantastic Vactrol.** A light-controlled variable resistor
  built from an LED and a light-dependent resistor. The brighter
  the LED, the more current through the LDR.
- **Pulse-width modulation.** Vary the duty cycle of a square wave.
  The "PWM" knob on a Moog synth does this.
- **Pulse-width modulation with vactrols.** Combine the two above.
- **A button-controlled VCA.** A voltage-controlled amplifier whose
  gain is set by a button press.
- **An LFO-pingable VCA.** Like the above, but the gain is set by
  a slow LFO.
- **The piezo drum trigger.** A piezo element that, when struck,
  produces a short trigger pulse that fires an envelope.
- **Ring modulation.** Multiply two audio signals. The result has
  sum-and-difference frequencies. The "Dalek voice" effect.
- **Stereo tremolo.** A stereo modulator: the left and right
  channels are modulated by LFOs that are out of phase.

### 🎓 Background: modulation as a software concept

In code, modulation is **a parameter that changes over time**.
There are three main kinds:

- **LFO modulation.** A slow oscillator (sub-audio frequency) that
  drives a parameter. Example: an LFO at 4 Hz modulates filter
  cutoff. The filter "wobbles" at 4 Hz.
- **Envelope modulation.** A one-shot rise-and-fall shape. Example:
  an envelope generator produces a quick attack and a slow decay.
  The amplitude of a note follows that envelope.
- **Step modulation.** A pre-set sequence of values. Example: a
  pattern of pitches (this is the sequencer!).

AMY supports all three. **LFOs** are exposed via `amy_event.mod_source`
and `mod_target` fields. **Envelopes** are exposed via `bp0_times`
and `bp0_values`. **Step modulation** is what our sequencer does
internally — every step is a discrete jump to a new value.

### 🔧 Try it on the device

**Exercise 1: ring modulation via FX.**

The PO-33 has a punch-in effect called **RING_MOD** (when fully
implemented). Ring modulation multiplies the slot with a slow sine
wave, producing sum-and-difference frequencies. Try:

```
> fx RING_MOD
> note 9 60
```

You should hear a metallic, bell-like tone with a slightly detuned
quality. That is the ring modulator at work.

(As of v0.6, the RING_MOD case in `apply_fx()` is a no-op. The
infrastructure is in place; the wiring is the missing piece.)

**Exercise 2: pulse-width modulation via FX.**

```
> fx PULSE_WIDTH
> note 9 60
```

When wired, this will change the duty cycle of the underlying
square wave oscillator. A 50% duty cycle sounds bright; a 10% duty
cycle sounds thin and "talkbox-y".

**Exercise 3: tremolo via FX.**

```
> fx STUTTER_4
> note 9 60
```

The note rapidly retriggers 4 times. This is the *gating* version
of tremolo. For a smoother tremolo, we'd need an LFO driving the
amplitude of the note — that is supported in AMY but not wired to a
verb.

**Exercise 4: an envelope on every note.**

Every `note` event has a built-in envelope. AMY's default envelope
is "fast attack, medium decay, sustain at half". It's what makes a
note sound like a *note* rather than a drone. Try:

```
> note 9 60
> note 9 60
> note 9 60
```

You hear the same note three times, each with the default envelope.
That envelope is the closest analog of Pearson's envelope generator
— implemented in software.

### 🛠 Code reference

- **LFO modulation** — AMY's `amy_event` struct has `mod_source`
  and `mod_target`. We have not yet exposed this via UART.
- **Envelope** — AMY's `bp0_times[]` and `bp0_values[]`. Default
  envelope is set in `amy_default_event()`.
- **Ring mod FX** — `main/audio/amy_bridge.c`, `apply_fx()`,
  `case PO33_FX_RING_MOD:`. Currently a no-op; the AMY code is
  there (`awaiting_factor` and similar).

### 🚫 What we can't simulate

- **The vactrol.** Pearson's light-controlled resistor is a beautiful
  analog of a knob that turns itself. We have no light sensor on
  our device. The closest is the I²S mic, which is a "sound-pressure
  controlled gain" — not the same.
- **The piezo drum trigger.** A piezo's impulse response is fast
  (sub-millisecond) and short. Our INMP441 mic has its own AGC
  (automatic gain control) that smooths impulses. They are not
  the same.

---

<a id="chapter-11"></a>
## Chapter 11 — Sequencers

### 📖 What Pearson does

Chapter 11 is the second-biggest chapter in the book, after Chapter 3.
Pearson teaches you to build **step sequencers** — circuits that
play a sequence of notes in a loop. The chapter introduces:

- **The 4051 multiplexer.** A chip that selects one of 8 inputs
  based on a 3-bit address. Used as the heart of an 8-step sequencer.
- **Boolean logic.** AND, OR, NOT — the basics of how digital
  circuits "decide" things.
- **Binary and the 4051.** How to set the 3 address bits using
  binary counting.

Projects (in increasing complexity):

- **The Disco Boole (Part I).** A 2-step sequencer using a 4017
  decade counter.
- **The Disco Boole (Part II).** Adding LEDs to the previous so you
  can see which step is active.
- **The Standard Eight-Step Sequencer.** The canonical project.
  Eight steps, each with a pitch knob and a trigger.
- **A Pattern-Changing Sequencer.** A sequencer that switches between
  two patterns based on an external input.
- **A First-Order Reset Harmonization Sequencer.** A sequencer that
  auto-resets based on the output of another sequencer.
- **A Second-Order Reset Harmonization Sequencer.** Like the above,
  but the cascaded resets create complex interlocking patterns.

### 🎓 Background: our sequencer, end to end

Our firmware has a sequencer that does almost everything Pearson's
circuits do, plus more. It is implemented in `main/sequencer/sequencer.c`
and `main/sequencer/pattern.{c,h}`. The key data structures:

```
#define PATTERN_COUNT  16
#define STEP_COUNT     16
#define SLOT_COUNT     16
#define CHAIN_LENGTH   128

typedef struct {
    uint8_t slot;            // which slot to play on this step
    uint8_t note;            // MIDI note number, 0..127 (0 = step off)
    uint8_t fx;              // punch-in effect for this step
    uint8_t fx_p1, fx_p2;    // effect params
    uint8_t filter_cutoff;   // tweak-filter cutoff, 0..255
    uint8_t filter_resonance;// tweak-filter resonance, 0..255
} pattern_step_t;

struct pattern_t {
    pattern_step_t steps[STEP_COUNT];
};

struct pattern_t patterns[PATTERN_COUNT];
uint8_t chain[CHAIN_LENGTH];
uint8_t chain_len;
```

When the sequencer ticks, it:

1. Looks up the current step in the active pattern.
2. If `note == 0`, the step is empty — do nothing.
3. Otherwise, call `amy_bridge_play_note(step.slot, step.note,
   step.fx, step.fx_p1, step.fx_p2, step.filter_cutoff,
   step.filter_resonance)`.
4. AMY plays the slot at the given pitch with the given effect and
   filter.
5. Advance to the next step. If we've gone past step 16, jump back
   to step 1 (or move to the next pattern in the chain).

This is Pearson's Standard Eight-Step Sequencer, with 16 steps
instead of 8, no knob per step (we have UART verbs instead), and a
chain instead of a single loop.

### 🔧 Try it on the device

This chapter has the most exercises. Take your time — the sequencer
is the heart of the device.

**Exercise 1: the Standard Sixteen-Step Sequencer.**

Clear pattern 1 and put four notes on steps 1, 5, 9, 13:

```
> pattern_clear 1
> step_set 1 1 9 60    ; step 1 plays slot 9 at MIDI 60 (C4)
> step_set 1 5 9 64    ; step 5 plays slot 9 at MIDI 64 (E4)
> step_set 1 9 9 67    ; step 9 plays slot 9 at MIDI 67 (G4)
> step_set 1 13 9 72   ; step 13 plays slot 9 at MIDI 72 (C5)
> bpm 120
> play
```

You should hear a 4-note pattern: C, rest, E, rest, G, rest, C,
rest, repeat. That is the Standard Sixteen-Step Sequencer —
16 steps, each with a slot and a pitch.

If you don't have a recorded slot 9, you can still play — AMY's
default synth will provide a patch on the fly.

**Exercise 2: the Pattern-Changing Sequencer.**

Add another pattern with different pitches:

```
> pattern_clear 2
> step_set 2 1 9 67
> step_set 2 5 9 72
> step_set 2 9 9 76
> step_set 2 13 9 79
```

Append both patterns to the chain:

```
> chain_clear
> chain_append 1
> chain_append 2
> chain_show
```

The chain now reads `[1, 2]`. When you press play, pattern 1 plays
once, then pattern 2 plays once, then the chain wraps back to
pattern 1. That is Pearson's Pattern-Changing Sequencer — the
hardware version uses an external input to switch patterns; ours
uses a chain.

**Exercise 3: chained reorder (live re-ordering).**

While the sequencer is playing, swap the two patterns in the chain:

```
> chain_swap 0 1
```

You should hear the order reverse immediately. That is Pearson's
"live re-arrangement" — possible in hardware only by physically
moving wires; possible in our firmware via two keystrokes.

**Exercise 4: the First-Order Reset Sequencer (chain auto-reset).**

If you build a chain of patterns where the last pattern is the
"reset trigger" pattern, the chain wraps back to pattern 1 after
the last. That is the simplest form of Pearson's auto-reset
sequencer. Our firmware does this automatically:

```
> chain_clear
> chain_append 1
> chain_append 2
> chain_append 1   ; explicitly loop back
> chain_append 2
> play
```

Plays 1, 2, 1, 2 forever. Or use the `loop_chain` option (if your
firmware version supports it) to auto-loop without explicit
appends.

**Exercise 5: the Disco Boole (binary counter).**

The Disco Boole is a sequencer that uses a binary counter to
choose the step. Our firmware does this implicitly — the sequencer
walks through steps 1, 2, 3, ... which is just a binary counter
that increments on each clock tick. No special verb needed.

**Exercise 6: cascade (Second-Order Reset Harmonization).**

Two chains, one driving the other. We don't expose this directly,
but you can approximate it with a long chain of alternating
patterns:

```
> chain_clear
> chain_append 1
> chain_append 2
> chain_append 1
> chain_append 2
> chain_append 1
> chain_append 2
```

The pattern 1 and 2 will play alternately, creating a "harmonized"
effect because they share the same step data but at different
pitches.

### 🛠 Code reference

- **The pattern data structure** — `main/sequencer/pattern.h`.
  `pattern_step_t` and `pattern_t`. About 50 lines of declarations.
- **The sequencer tick** — `main/sequencer/sequencer.c`,
  `sequencer_tick()`. About 60 lines. Pulls the current step from
  the active pattern, calls `amy_bridge_play_note()`, advances the
  step counter.
- **The chain** — `main/sequencer/sequencer.c`. `chain[]`,
  `chain_len`, `chain_append()`, `chain_remove()`, `chain_swap()`,
  `chain_insert()`.
- **The UART verbs** — `main/main.c`. Each shell command
  (`cmd_chain_append`, `cmd_chain_swap`, etc.) is about 15 lines.

### 🚫 What we can't simulate

- **The 4051 multiplexer chip.** It's a piece of silicon that
  selects one of 8 inputs based on a 3-bit address. Our firmware
  has no analog; the sequencer just walks an array index.
- **Boolean logic gates.** Pearson uses discrete logic gates to
  decide "should this step fire?" Our firmware uses C `if`
  statements, which compile to conditional branches on the CPU.
  Same thing.

---

<a id="chapter-12"></a>
## Chapter 12 — Electronic Percussion

### 📖 What Pearson does

Chapter 12 builds a **drum machine** out of analog oscillators.
Sub-sections:

- **A brief history of drum machines.** From the Rhythmicon (1930s)
  to the Roland TR-808 (1980) to the SP-12, MPC, etc.
- **Making inharmonic sounds.** Sounds whose frequencies are not
  integer multiples of a fundamental. (Most acoustic instruments are
  inharmonic to some degree.) Pearson builds:
  - **Tambourines** — short bursts of inharmonic noise.
  - **Crash cymbals** — long inharmonic noise with a sharp attack.
- **Making harmonic sounds.** Sounds whose frequencies are integer
  multiples of a fundamental. Pearson builds:
  - **Kick drums** — a low sine wave with a quick pitch envelope.
  - **Tom-toms** — a mid sine wave with a slower envelope.
- **Making noise.** Pure noise. Pearson builds:
  - **Snare drums** — bandpass-filtered noise.
- **Percussion sequencing.** A drum machine with multiple voices
  driven by a sequencer. Examples:
  - **Bomba** — a simple 3-voice pattern.
  - **Bossa Nova** — a complex 4-voice pattern.

The chapter leans on the **40106 hex inverter** chip, which is
both an oscillator (when used with RC timing) and a logic inverter.

### 🎓 Background: drum slots vs melodic slots

Our firmware has a hard split:

- **Slots 1–8** are drum slots. Monophonic (one voice each). When
  triggered, they play a recorded sample at fixed pitch.
- **Slots 9–16** are melodic slots. 4-voice polyphonic each.
  When triggered, they play at the requested pitch.

Drum slots are perfect for the kinds of sounds Pearson builds —
kick, snare, hat, tom, cymbal. They are recorded from a microphone
(or synthesised offline). They play at one pitch. They are triggered
in patterns.

Our firmware **does not synthesise** drum sounds from scratch. You
record or upload a sample, then trigger it. That is a different
design choice than Pearson's. It is closer to the original PO-33's
design — sample-based drums, oscillator-based melody — than to
Pearson's drum-machine chapters.

### 🔧 Try it on the device

**Exercise 1: record a kick drum.**

If you have a real kick-drum sample (or a YouTube video of one),
play it near the device's microphone and record:

```
> record_mic 1 1   ; record 1 second into slot 1
> slot_play 1
```

You'll hear the recorded kick.

**Exercise 2: record a snare, kick, hat.**

```
> record_mic 2 1   ; snare to slot 2
> record_mic 3 1   ; hat to slot 3
```

**Exercise 3: build a 4-on-the-floor pattern.**

```
> pattern_clear 1
> step_set 1 1 1 1   ; step 1: slot 1 (kick) at velocity 1
> step_set 1 5 2 1   ; step 5: slot 2 (snare) at velocity 1
> step_set 1 9 1 1   ; step 9: slot 1 (kick)
> step_set 1 13 2 1  ; step 13: slot 2 (snare)
> bpm 120
> play
```

A basic kick-snare pattern. (See the note below about the step_set
syntax — depending on your firmware version it might be
`step_set <pattern> <step> <slot> <note>` or
`step_set <pattern> <step> <slot> <velocity>`.)

**Exercise 4: add a hi-hat on every off-beat.**

```
> step_set 1 3 3 1
> step_set 1 7 3 1
> step_set 1 11 3 1
> step_set 1 15 3 1
```

Now you have kick on 1, hat on 3, snare on 5, hat on 7, kick on 9,
hat on 11, snare on 13, hat on 15 — the classic "four-on-the-floor"
with snare on 2 and 4.

**Exercise 5: layer two slots on one step.**

You can't currently play two slots on one step through the standard
sequencer, but you can fake it by recording slot 1 and slot 2 into
the *same* buffer (via a separate command, if your firmware
supports it). v2 will have explicit layering.

### 🛠 Code reference

- **Slot types** — `main/audio/amy_bridge.h`. `SLOT_COUNT` is 16;
  `SLOT_DRUM_COUNT` is 8. The split is at index 8.
- **Recording to a slot** — `main/audio/amy_bridge.c`,
  `amy_bridge_record_mic()`. The function allocates PSRAM,
  streams from I²S RX into the buffer, registers with AMY.
- **Pattern step data** — `main/sequencer/pattern.h`,
  `pattern_step_t`. The `slot` field is 0..15; 0 means the step
  is empty.

### 🚫 What we can't simulate

- **Synthesis of drum sounds.** We don't have a `kick_drum()`
  function that makes a kick from a sine wave with a pitch envelope.
  You have to record or upload a sample.
- **The 40106 oscillator-based drum sounds.** Pearson's drum circuits
  are hardware oscillators with envelope generators. Our firmware
  has nothing of the kind — drum slots are sample players, full
  stop.

---

<a id="chapter-13"></a>
## Chapter 13 — Phase-Locked Loops

### 📖 What Pearson does

Chapter 13 builds circuits that lock the frequency of one oscillator
to a multiple or fraction of another. The big chip is the **4046
PLL** (phase-locked loop). Sub-projects:

- **The 4046 VCO.** A voltage-controlled oscillator.
- **Portamento, glide, and glissando.** Slowly sliding between two
  pitches.
- **VCO switching.** Switching between multiple VCOs.
- **Sample and hold.** Freezing a changing control voltage to a
  constant.
- **LFO multiplication.** Using a PLL to make a slow LFO.
- **Audio-frequency multiplication.** Using a PLL to make a fast
  oscillator.

### 🎓 Background: PLLs in software

PLLs are analog circuits that lock an oscillator's phase to a
reference signal. They are how analog synths tune their oscillators
to other instruments or to a master clock. In software, a PLL is
just a **frequency-locked loop** — a control algorithm that
adjusts an oscillator's frequency until its phase matches a
reference.

Our firmware does not implement PLLs explicitly. The closest
analog is the **jam-sync feature** (F-031): when sync-in is
enabled, the sequencer advances on each incoming sync pulse,
which keeps it phase-locked to the master device. There is no
"fractional" sync — one pulse is one step. Pearson's PLL
multipliers and dividers are not implemented.

Portamento (smoothly sliding between two pitches) is approximated
by AMY's `slew_time` field, which can be set on a per-event
basis. We have not wired this to a UART verb.

### 🔧 Try it on the device

**Exercise 1: jam-sync as a simple PLL.**

If you have another PO-33 (real or otherwise) running, connect a
cable from its sync OUT to our device's sync IN. Long-press FX
for 2 seconds to enable sync-in listening:

```
> sync_in_toggle
```

Now when you press play on our device, it will wait for sync
pulses from the master. Each pulse advances one step. That is a
PLL with multiplier 1.

**Exercise 2: pitch glide via FX.**

The PO-33 has a punch-in effect called **PORTAMENTO** (when
fully implemented). Set it:

```
> fx PORTAMENTO
> note 9 60
> note 9 72
```

The note should slide from C4 to C5 rather than jumping. (As of
v0.6, the PORTAMENTO case in `apply_fx()` is a no-op.)

### 🛠 Code reference

- **Sync-in listener** — `main/system/sync.c`. `sync_in_task()`.
  Blocked on a semaphore; wakes per pulse; calls
  `sequencer_tick()`.
- **Sync-in toggle** — `main/ui/input.c`, end-of-drain polling
  block. Long-press FX for 2 seconds toggles
  `sequencer_set_sync_in_active()`.

### 🚫 What we can't simulate

- **The 4046 PLL chip.** No analog here. PLL math in software is
  trivial (an integer-N counter), but the analog version has
  subtle behaviour (lock range, capture range, hold range) that
  no digital emulator captures without effort.
- **VCO switching.** No equivalent.
- **Sample and hold.** Could be done in software but is not.

---

<a id="chapter-14"></a>
## Chapter 14 — The Dogbotophone MK1

### 📖 What Pearson does

Chapter 14 is the boss fight. It walks you through building the
**Dogbotophone MK1** — a "massive electronic orchestra complete with
multiple voices, drums, and self-patching sequencers that compose on
the fly". The build has **15 numbered steps**:

1. **An audio-rate oscillator.** A 4093-based square-wave VCO.
2. **A sequencer clock.** A 4040 binary counter, dividing the VCO into
   three square waves at three octaves of frequency. These drive the
   three sequencers.
3. **A 4017 overtone synth.** An overtone synth that can chirp out
   melodies that can't play out of tune.
4. **A 16-step sequencer for the overtone synth.** Walks through 16
   steps driven by the 4040's three octaves of binary.
5. **A 4017 undertone synth.** Plays undertones — frequencies below
   the audible range.
6. **A 16-step sequencer for the undertone synth.** Like step 4.
7. **A 16-step sequencer for drums.** Triggers the drum voices.
8. **A tunable square wave.** A 4046-based square wave VCO, played
   manually.
9. **Twin-T voice 1.** A harmonic drum voice, tunable from kick to
   tom to woodblock.
10. **Twin-T voice 2.** A second harmonic drum voice.
11. **A cymbal (XOR voice).** An inharmonic cymbal built from a 40106
    (six oscillators) + a 4070 (four XOR gates) + a diode VCA.
12. **Diode AND-gate sequencer reset.** A logic gate that resets the
    drum sequencer.
13. **An optocoupler-controlled sequencer.** (Optional.) Uses an
    optocoupler to trigger the sequencer.
14. **(Optional) Active high-pass and low-pass filters.** Sends voices
    through filters.
15. **An op-amp mixer.** Mixes all six voices.

The chapter concludes: "If you've made it this far, congratulations! Your
room is likely a mess of wires, but you've done it."

### 🎓 Background: the Dogbotophone vs. our device

The Dogbotophone MK1 is **the closest analog to our project** in
the entire book. It has:

- Multiple oscillators (analog) ↔ multiple AMY voices (digital).
- Three 16-step sequencers (hardware) ↔ our chain of patterns
  (software).
- Two sequenced melodic voices (overtone + undertone synths) ↔ our
  melodic slots.
- Three drum voices (Twin-T + cymbal) ↔ our drum slots.
- A tunable square wave (the 4046) ↔ our tweak Tone + Knob A.
- A mixer ↔ AMY's built-in voice allocator.
- Active filters ↔ our tweak Filter mode + punch-in FILTER_SWEEP.
- Optocouplers ↔ no analog.

Our firmware does almost all of what the Dogbotophone does, with
much less soldering.

### 🔧 Try it on the device

The complete Dogbotophone experience, in our firmware, can be
built up over a few minutes:

**Step 1: three sequencers.**

Create three patterns, each with a different "instrument":

```
> pattern_clear 1
> step_set 1 1 9 60
> step_set 1 5 9 64
> step_set 1 9 9 67
> step_set 1 13 9 72
> pattern_clear 2
> step_set 2 1 10 67
> step_set 2 5 10 71
> step_set 2 9 10 74
> step_set 2 13 10 79
> pattern_clear 3
> step_set 3 1 11 60
> step_set 3 5 11 65
> step_set 3 9 11 67
> step_set 3 13 11 72
```

Three patterns, each with its own slot (9, 10, 11), each with
its own melodic content. Now chain them:

```
> chain_clear
> chain_append 1
> chain_append 2
> chain_append 3
> bpm 120
> play
```

You hear three patterns play in sequence. That is the three
sequencers of the Dogbotophone, in software.

**Step 2: the overtone synth.**

The overtone synth plays melodies that "can't play out of tune" —
they are harmonics of a fundamental. In our firmware, you can
approximate this by recording a harmonic-rich sample (e.g. a
square wave) and using `slot_play` on a melodic slot.

**Step 3: the drum voices.**

Record three drum samples (kick, snare, hat) into slots 1, 2, 3.
Write a pattern that triggers them on different beats:
```
> pattern_clear 4
> step_set 4 1 1 1
> step_set 4 3 3 1
> step_set 4 5 2 1
> step_set 4 7 3 1
> step_set 4 9 1 1
> step_set 4 11 3 1
> step_set 4 13 2 1
> step_set 4 15 3 1
```

This is the Bossa Nova example from Pearson's chapter 12 — but
applied to our project.

**Step 4: the mixer.**

AMY automatically mixes all six voices (3 melodic patterns + 1
drum pattern). You don't have to wire a mixer.

**Step 5: the active filters.**

Add tweak-filter values to your melodic steps:
```
> edit
> tweak_filter_set 1 1 50 100
> tweak_filter_set 2 5 100 50
> tweak_filter_set 3 9 200 0
> write_off
```

When the patterns run, the filter sweeps each step differently.
That is Pearson's "active high-pass and low-pass filters on
individual voices".

### 🛠 Code reference

- **The pattern step data with filter values** —
  `main/sequencer/pattern.h`, `pattern_step_t`. The
  `filter_cutoff` and `filter_resonance` fields.
- **The tweak filter UI** — `main/ui/knobs.c`. Maps ADC readings
  to step-level filter values.
- **The mixer** — `components/amy/src/amy.c`. AMY's voice
  allocator mixes up to N concurrent events into the output
  buffer.

### 🚫 What we can't simulate

- **The undertone synth.** Frequencies below the audible range.
  Our firmware doesn't generate them.
- **The XOR cymbal.** The XOR-based inharmonic cymbal is a
  beautiful analog circuit. We can't synthesize that timbre
  without a custom wavetable.
- **Optocoupler-controlled sequencing.** An optocoupler is an
  electrically-isolated analog component. We have no analog of
  electrical isolation.

---

<a id="chapter-15"></a>
## Chapter 15 — Thoughts on Automation

### 📖 What Pearson does

Chapter 15 is the closing essay. Pearson recounts a story from the
Bay Area Maker Faire: a man told him that synthesizers were going
to take jobs away from human drummers. Pearson uses this as a
springboard to argue that synthesizers have *always* been folk
instruments, *never* job-replacement machines. He traces the
"synthesizers will replace us" fear back to the Ondioline of the
1940s — when unions really did panic — and notes that the fear
never came true.

The chapter is not technical. It is a manifesto. Pearson closes
with:

> "Deep down, we really truly believe that instrument building is
> an important, empowering activity, and that the more
> experimentation there is in the world, the more aware and
> compassionate we all become."

### 🎓 Background: the politics of our project

Building a piece of firmware that emulates an existing commercial
product is a slightly different act from building a brand-new
instrument — but it shares the same spirit. To make our firmware,
you have to understand *why* the PO-33 is special: the
immediacy of recording, the constraint of 16 slots, the way 16
steps force you into a particular kind of rhythm, the way 16
punch-in effects turn every performance into a small surprise.

You are not just transcribing code. You are reverse-engineering a
*design philosophy* and rebuilding it on cheaper, more open
hardware. The Teenage Engineering product team made specific
choices — 16 steps, 16 slots, 16 effects — that constrain the
creative output in ways that turn out to be generative. Our
firmware inherits those choices because they work.

### 🔧 Try it on the device

There is no exercise for this chapter. Read it; let it set the
tone for what you've just learned.

### 🛠 Code reference

There is no code reference. The closest reference is the
**LICENSE** file at the root of this repository: MIT. The whole
project is open source, free to fork, free to remix, free to use in
your own instruments.

### 🚫 What we can't simulate

Politics. But we can build on it.

---

<a id="appendices"></a>
## Appendices

Pearson's book has seven appendices. They are reference material,
not project material. Each has a short description here and a
pointer to where the same content lives in our project.

### Appendix I — The Components of a Classical Synthesizer

A glossary of synth components: oscillators, filters, envelopes,
LFOs, mixers, etc. The same concepts are scattered throughout our
firmware. Cross-references:

| Pearson's term | Our equivalent | Where |
|---|---|---|
| VCO (voltage-controlled oscillator) | An `amy_event` with `wave=SAW/SINE/SQUARE` and a `frequency` | `components/amy/src/amy.h` `amy_event` |
| VCF (voltage-controlled filter) | An `amy_event` with `filter_type`, `filter_freq`, `filter_resonance` | same |
| VCA (voltage-controlled amplifier) | The amplitude field of an `amy_event` | same |
| Envelope generator | AMY's `bp0_times[]`, `bp0_values[]` | same |
| LFO | AMY's `mod_source` and `mod_target` | same |
| Mixer | AMY's voice allocator | `components/amy/src/amy.c` `amy_render_buffer` |
| Sample-and-hold | (not implemented) | — |

### Appendix II — Integrated Circuit Information

A datasheet summary for every IC the book uses. The book uses:

- **555** — timer chip, used as oscillator.
- **40106** — hex Schmitt trigger inverter, used as oscillator and
  tone shaper.
- **LM386** — audio power amplifier.
- **LM741** — op-amp, used in active filters.
- **4093** — quad 2-input NAND Schmitt trigger.
- **4040** — 12-stage binary counter.
- **4017** — decade counter / divider.
- **4046** — phase-locked loop.
- **4070** — quad XOR gate.
- **4051** — 8:1 analog multiplexer.

None of these are used in our firmware directly. Our "IC" is the
ESP32-S3 microcontroller, and our "components" are the AMY library
primitives. The closest analog chips are listed in
`hardware/HARDWARE.md`:

- **PCM5102A** — audio DAC.
- **INMP441** — I²S MEMS microphone.
- **ILI9341** — TFT display controller.
- **MCP23017** (or similar) — I/O expander for the button matrix.

### Appendix III — Part Sourcing

Where to buy parts. Our equivalent is the BOM in
`hardware/HARDWARE.md`. Rough cost for our device: about $15 in
parts (ESP32-S3, DAC, mic, TFT, buttons, headers). Pearson's
parts list for the Dogbotophone is roughly the same cost — many of
his ICs are a few cents each, but the cumulative cost is similar.

### Appendix IV — Electronics Formulas to Know

Ohm's law (V = IR), the RC time constant, the 555 oscillator's
period formula, the LM386 gain formula, the decibel formula. We don't
need any of these in our firmware — software has different
primitives. The closest "formulas we use" are:

- **MIDI note to frequency:** `freq = 440 × 2^((note - 69) / 12)`
- **Sample rate to time:** `duration_seconds = sample_count /
  AMY_SAMPLE_RATE`
- **Decibels to amplitude:** `amp = 10^(dB / 20)` (used in AMY's
  reverb and filter math, not directly in our firmware)

These are encoded in the AMY library.

### Appendix V — Electronic Music You Should Know

A listening list. The same applies to our project — listen to
electronic music made with PO-33s and similar sample-based gear.
Search for "PO-33 lofi" or "PO-33 hip-hop" on your streaming
service of choice. The genre is small but devoted.

### Appendix VI — Seventy Great Synth Albums

A canon. Worth listening to all of them. They have nothing to do
with our firmware, but they will inspire your sound design.

### Appendix VII — Glossary

This is Pearson's standalone glossary. We have our own Glossary
below, which doubles as a cross-reference for our project. If a
term appears in both glossaries with different meanings, ours
defines it for the firmware-friendly meaning.

---

<a id="glossary"></a>
## Glossary

This glossary defines every concept Pearson introduces, plus a few
that are specific to our firmware. Where a concept has a direct
digital analog in our project, the analog is listed.

### A

- **Active filter** — a filter circuit that uses an amplifier
  (typically an op-amp) to boost the signal. *Our analog*: a
  digital filter with `filter_type = FILTER_LPF/HPF/BPF`.
- **Amplifier (amp)** — a circuit that increases the power of a
  signal. *Our analog*: the `velocity` field of an `amy_event`,
  multiplied by `volume_multiplier()`.
- **AMY** — All-purpose Music synthesizer librarY. The open-source
  fixed-point DSP library our firmware uses for all sound
  generation. <https://github.com/shorepine/amy>
- **Asthma** — a respiratory condition. Not a circuit.
- **Astable** — a 555 timer mode where the chip oscillates. *Our
  analog*: any AMY oscillator with a non-zero frequency.

### B

- **Binary counter** — a chip that increments a binary count on
  each clock pulse. *Our analog*: the `step` counter in
  `sequencer.c`, which increments from 0 to 15 and wraps.
- **Breadboard** — a prototyping board with internal metal strips
  that connect components. *Our analog*: the dev board + the UART
  shell.
- **BPM** — beats per minute. The tempo of the sequencer.
  60 BPM = 1 beat per second; 240 BPM = 4 beats per second.

### C

- **Capacitor** — a passive component that stores charge. *Our
  analog*: not directly modeled, but the concept is the same as
  AMY's delay line, which "stores" audio samples in memory.
- **Chip** — colloquial for integrated circuit.
- **Cutoff frequency** — the frequency at which a filter starts
  attenuating. *Our analog*: AMY's `filter_freq` field.

### D

- **DAC** — digital-to-analog converter. Turns numbers into
  voltage. Our hardware uses the PCM5102A.
- **Darlington pair** — two transistors wired together for high
  current gain. Not used in our project.
- **Drum slot** — a sample slot (1–8) that is monophonic and
  plays at fixed pitch.

### E

- **Envelope** — a control voltage that rises and falls over time
  to shape a sound. *Our analog*: AMY's `bp0_times[]` and
  `bp0_values[]` — a list of (time, value) pairs.
- **ESP32-S3** — the microcontroller chip at the heart of our
  hardware. Espressif's flagship.
- **EXPERIMENT** (in Pearson's book) — a small, observation-only
  exercise. Often builds intuition without soldering.

### F

- **Filter** — a circuit (or software function) that passes some
  frequencies and attenuates others.
- **FX** — short for "effect". In our project, the punch-in
  effects that mutate the next triggered note.
- **Frequency** — how many times per second a wave repeats. Our
  sample rate is 44 100 Hz; audible range is 20 Hz to 20 000 Hz.

### G

- **Glide** — smoothly sliding between two pitches. *Our analog*:
  AMY's `slew_time`. (Not wired to a UART verb yet.)
- **Glissando** — sliding through every pitch in between (as
  opposed to going directly from one to another). Same as glide.

### H

- **Hertz (Hz)** — cycles per second.
- **Hex inverter** — six inverters in one chip. The 40106. Used
  by Pearson as both oscillator and tone shaper.

### I

- **I²C** — a two-wire serial bus. Our button-matrix I/O expander
  uses I²C.
- **I²S** — a serial protocol for audio. Our DAC and mic both use
  I²S.
- **IC** — integrated circuit.
- **IDE** — integrated development environment.
- **Integrated circuit** — see IC.
- **ISR** — interrupt service routine. A function that runs in
  response to a hardware event. Used for button presses and
  sync-in pulses.

### J

- **Jumper wire** — a short piece of wire with pins on each end,
  used to connect two points on a breadboard. *Our analog*: a
  trace on a PCB. (No wires for the user to plug in.)

### K

- **Knob A / Knob B** — the two analog potentiometers on our
  device. A is GPIO 2, B is GPIO 46.

### L

- **LED** — light-emitting diode. We don't have one on our
  device; the TFT serves as the visual indicator.
- **LFO** — low-frequency oscillator. An oscillator below the
  audible range used to modulate other parameters. *Our analog*:
  AMY's `mod_source` field.
- **Line-in** — an audio input that takes a line-level signal.
  Our device has the INMP441 mic but no dedicated line-in jack.

### M

- **MIDI** — musical instrument digital interface. A standard
  protocol for music gear. Our firmware uses MIDI note numbers
  internally (0..127) but does not speak the MIDI protocol
  externally.
- **Microphone (mic)** — a transducer that converts air pressure
  into voltage. Our firmware uses an I²S MEMS mic (INMP441).
- **MIDI note number** — a number from 0 to 127 that names a
  pitch. 60 = C4, 69 = A4, 72 = C5.

### N

- **Noise** — a signal with no discernible pitch. *Our analog*:
  AMY's `wave = NOISE` patch.
- **NVS** — non-volatile storage. A key-value store in the
  ESP32's flash. We use it for clock + alarm settings.

### O

- **Op-amp** — operational amplifier. Pearson uses the LM741 for
  active filters. We don't have an op-amp — AMY's filter is
  digital.
- **Octave** — a doubling (or halving) of frequency. C4 to C5 is
  one octave.
- **Oscillator (OSC, VCO)** — a circuit that produces a repeating
  wave. *Our analog*: AMY's `wave = SAW/SINE/SQUARE` patch.
- **Optocoupler** — an electrically-isolated switch. Not used in
  our project.

### P

- **PCM** — pulse-code modulation. The raw format of digital
  audio: a sequence of integer samples.
- **Phase-locked loop (PLL)** — a circuit that locks one
  oscillator's phase to another. Not implemented in our firmware
  beyond a 1-pulse-per-step sync.
- **Phase** — the position in a periodic cycle. Important for
  filter math; less important for our sample-based playback.
- **Piezo** — a crystal that converts voltage to vibration and
  vice versa. We don't use one.
- **Pitch** — how high or low a sound is. Doubling the frequency
  = +1 octave.
- **Plate reverb** — a reverb made by vibrating a metal plate.
  *Our analog*: AMY's reverb algorithm.
- **PNP / NPN** — types of bipolar transistors. Not used in our
  project.
- **Portamento** — see Glide.
- **Potentiometer (pot)** — a variable resistor. *Our analog*:
  the two Knobs on our device (GPIO 2 and GPIO 46).
- **PWM** — pulse-width modulation. Varying the duty cycle of a
  square wave. *Our analog*: AMY's `pulse_width` field.
- **PSRAM** — pseudo-static RAM. Cheap, large, external memory.
  Our board has 8 MB of PSRAM; we use it to store recorded
  samples.

### Q

- **Q factor** — see Resonance.

### S

- **Sample** — a single value in a digital audio stream. We
  record and play back 16-bit signed integer samples at 44 100 Hz.
- **Sample rate** — how many samples per second. 44 100 = CD
  quality.
- **Saw wave** — a waveform shaped like a sawtooth. Bright,
  buzzy. *Our analog*: AMY's `wave = SAW`.
- **Schematic** — a circuit diagram. *Our analog*: the block
  diagrams in `docs/DESIGN.md`.
- **Sequencer** — a device that plays a sequence of notes in a
  loop. Our firmware has a 16-step sequencer.
- **Sine wave** — the mathematically purest periodic wave.
  *Our analog*: AMY's `wave = SINE`.
- **Slot** — a numbered storage location for a sample. Our
  firmware has 16 slots.
- **Speaker** — a transducer that converts voltage into air
  pressure. Our device outputs via the PCM5102A DAC + an external
  amplifier chip + a 3.5mm jack.
- **Square wave** — a wave that is +V for half its period and 0V
  for the other half. *Our analog*: AMY's `wave = SQUARE`.
- **Step** — one moment in a beat. The PO-33 has 16 steps per
  pattern.
- **Subharmonic** — a frequency below the fundamental. Pearson
  builds a subharmonic generator. Not implemented in our
  firmware.

### T

- **Talkbox** — a plastic tube that plays audio into your mouth,
  with a mic picking up the result. The "robot voice" effect.
- **TFT** — thin-film transistor, the type of display we use.
  ILI9341 controller, 240×320 pixels.
- **Timer chip** — an IC that produces pulses at a fixed rate.
  Pearson uses the 555.
- **Tweak mode** — one of three "knobs" we affect with the two
  physical knobs: Tone, Filter, Trim.
- **Tweak** — a small adjustment to a sound.

### U

- **UART** — universal asynchronous receiver/transmitter. The
  serial protocol we use to talk to the firmware over USB.

### V

- **Vactrol** — a light-controlled variable resistor. Pearson
  uses one to make his arpeggiator tunable.
- **VCO** — voltage-controlled oscillator. Pearson uses the
  4046 chip.
- **VCA** — voltage-controlled amplifier. *Our analog*: the
  `velocity` field of an `amy_event`.
- **VCF** — voltage-controlled filter. *Our analog*: AMY's
  filter struct.
- **Velocity** — how hard a sound is played. 0..127. Mapped to
  amplitude in our firmware.

### W

- **Waveform** — the shape of a periodic wave. Common: sine,
  square, saw, triangle, pulse.
- **Write mode** — a PO-33 mode where tapping a step toggles
  whether the step is bound to a slot. Press `WRITE` to enter.

---

<a id="index"></a>
## Index

This is an alphabetical list of every concept covered in this book,
with chapter references. Concepts in **bold** are the most important
ones.

- **Active filter** — Ch 8, Appendix I
- Alarm (wall-clock) — Setup
- AMY library — Setup, Ch 3, Glossary
- Amplifier — Ch 4, Glossary
- App note (datasheet) — Appendix II
- Battery — Setup, Ch 11
- Binary counter — Ch 11, Appendix II
- **BPM** — Ch 11, Glossary
- Breadboard — Ch 3, Glossary
- Capacitor — Ch 3, Glossary
- Chain — Ch 11, Appendix I
- Clock — Ch 11, Ch 13
- Companion book — How to use this book
- Component — Appendix I, Appendix II
- Concept of voltage — Ch 2
- Cutoff frequency — Ch 8, Glossary
- DAC (PCM5102A) — Setup, Ch 2, Ch 4, Glossary
- Delay line — Ch 4 (reverb), Ch 12 (noise)
- **Drum slot** — Ch 12, Glossary
- Drum synthesis — Ch 12
- Envelope — Ch 9, Ch 12, Glossary
- ESP32-S3 — Setup, Glossary
- **EXPERIMENT** (Pearson) — How to use this book, Ch 2
- Factory reset — Setup
- Filter — Ch 8, Glossary
- **FM synthesis** — Ch 9
- Formulas — Appendix IV
- FX (punch-in) — Ch 6, Ch 10, Ch 12, Glossary
- Gain — Ch 4
- Glide / Glissando — Ch 13
- Glossary — Glossary
- Hardware — Setup, Ch 5
- Headphone — Ch 4
- Hertz — Glossary
- Hex inverter (40106) — Ch 6, Ch 12, Appendix II
- History — Ch 1
- IC (integrated circuit) — Ch 3, Appendix II, Glossary
- I²S — Ch 2, Glossary
- INMP441 (microphone) — Ch 2, Glossary
- Knob A / B — Setup, Ch 3, Ch 8
- LED — Ch 3, Glossary
- **LFO** — Ch 10, Appendix I, Glossary
- Light-dependent resistor (LDR) — Ch 3
- Line-in — Glossary (see "I²S microphone" for what we have)
- LM386 (amp chip) — Ch 4, Appendix II
- LM741 (op-amp) — Ch 8, Appendix II
- Manufacturing the synth — Ch 1, Ch 5
- **Melodic slot** — Ch 3, Ch 9, Glossary
- **MIDI note number** — Ch 3, Ch 9, Glossary
- Mixer — Ch 4, Ch 14, Appendix I
- Multiplexer (4051) — Ch 11, Appendix II
- Noise — Ch 12, Glossary
- Op-amp — Ch 8, Appendix II
- **Octave** — Ch 9, Glossary
- **Oscillator** — Ch 3, Appendix I, Glossary
- Pattern — Ch 11, Ch 14, Glossary
- **PCM (audio format)** — Ch 2, Glossary
- Phase-locked loop (PLL) — Ch 13, Glossary
- Pitch — Glossary
- Plate reverb — Ch 4, Glossary
- **PLL** — Ch 13
- Portamento — Ch 13, Glossary
- **Potentiometer** — Ch 3, Glossary
- PROJECT (Pearson) — How to use this book, Ch 3, Ch 4, Ch 6, Ch 8, Ch 9, Ch 10, Ch 11
- PWM — Ch 10, Glossary
- Q factor — Ch 8
- Recording — Ch 2, Ch 12
- Resistor — Ch 3, Glossary
- **Resonance** — Ch 8, Glossary
- Reverb — Ch 4
- Ring modulation — Ch 10
- Sample (audio) — Ch 2, Glossary
- Sample-and-hold — Ch 13
- Sample rate — Ch 2, Ch 4, Glossary
- Saw wave — Ch 3, Glossary
- Schematic — Ch 7
- Sequencer — Ch 11, Ch 14
- Sine wave — Ch 3, Glossary
- **Slot** — Ch 12, Ch 2, Glossary
- Soldering — Ch 5
- Speaker — Ch 2, Ch 4
- **Step** — Ch 11, Glossary
- **Stereo panning** — Ch 4
- Square wave — Ch 3, Ch 14, Glossary
- Subharmonic — Ch 9, Ch 14, Glossary
- Sync — Ch 11, Ch 13
- Synthesizer as a folk instrument — Ch 1
- Talkbox — Ch 4
- TFT — Ch 5, Glossary
- Tweak mode (TONE / FILTER / TRIM) — Ch 3, Ch 8, Ch 9
- Tremolo — Ch 6, Ch 10
- Twin-T voice — Ch 14
- UART — Setup, Glossary
- Vactrol — Ch 6, Ch 10, Ch 14, Glossary
- **Velocity** — Ch 3, Ch 4, Glossary
- VCO (voltage-controlled oscillator) — Ch 13, Appendix I, Glossary
- Volume — Ch 4
- Wiring — Ch 12
- Wavetable — Ch 9
- Write mode — Ch 11
- XOR — Ch 14

---

*This book was generated by reverse-engineering Kirk Pearson's
*Make: Electronic Music from Scratch* (Dogbotic, 2024). The complete
source text of Pearson's book is included at
`docs/Make_Electronic_Music.md` in this repository, used with the
intent of producing a fair-use companion guide. Pearson is the
author; we are not. We have summarised rather than reproduced. If you
find this companion helpful, please consider purchasing the book from
[Maker Media](https://make.co/) or your local bookstore.*

*This companion book is licensed MIT, same as the rest of this
project.*

