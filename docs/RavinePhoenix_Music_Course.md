# The PO-33 Musical Companion

## Beat, Melody, Groove, and Genre for the Novice Operator

*A self-study music course for the RavinePhoenix K.O! firmware and the real PO-33 K.O! Pocket Operator.*

---

## Edition Notice

This is **Edition 1** of *The PO-33 Musical Companion*. It is written to accompany the open-source **RavinePhoenix** firmware (an ESP32-S3 port of the Teenage Engineering PO-33 K.O! sampler), but every exercise in this book also works on a real PO-33 K.O! Pocket Operator. Where the two devices differ, the book flags the difference with a sidebar.

---

## How to Read This Book

This is **not** the PO-33 manual. The manual tells you what each button does. This book tells you **why** to press it, **what** to press it for, and **how** to combine presses into music that sounds like music, not noise.

You should read the manual first (or at least skim §1–§4 of it), then read this book from the beginning. The book is divided into **5 Parts** (17 chapters) plus **5 Appendices**. Each chapter follows the same 8-section pattern:

1. **Chapter Title and Objectives** — what you will learn.
2. **Musical Concept** — the theory, explained simply.
3. **PO-33 Translation** — the exact button sequence.
4. **Guided Exercise** — a step-by-step activity with clear success criteria.
5. **Creative Challenge** — an open-ended prompt for personal expression.
6. **Listening Assignment** — a song or genre to analyse.
7. **Reflection Questions** — to consolidate what you just learned.
8. **Summary and Checkpoint** — key takeaways and a self-check.

A chapter takes about **45 to 60 minutes** to read and **another 30 to 60 minutes** to complete the exercises. The full book is therefore about a **6-week study plan** if you do one chapter every three days, or a **single immersive weekend** if you binge.

You do not need any prior musical training. Every term is defined the first time it appears and again in the glossary. If you have never touched a sampler before, **start at Chapter 1**.

---

## Table of Contents

### Front matter

- Title page
- How to read this book
- Acknowledgements
- Source material

### Introduction — Why a PO-33 Book That Isn't the Manual

### Part I — Orientation

- Chapter 1 — The PO-33 and Why Limits Breed Music
- Chapter 2 — Anatomy of the Box: Sampling, Slots, and the 16-Step Grid
- Chapter 3 — Tempo, Meter, and the Pulse: Setting Your First BPM

### Part II — Rhythm and Beat

- Chapter 4 — Kick, Snare, Hi-Hat: The Drum Trinity
- Chapter 5 — Slicing a Break: Building Beats from Samples
- Chapter 6 — Syncopation, Swing, and the Human Groove

### Part III — Melody and Harmony

- Chapter 7 — Pitches, Notes, and the Chromatic Pad
- Chapter 8 — Scales and Intervals: Why Some Notes Sound Good Together
- Chapter 9 — Basslines: The Foundation Under Everything
- Chapter 10 — Chords and Chord Progressions: Harmony for One Octave

### Part IV — Groove and Genre

- Chapter 11 — Hip-Hop at 80 BPM
- Chapter 12 — House and Disco at 120 BPM
- Chapter 13 — Techno and "Tencho" at 140 BPM
- Chapter 14 — Lo-Fi, Experimental, and the Sound of Constraint

### Part V — Full Tracks and Performance

- Chapter 15 — Song Structure: Intro, Verse, Chorus, Bridge
- Chapter 16 — Pattern Chaining, Parameter Locks, and Variation
- Chapter 17 — Live Performance and Recording Your Music

### Appendices

- Appendix A — Glossary of Musical Terms
- Appendix B — PO-33 Button Cheat Sheet
- Appendix C — Four-Week Practice Plan
- Appendix D — Troubleshooting Common Issues
- Appendix E — Sampling Ethics, Sources, and Further Reading

---

## Acknowledgements

This book draws on two excellent open-source theory texts:

- **J. Anthony Allen, *Music Theory for Electronic Music Producers: The Producer's Guide to Harmony, Chord Progressions, and Song Structure in the MIDI Grid*** (Slam Academy, 2018). Allen's "no pianos, no singing" method — translating every theoretical concept into a step on a piano-roll grid — is the direct ancestor of the way this book translates every concept into a step on the PO-33's 16-step grid.
- **Michael Hewitt, *Music Theory for Computer Musicians*** (Cengage, 2008). Hewitt's musician-first approach — defining terms, building intuition, then introducing notation — informed the book's beginner-friendly tone.

Both books are paraphrased and adapted throughout. Where I have quoted a concept directly, I have tried to keep the quote short and the citation visible. The reader who wants the longer theoretical treatment is directed to those two books in Appendix E.

The PO-33 hardware facts are drawn from the Teenage Engineering PO-33 K.O! manual and from the **RavinePhoenix** firmware design document (`docs/DESIGN.md` in the RavinePhoenix repository), both of which are freely available.

---

## Source Material

This book synthesizes concepts from three sources:

1. **The PO-33 K.O! manual** — Teenage Engineering's official guide to the hardware. The RavinePhoenix firmware design document is the v1 reference for what our device does.
2. ***Music Theory for Electronic Music Producers*** by J. Anthony Allen — the source for the chord-progression-as-MIDI-grid philosophy that runs through Part III and Part V.
3. ***Music Theory for Computer Musicians*** by Michael Hewitt — the source for the rhythm fundamentals that run through Part II.

Where the two theory books disagree, this book follows the more producer-friendly position. Where the firmware disagrees with the real PO-33 hardware, this book flags it in a sidebar and follows the firmware (since most readers will be using the RavinePhoenix board).

---

# Introduction — Why a PO-33 Book That Isn't the Manual

You have probably already looked at the PO-33 K.O! manual. It is a fold-out sheet of paper with tiny diagrams. It tells you which buttons are record, play, write, FX, sound, BPM, and pattern. It tells you that the 16 numbered pads are simultaneously step buttons, sound selectors, FX selectors, and pattern selectors, depending on which modifier you hold. It tells you to hold record + pad 5 to record into slot 5, and to press the BPM button to cycle between Hip Hop (80 BPM), Disco (120 BPM), and Techno (140 BPM).

If you read the manual and then tried to make a beat, you probably failed. Not because the manual is bad — it is excellent — but because the manual is a **reference**, not a **course**. A reference tells you every fact. A course tells you which facts matter, in which order, with which examples, and with which warnings. A course builds skill. A reference builds vocabulary.

This book is a course. It will not list every button combination. It will list the **five** button combinations you need to make a beat, the **eight** combinations you need to make a melody, and the **three** combinations you need to make a song. Everything else, the manual can tell you when you need it.

## Who this book is for

This book is for you if any of the following are true:

- You have a PO-33 (real or RavinePhoenix firmware) and you have not yet made a song you actually like.
- You have made sounds on a PO-33 but they sounded stiff, lifeless, or random.
- You tried to follow YouTube tutorials but they assumed you already knew what a "step" was, what a "chord progression" was, or what "swing" did.
- You have never made music before and the PO-33 looks like the easiest possible entry point (it is, and this book will prove it).
- You are a returning musician who used to play another instrument, and you want to make beats on a PO-33 but the language of "sampling" and "step sequencing" feels alien.
- You are an electronic music producer on a computer (Ableton, Logic, FL Studio) and you want to understand the PO-33 because you bought one as a sketchpad.

This book is **not** for you if you already make PO-33 music confidently and you are looking for advanced synthesis techniques. You already know more than this book can teach.

## What this book is not

- It is **not** a manual. The manual is the manual. This book uses it.
- It is **not** a music theory textbook. It teaches you only the theory you need to make music on a PO-33. If you want a deep dive into counterpoint, jazz harmony, or 12-tone serialism, this book will not help you (and neither will a PO-33).
- It is **not** a music production manual for computer software. The PO-33 is a sketchpad, not a multitrack recorder. If you need a 64-track DAW, buy Ableton. If you want a device you can hold in your hand and finish a song on the bus, the PO-33 is your friend.

## The promise of this book

By the end of the book, if you do the exercises, you will be able to:

- Make a complete beat (drums + bass + lead) from scratch in under 30 minutes.
- Switch between hip-hop, house, and techno grooves and explain why they feel different.
- Use the PO-33's 16-step sequencer to capture a song idea in one sitting.
- Jam live on the PO-33 without a plan, and have it sound like music.
- Read the manual's terse instructions and understand them on the first read.

You will not be a professional producer. You will, however, be a **musician** — which is a different and better thing.

## How the book is organised

The book follows the natural order of making music:

- **Part I** gets you oriented — what the device is, what the buttons do, what a step is.
- **Part II** gets you making beats — rhythm first, melody later.
- **Part III** introduces melody and harmony — notes, scales, chords, basslines.
- **Part IV** puts rhythm and melody together in **genre contexts** — hip-hop, house, techno, lo-fi. Each genre has a different BPM, a different swing setting, and a different harmonic palette. By the end of Part IV you will have four complete beats in four genres.
- **Part V** teaches you to make **songs** — how to extend a beat into a 3-minute track with intro, verse, chorus, and outro, and how to perform it live.

Each chapter ends with a **checkpoint**: a short list of self-check questions. If you can answer "yes" to all of them, you are ready for the next chapter. If you can't, re-read the chapter and redo the exercise. There is no shame in redoing an exercise — every working musician has done every exercise they teach more than once.

## A note on "musicality"

The PO-33 has 16 buttons, 2 knobs, and a microphone. It cannot play a piano, a guitar, or a string section. It cannot sustain a note for longer than the sample you recorded. It cannot play more than four sounds at once.

These are **not** limitations. They are **boundaries**, and boundaries are where creativity lives. A haiku is not a "limited poem" — it is a poem whose boundaries force the poet to find the perfect word. A 16-step sequencer is not a "limited DAW" — it is a beat whose boundaries force you to find the perfect rhythm. J Dilla, the most influential hip-hop producer of the 2000s, used a 12-step sequencer (the MPC's default) and made beats that changed the sound of pop music. The PO-33's 16 steps are more than enough.

If you finish this book and you have made four beats you actually like, you have succeeded. If you have also learned to perform one of them live, you have excelled. The book does not need to give you more than that to be worth your time.

Let's begin.

---

*Continuing Part 1…*
# Part I — Orientation

Before you make a sound, you need to know the instrument. Part I introduces the PO-33's three core ideas — **the slot**, **the step**, and **the tempo** — and gives you the smallest possible beat you can make: a single drum hit on a single step. By the end of Part I you will have made a sound, stored it, sequenced it, and pressed play. You will also know why the PO-33 has only 16 steps and only 16 slots, and why those numbers are exactly right.

---

## Chapter 1 — The PO-33 and Why Limits Breed Music

### Objectives

By the end of this chapter you will be able to:

- Explain in one sentence what a **sampler** does.
- Identify the PO-33's three BPM presets by name and number.
- Name three reasons why 40 seconds of sampling memory is enough.
- Make a single sound and play it back.

### Musical concept: what is a sampler?

A **sampler** is a musical instrument that records a sound and then lets you play the recording back on command. A microphone hears the sound. A computer stores the recording. A speaker plays the recording back, or **samples** it. The word *sample* in modern music means both **a recorded sound** ("I sampled a cat meowing") and **the act of recording it** ("I sampled my guitar").

The PO-33 is a **pocket sampler**. It fits in your hand. It runs on two AAA batteries. It can record up to **40 seconds** of audio total, across all 16 slots. You record a sound, you store it in a slot, and then you play it back from the slot. The PO-33's distinctive feature — the thing that makes it different from earlier pocket samplers — is the **16-step sequencer**: 16 numbered buttons that you can use to play back your samples in a pattern, one step at a time, in a loop.

This book assumes you have either a real PO-33 or a RavinePhoenix firmware running on an ESP32-S3 board. The buttons, the sequencer, and the audio quality are similar; the differences are flagged in sidebars.

### The three BPM presets

The PO-33 has three preset tempos: **Hip Hop (80 BPM)**, **Disco (120 BPM)**, and **Techno (140 BPM)**. BPM stands for **beats per minute**. 80 BPM means 80 beats fit into 60 seconds — each beat lasts 0.75 seconds. 120 BPM means each beat lasts 0.5 seconds. 140 BPM means each beat lasts about 0.43 seconds.

The presets are not arbitrary. 80 BPM is the bottom of the comfortable walking tempo — slow enough to sway to, fast enough to feel motion. 120 BPM is exactly two beats per second — easy to count, the tempo of disco and most house. 140 BPM is fast — the tempo of techno, drum'n'bass (mostly), and "Tencho" (PO-33 slang for **techno slowed down enough that you can play it on the grid** — actually the PO-33 just calls it "Techno").

> **Sidebar — PO-33 manual vs. RavinePhoenix firmware.** The real PO-33 calls them "HIP HOP", "DISCO", and "TENCHO" (with the typo, which is intentional, a Teenage Engineering joke). The RavinePhoenix firmware displays them the same way.

You can also fine-tune the tempo anywhere from 60 to 240 BPM by holding the BPM button and turning **Knob B**. You will use this feature in Part IV when we explore genres outside the three presets.

### Why 40 seconds is enough

The PO-33 gives you **40 seconds of total recording time, divided among 16 slots**. Most slots will hold a few seconds of audio — a drum hit (less than half a second), a vocal phrase (a few seconds), a synth chord (one or two seconds). The longest single sample you can record is about 10 seconds if you use only one slot.

Why is this enough? Because the PO-33 is a **sequencer**, not a recorder. A sequencer plays a short sound many times. A 0.25-second kick drum sample, played sixteen times in a loop at 120 BPM, fills **8 seconds of music**. The PO-33 is designed to make music out of short sounds repeated often, not out of long sounds played once.

This is the most important idea in the book. **The PO-33 is a loop machine. Its limitations are loops.** When you finish this book, you will be making beats by recording 16 short sounds, looping them, and layering the loops.

### The three limits and why they help you

The PO-33 has three hard limits:

1. **40 seconds of sample memory total.**
2. **16 sample slots total.**
3. **16 steps per pattern.**

These limits are not arbitrary. They were chosen by Teenage Engineering to **force you into creative decisions**. A 64-step DAW encourages you to fill every step. A 16-step sequencer forces you to leave most steps empty — and the empty steps are where the groove lives. A 40-second memory budget forces you to commit to short sounds — and short sounds loop better than long ones. A 16-slot limit forces you to pick your 16 favourite sounds — and that picking is where your musical taste develops.

This is the second-most important idea in the book. **Limits are creative tools.** When you feel stuck, ask yourself "what is the smallest change I could make?" — that is almost always the right question on a PO-33.

### PO-33 translation: recording your first sound

The PO-33 has 16 numbered pads. Each pad is both a **sound selector** and a **step button**. The same physical pad means different things depending on which modifier you hold.

To record a sound into slot 1:

1. Hold the **REC** button (a star symbol on the real PO-33).
2. Press **pad 1**. The screen now shows a recording timer.
3. Make the sound you want to record — talk, clap, snap, sing, hit a desk, whatever.
4. Release **REC**.
5. The sound is now stored in slot 1.

To play back the sound:

1. Press **pad 1** without holding REC. The sound plays once.
2. To play it repeatedly, hold **REC** again — actually no, that's record. To play it on a step, you write it into the sequencer. We'll do that in Chapter 2.

> **Sidebar — RavinePhoenix firmware.** The RavinePhoenix firmware's button layout is identical to the real PO-33. REC is the star button on GPIO 9. Pad 1 is the top-left of the 4×4 grid. There is no functional difference for the operator.

### Guided exercise: record and play back your first sound

You will need: a PO-33, a quiet room, and one object that makes a sound.

**Success criteria:** you have a recording in slot 1 that plays back when you press pad 1.

1. Find a quiet room. The PO-33's built-in microphone will pick up everything, including your breathing. Hold the device about 15 cm from your mouth.
2. Hold **REC**.
3. Press **pad 1**.
4. Say your name out loud. Keep it under 2 seconds.
6. Release **REC**.
7. Press **pad 1** (without holding REC). Your name should play back.
8. If it doesn't, check the trim — you may have recorded only silence. The PO-33 displays the recording's waveform; if it is flat, you didn't record anything.

If your recording has silence at the start (because you pressed REC and then took a moment to start talking), trim it. The PO-33's trim feature is on the **Tweak Trim** mode: hold the FX button, tap it three times to cycle through Tone, Filter, Trim. Then turn Knob A to move the start point, Knob B to move the end point. We will return to trim in detail in Chapter 5.

### Creative challenge: record three contrasting sounds

Record three more sounds, each into its own slot:

- A **percussive** sound (clap, snap, hit) into slot 1.
- A **tonal** sound (sing a vowel, hum a note) into slot 2.
- A **long** sound (whistle, sigh, slide) into slot 3.

Play each one back. Notice how the percussive sound has a clear attack and decay. The tonal sound has a clear pitch. The long sound has neither. These three sound types are the three building blocks of every piece of music you will ever make on the PO-33.

### Listening assignment

Listen to **J Dilla, "Workinonit"** from *Donuts* (2006). The entire beat is built from short drum samples looped into a 16-step pattern. Notice how many of the steps are empty. Notice how the beat breathes.

Listen to **Daft Punk, "Da Funk"** (1995). The bassline is a 1-second sample of an analogue synth, looped 16 times per bar. The drum loop is 4 short samples, layered.

Listen to **Madlib, "Accordion"** from *Madvillainy* (2004). The entire beat is one sample (a looped accordion figure) plus a kick and a snare. Two drum sounds plus one loop, four total samples. This is the PO-33's native format.

### Summary and checkpoint

**Key takeaways:**

- A sampler records sound and plays it back.
- The PO-33 has 40 seconds of memory, 16 slots, and 16 steps. These limits are creative tools.
- The three BPM presets are 80 (Hip Hop), 120 (Disco), and 140 (Tencho). Fine-tune with Knob B.
- Record by holding REC + a pad. Play by pressing the pad.

**Self-check:**

- [ ] Can you explain in one sentence what a sampler does?
- [ ] Can you name the three BPM presets and their tempos?
- [ ] Can you record a sound into slot 1 and play it back?
- [ ] Do you understand why "fewer samples, used cleverly" is the PO-33 ethos?

If you answered "yes" to all four, move on to Chapter 2.

---

## Chapter 2 — Anatomy of the Box: Sampling, Slots, and the 16-Step Grid

### Objectives

By the end of this chapter you will be able to:

- Distinguish **melodic slots** (1–8) from **drum slots** (9–16).
- Place a sound on a specific step of the 16-step sequencer.
- Press play and hear your sound loop.
- Edit a step (remove it, move it, change its slot).

### Musical concept: what is a step?

A **step sequencer** is a row of buttons, one per beat, that plays a sound when you press play and reaches that step. The PO-33 has 16 steps in a 4×4 grid. Step 1 is the top-left button. Step 16 is the bottom-right button.

Each step is a sixteenth-note in 4/4 time. That means there are **four steps per beat**. In one bar (4 beats) there are 16 steps. In a 4-minute song at 120 BPM there are 1,920 steps. The PO-33 loops one bar (16 steps) by default. You can chain patterns to make longer songs — we'll get to that in Chapter 15.

> **Sidebar — Hewitt's note values.** Michael Hewitt (*Music Theory for Computer Musicians*) introduces note values as fractions of a whole note: a whole note is four beats, a half note is two beats, a quarter note is one beat, an eighth note is half a beat, a sixteenth note is a quarter of a beat. The PO-33's "step" is exactly a sixteenth note: each step is one quarter of one beat, and four steps make one beat.

When you press a step button while the sequencer is playing, you **toggle** that step. If the step is empty, it becomes occupied (you've just written a note there). If the step is occupied, it becomes empty (you've erased it). This is how you "write" a beat into the PO-33.

### Slots vs. steps

There are two distinct things to keep straight:

- A **slot** is a place to **store** a sound. Slots are numbered 1 to 16. Slot 1 can hold a kick drum. Slot 2 can hold a snare. Slot 15 can hold a hi-hat. Slots are persistent — turn off the device, they remain.
- A **step** is a place to **trigger** a sound. Steps are also numbered 1 to 16, and they loop. Step 5 might play slot 2 on every loop, then step 13 might also play slot 2, then step 14 might play slot 15.

Slots are the cast of characters. Steps are the choreography. The same kick drum (slot 2) might play on step 1, 5, 9, and 13 (a four-on-the-floor pattern), or it might play only on step 1 and step 11 (a syncopated pattern). The slot doesn't change; the steps it plays on do.

### Melodic slots vs. drum slots

The PO-33 splits its 16 slots into two groups:

- **Melodic slots** (slots 1–8): when you play these from the sequencer, the device chooses a **pitch** for you based on which step is playing. Pad 1 plays C4 (middle C), pad 2 plays C#4, pad 3 plays D4, ..., pad 8 plays G#4, pad 16 plays D#5 (the 16th pad triggers D#5 because the chromatic scale is one octave wide — 16 semitones — and pad 1 is C4). This is the **auto-mapping** feature (called F-005 in the RavinePhoenix firmware). It means that, by default, pressing different steps plays different notes — pad 1 is C, pad 8 is G#, pad 16 is D#.
- **Drum slots** (slots 9–16): when you play these from the sequencer, the device plays a **slice** of the recording. **Pad 1 plays the first 1/16th of the active drum slot's recording. Pad 16 plays the last 1/16th.** All 16 pads trigger 16 equal slices, so the entire recording is reachable — pads 1–16 cover slices 1–16. This is the **auto-slicing** feature (F-024). If you record a 4-second break into slot 9, pressing pad 1 plays the first 0.25 seconds, pressing pad 2 plays the next 0.25 seconds, ..., pressing pad 16 plays the last 0.25 seconds.

The two halves are intentionally different. Melodic slots let you play melodies and chords. Drum slots let you play sliced breaks.

> **Sidebar — PO-33 vs. RavinePhoenix.** The original PO-33 maps the 16 melodic pads to one octave of the chromatic scale. The RavinePhoenix firmware does the same. Drum auto-slicing was ❌ missing in our v1 firmware before October 2025; this book is written assuming auto-slicing works (v1.1 firmware and later). On a real PO-33, drum auto-slicing has always worked.

### PO-33 Playback: writing your first beat

To write a single sound into the sequencer:

1. Press **REC** + **pad 1** to record a sound into slot 1 (or use a sound you already recorded in Chapter 1).
2. Press the **PLAY** button (a triangle symbol). The device will start counting through the 16 steps, light the current step on the grid, and play the BPM sound at each step.
3. Press **pad 1**. The device will write slot 1 to the currently-playing step. Actually no — that's not how the PO-33 works either.

Let me start over with the correct flow:

1. Press the **WRITE** button (the PO-33's "·" key) to enter write mode. The screen shows a writing symbol.
2. Press the **PLAY** button to start the sequencer.
3. Press **pad 1**. The slot 1 sample is armed — the device plays the note in slot 1 at every step until you change it.
4. Press **pad 1** again. The slot 1 sample is removed from the current step.
5. Wait for the next step. Press **pad 1**. Slot 1 is now playing on two steps.

Actually, the correct flow is even simpler. The PO-33's sequencer has a "current sound" concept. When you enter write mode and press play:

- The current sound is whatever you last played.
- When the sequencer reaches a step that has a sound written to it, it plays the written sound (overriding the current sound).
- When the sequencer reaches an empty step, it plays the current sound.
- When you press a pad in write mode, you are writing that pad's slot to the next step that the sequencer hits.

This is confusing. Let me describe the simplest possible workflow:

**Workflow 1 — "Tap to write":**

1. Enter write mode by pressing **WRITE**.
2. Press **PLAY** to start the sequencer.
3. Press **pad 1** four times in a row, with one tap per step. The PO-33 will write slot 1 to the next four steps. You now have a sound on every step.

This is rarely what you want. Most of the time you want some steps empty, so you use **Workflow 2 — "Tap to toggle":**

1. Enter write mode by pressing **WRITE**.
2. Press **PLAY**.
3. Press **pad 1** when the current step is one you want the sound on. The PO-33 toggles slot 1 on that step.
4. Wait for the next step you want. Press **pad 1** again. The slot 1 sample is now on two steps.
5. Continue until your pattern is complete. Press **WRITE** to exit write mode.

You can also write a sound to every step in a range by holding the pad while the sequencer steps through that range:

1. Enter write mode.
2. Press **PLAY**.
3. Hold **pad 1** for exactly 4 steps. The slot 1 sample is written to those 4 steps.
4. Continue.

For your first beat, use **Workflow 2**: enter write mode, press play, and tap pad 1 four times — on steps 1, 5, 9, and 13. That's the four-on-the-floor kick pattern. We'll build on it in Chapter 4.

### The sequencer screen

The PO-33's screen shows a **playhead** (the current step) and a **pattern**. The pattern is shown as a row of 16 dots or numbers. A filled dot means "a sound is written here". An empty dot means "nothing plays here". You can also see the **BPM**, the **FX** number, and the **swing** percentage on the screen.

The RavinePhoenix firmware uses a small OLED screen that displays the same information. On the real PO-33 the screen is a smaller LCD. The information is the same.

### Guided exercise: a one-sound, four-step beat

You will need: a sound recorded into slot 1 (from Chapter 1), and the PO-33.

**Success criteria:** the sound in slot 1 plays four times per loop, on steps 1, 5, 9, and 13. Steps 2, 3, 4, 6, 7, 8, 10, 11, 12, 14, 15, 16 are silent.

1. Make sure you have a sound in slot 1. If not, record one: hold REC + pad 1, clap, release REC.
2. Press **WRITE** to enter write mode.
3. Press **PLAY** to start the sequencer.
4. Wait for step 1. Press **pad 1** quickly.
5. Wait for step 5. Press **pad 1**.
6. Wait for step 9. Press **pad 1**.
7. Wait for step 13. Press **pad 1**.
8. Press **WRITE** to exit write mode.

You have just written a four-on-the-floor pattern with a single sound. The sound plays four times per loop. The loop is one bar (16 steps) long. At 120 BPM, the loop is **2 seconds long**.

### Editing a step

To **remove** a sound from a step:

1. Press **WRITE** to enter write mode.
2. Press **PLAY** to start the sequencer.
3. Wait for the step you want to remove. Press **pad 1** (or whatever slot is on that step). The sound toggles off.

To **change** the slot on a step:

1. Enter write mode.
2. Start play.
3. Wait for the step. Press the new pad. The old slot is replaced with the new slot.

To **erase the entire pattern**:

1. Hold **WRITE** + the **erase** button (the PO-33's backspace key — on a real PO-33 it's the bottom-left key of the bottom row; on RavinePhoenix it's the same).
2. Or, on a real PO-33: hold **REC** + **WRITE**. The pattern is wiped.

### Creative challenge: write three patterns with different densities

Write three versions of the same sound:

- **Sparse**: pad 1 only on steps 1 and 13. (2 hits per loop.)
- **Medium**: pad 1 on steps 1, 5, 9, 13. (4 hits per loop.)
- **Dense**: pad 1 on every odd step (1, 3, 5, 7, 9, 11, 13, 15). (8 hits per loop.)

Listen to each. Notice how the sparse pattern feels "open", the medium feels "regular", and the dense feels "busy". The PO-33 has 16 steps, but most music uses **between 2 and 8 hits per loop**. Empty steps are the most underused tool in beat-making.

### Listening assignment

Listen to **Adonis, "No Way Back"** (1986). The kick drum plays on every step (all 16 steps). This is one of the densest kick patterns in house music. It works because the kick is the only sound — no snare, no hi-hat, just kick.

Listen to **J Dilla, "Stop"** from *Donuts* (2006). The kick plays on steps 1 and 11. The snare plays on steps 5 and 13. That's it. Two sounds, four hits per loop. Most Dilla beats are this sparse.

Listen to **DJ Spinn, "Make Me Hollown"** (2014). The kick plays on every step. The hi-hat plays on every odd step. The snare plays on steps 5 and 13. This is a "full" pattern — kick, snare, and hat on every step where they could possibly fit. Notice how busy it feels compared to J Dilla.

### Summary and checkpoint

**Key takeaways:**

- A step is a sixteenth-note. There are 16 steps per loop.
- A slot is a stored sound. There are 16 slots (8 melodic, 8 drum).
- Write mode + PLAY writes a slot to a step. Tap a pad to toggle the current step.
- Most beats use 2–8 hits per loop. Empty steps are a feature.

**Self-check:**

- [ ] Can you write a four-on-the-floor kick pattern (steps 1, 5, 9, 13)?
- [ ] Can you remove a step?
- [ ] Can you change a slot on a step?
- [ ] Do you understand why "less is more" is the PO-33 ethos?

If you answered "yes" to all four, move on to Chapter 3.

---

## Chapter 3 — Tempo, Meter, and the Pulse: Setting Your First BPM

### Objectives

By the end of this chapter you will be able to:

- Explain what BPM means and how it relates to note duration.
- Cycle through the three BPM presets on the PO-33.
- Fine-tune the BPM with Knob B.
- Choose an appropriate BPM for your first beat.

### Musical concept: what is a beat?

A **beat** is the regular pulse you tap your foot to. Almost all popular music has a regular beat. The beat is what makes you nod your head, tap your foot, or dance.

**BPM (beats per minute)** is the number of beats that fit into 60 seconds. At 60 BPM there is one beat per second. At 120 BPM there are two beats per second. At 180 BPM there are three beats per second.

Hewitt (*Music Theory for Computer Musicians*) introduces BPM as the practical answer to "how fast is the music?". He notes that:

- 60–80 BPM is **slow** — ballads, slow rock, some hip-hop.
- 80–110 BPM is **medium** — most pop, R&B, hip-hop.
- 110–130 BPM is **medium-fast** — dance-pop, disco, house.
- 130–160 BPM is **fast** — techno, drum'n'bass, hardstyle.
- 160+ BPM is **very fast** — speedcore, some jungle.

The PO-33's three presets are at 80, 120, and 140 BPM — slow, medium-fast, and fast. These three tempos cover most popular music genres. We will explore genre-specific tempos in Part IV.

### The relationship between BPM and step duration

The PO-33's 16 steps are sixteenth-notes. At 120 BPM:

- One beat = 0.5 seconds.
- One quarter-note = 0.5 seconds.
- One eighth-note = 0.25 seconds.
- One sixteenth-note (one PO-33 step) = 0.125 seconds.
- One bar (16 steps) = 2 seconds.

At 80 BPM (Hip Hop):

- One beat = 0.75 seconds.
- One sixteenth-note = 0.1875 seconds.
- One bar = 3 seconds.

At 140 BPM (Techno):

- One beat ≈ 0.43 seconds.
- One sixteenth-note ≈ 0.107 seconds.
- One bar ≈ 1.71 seconds.

The same 16-step pattern **feels different** at different tempos. At 80 BPM it feels relaxed. At 120 BPM it feels groovy. At 140 BPM it feels urgent. This is one reason the PO-33 has BPM presets — different genres want different feels.

### PO-33 Playback: setting the tempo

**To cycle presets:**

1. Tap the **BPM** button. The screen shows "HIP HOP 80", "DISCO 120", or "TENCHO 140".
2. Tap again to cycle to the next preset.

**To fine-tune:**

1. Hold the **BPM** button.
2. Turn **Knob B**. The BPM changes in 1-BPM increments from 60 to 240.

> **Sidebar — swing.** Hold BPM + turn Knob A to set the swing. The PO-33 has 8 swing levels (0–7). Swing makes off-beat steps play slightly late, which gives the beat a "shuffled" feel. We'll explore swing in Chapter 6.

### Why three presets?

The PO-33's three presets are at 80, 120, and 140 because those are the tempos of three of the most popular genres in the PO-33's lineage:

- **Hip Hop (80 BPM)**: classic boom-bap, J Dilla, Madlib. 80 BPM is the tempo of "Nuthin' but a G Thang", "C.R.E.A.M.", "93 'til Infinity".
- **Disco (120 BPM)**: classic disco, house, French house. 120 BPM is the tempo of "I Will Survive", "Da Funk", "One More Time".
- **Tencho (140 BPM)**: techno, trance, hard house. 140 BPM is the tempo of "Spastik" by Plastikman, "Strings of Life" by Rhythm Is Rhythm.

If you start a beat and it doesn't feel right, **change the tempo first**. It is the cheapest change you can make and the most impactful.

### Guided exercise: the same pattern at three tempos

You will need: the pattern from Chapter 2 (sound on steps 1, 5, 9, 13).

**Success criteria:** you can hear the same pattern feel different at 80, 120, and 140 BPM.

1. Make sure your four-on-the-floor pattern is still in the sequencer. If not, redo Chapter 2's exercise.
2. Set the BPM to Hip Hop (80). Press PLAY. Listen for 30 seconds.
3. Tap the BPM button to cycle to Disco (120). The pattern continues playing at the new speed. Listen for 30 seconds.
4. Tap the BPM button to cycle to Tencho (140). Listen for 30 seconds.
5. Cycle back to Hip Hop (80). Press WRITE to exit write mode.

You should hear the same four hits per loop but at three different feels. The pattern is identical; the tempo is different.

### Creative challenge: match the genre

For each of the following songs, set the PO-33's BPM (using fine-tune with Knob B if needed) so that your pattern plays at the song's tempo:

- "Juicy" by The Notorious B.O. — around 97 BPM.
- "Around the World" by Daft Punk — around 121 BPM.
- "Windowlicker" by Apendix Twin — around 134 BPM.

Press PLAY on the PO-33 and listen to the song. Try to tap your foot in time with both. If they don't sync, fine-tune Knob B up or down by 1 BPM at a time until they do.

### Listening assignment

Listen to **three songs at three tempos**, all in the same session:

1. **A Tribe Called Quest, "Can I Kick It?"** (1991) — 96 BPM. Hip-hop, classic boom-bap. The bassline loops a single sampled phrase, the drums are a break chopped into pieces.
2. **Daft Punk, "Around the World"** (1997) — 121 BPM. House/disco. The bassline is a single repeated note, the drums are a four-on-the-floor.
3. **Underworld, "Born Slippy .NUXX"** (1995) — 137 BPM. Techno. The drums are a four-on-the-floor with off-beat hi-hats.

Notice how the same drum pattern (kick in the same step positions) would feel different at these three tempos. Then notice how each song's drum pattern is actually different — they're not just the same pattern at different tempos. Producers pick patterns that **fit** the tempo, not just play at it.

### Summary and checkpoint

**Key takeaways:**

- BPM = beats per minute. The PO-33's presets are 80, 120, 140.
- One step is a sixteenth-note. At 120 BPM, one step is 0.125 seconds.
- The same pattern feels different at different tempos. Change tempo first if your beat doesn't feel right.
- Fine-tune BPM from 60–240 by holding BPM + turning Knob B.

**Self-check:**

- [ ] Can you cycle through the three BPM presets?
- [ ] Can you fine-tune the BPM by 1 BPM with Knob B?
- [ ] Do you understand why a "kick on every step" pattern feels different at 80 vs. 140 BPM?
- [ ] Can you tap your foot to a song and the PO-33 simultaneously?

If you answered "yes" to all four, you are ready for Part II: rhythm and beat.

---


# Part II — Rhythm and Beat

By the end of Part I you have a single sound that plays on four steps of a 16-step sequencer. That is technically a beat, but it is not yet **interesting**. A real beat has at least three sounds (kick, snare, hi-hat) and they play at different densities. Part II teaches you the building blocks of rhythm — how to layer drum sounds, how to slice a break, and how to make the beat swing.

---

## Chapter 4 — Kick, Snare, Hi-Hat: The Drum Trinity

### Objectives

By the end of this chapter you will be able to:

- Identify the three drum roles — kick, snare, hi-hat — by ear.
- Record (or load) one of each into slots 9–11.
- Place them on a basic 4/4 beat pattern.
- Recognise the pattern in popular music.

### Musical concept: the three drum voices

Almost every drum beat in popular music is built from three voices:

- **Kick drum** (also called **bass drum** or just **kick**): a low-pitched, deep sound with a sharp attack. The kick provides the **low end** and the **downbeat**. It is the heartbeat of the beat.
- **Snare drum** (also called **snare**): a mid-pitched, sharp sound with a noisy attack. The snare provides the **backbeat** (the "hit" that lands on beats 2 and 4 in 4/4 time). It is the clap of the beat.
- **Hi-hat** (also called **hat**): a high-pitched, hissing sound. The hi-hat provides **speed** and **energy**. It is the shaker of the beat.

These three voices have **different frequency ranges**:

- Kick: 40–100 Hz (the bottom).
- Snare: 200 Hz–5 kHz (the middle, with the snare wires adding noise above 1 kHz).
- Hi-hat: 5 kHz–15 kHz (the top).

Because they occupy different frequencies, they can **layer** — play at the same time — without muddying each other. This is why a beat with three voices sounds "fuller" than a beat with one voice. (See Hewitt, *Music Theory for Computer Musicians*, on the "frequency spectrum" of drum sounds.)

> **Sidebar — the PO-33's polyphony.** The PO-33 can play **4 sounds at the same time**. A kick + snare + hi-hat pattern uses only 3 voices, so there is one voice of headroom for a fourth drum sound (a clap, a tom, a percussion loop) or for the start of a bassline. This is plenty for most PO-33 music. If you want to layer more, you'll need to be careful.

### The basic 4/4 beat

The most common drum pattern in popular music is the **four-on-the-floor**:

- Kick on every beat: steps 1, 5, 9, 13.
- Snare on beats 2 and 4: steps 5 and 13.
- Hi-hat on every eighth-note: steps 1, 3, 5, 7, 9, 11, 13, 15.

Wait, that overlaps the kick and snare on steps 5 and 13. Of course — in 4/4 time, the kick plays on beats 1 and 3 (steps 1 and 9), and the snare plays on beats 2 and 4 (steps 5 and 13). Let me correct:

- Kick: steps 1, 9 (beats 1 and 3).
- Snare: steps 5, 13 (beats 2 and 4).
- Hi-hat: steps 1, 3, 5, 7, 9, 11, 13, 15 (every eighth-note).

This is the **basic rock beat**. It is the foundation of disco, house, pop, R&B, and hip-hop. Master it on the PO-33 before you do anything else.

> **Sidebar — Hewitt on backbeat.** Michael Hewitt (*Music Theory for Computer Musicians*) calls the snare on beats 2 and 4 the **backbeat** and identifies it as one of the most universal features of popular music. Almost every song you have ever heard has a backbeat.

### Variations

Once you have the basic beat, you can vary it:

- **Disco/house variation**: kick on every beat (steps 1, 5, 9, 13), snare on 5 and 13, hi-hat on 1, 3, 5, 7, 9, 11, 13, 15. This is the four-on-the-floor.
- **Hip-hop variation**: kick on 1 and 11 (a syncopated "boom-bap"), snare on 5 and 13, hi-hat on every odd step (1, 3, 5, 7, 9, 11, 13, 15).
- **Reggaeton variation**: kick on 1, 3, 5, 7, 9, 11, 13, 15 (every step), snare on 5 and 13, hi-hat on 1, 3, 5, 7, 9, 11, 13, 15.

The differences are tiny but they change the feel completely. Listen to one song in each style and you'll hear the difference immediately.

### PO-33 Playback: building a three-voice beat

You need three drum sounds, one in each of slots 9, 10, 11.

**To record a kick:**

1. Hold REC + pad 9.
2. Make a kick sound (with your mouth: "boom"; or hit a low drum; or sample from your phone — but the PO-33 only has a mic, so use your mouth).
3. Release REC.

**To record a snare:**

1. Hold REC + pad 10.
2. Make a snare sound ("BP" or "tss" or hit a snare drum).
3. Release REC.

**To record a hi-hat:**

1. Hold REC + pad 11.
2. Make a hi-hat sound ("tss tss tss" or a shaker).
3. Release REC.

**To write the basic 4/4 beat:**

1. Press WRITE.
2. Press PLAY.
3. On step 1: press pad 9 (kick) and pad 11 (hi-hat).
4. On step 3: pad 11.
5. On step 5: pad 10 (snare) and pad 11.
6. On step 7: pad 11.
7. On step 9: pad 9 and pad 11.
8. On step 11: pad 11.
9. On step 13: pad 10 and pad 11.
10. On step 15: pad 11.
11. Press WRITE to exit.

You have just written a basic rock beat. With practice, you can write a beat in under 2 minutes.

### Guided exercise: the three-voice beat

**Success criteria:** you have a beat with kick on 1 and 9, snare on 5 and 13, hi-hat on every odd step. It loops cleanly.

1. Record kick, snare, hi-hat into slots 9, 10, 11. (Use your mouth if you don't have drums.)
2. Set BPM to Disco (120).
3. Write the pattern above.
4. Press PLAY and listen.
5. If the kick or snare sounds weak, try recording a new sample. The PO-33 lets you re-record into a slot — just hold REC + the pad again. The new recording replaces the old one.

### Creative challenge: four variants of the basic beat

Write four different beats:

1. **Four-on-the-floor**: kick on 1, 5, 9, 13. Snare on 5, 13. Hi-hat on every odd step.
2. **Boom-bap**: kick on 1, 11. Snare on 5, 13. Hi-hat on every odd step.
3. **Half-time**: kick on 1. Snare on 9. Hi-hat on every odd step.
4. **Double-time**: kick on 1, 5, 9, 13. Snare on 5, 13. Hi-hat on every step (1–16).

Play each one at 120 BPM. Notice how the same three sounds make four completely different feels.

### Listening assignment

Listen to:

- **The Ronettes, "Be My Baby"** (1963). Four-on-the-floor kick, snare on 2 and 4, hi-hat on every eighth-note. This is the basic rock beat, the most influential drum pattern in pop history.
- **N.W.A., "Straight Outta Compton"** (1988). Kick on 1 and 11, snare on 5 and 13, hi-hat on every odd step. This is the boom-bap. The "Compton beat" is the foundation of West Coast hip-hop.
- **J Dilla, "Workinonit"** from *Donuts* (2006). Kick on 1 and 11, snare on 5 and 13, hi-hat with skips. J Dilla's hi-hat pattern is famous for having "holes" — eighth-notes where the hi-hat doesn't play. We will learn to make Dilla-style hi-hat patterns in Chapter 6.

### Summary and checkpoint

**Key takeaways:**

- Three voices: kick (low), snare (mid), hi-hat (high).
- Basic rock: kick 1+9, snare 5+13, hat every odd step.
- Variations: four-on-the-floor, boom-bap, half-time, double-time.
- 4-voice polyphony is enough for kick + snare + hat + one more.

**Self-check:**

- [ ] Can you record three drum sounds into slots 9, 10, 11?
- [ ] Can you write a basic rock beat?
- [ ] Can you hear the difference between four-on-the-floor and boom-bap?
- [ ] Do you understand why kick, snare, and hat occupy different frequencies?

If you answered "yes" to all four, move on to Chapter 5.

---

## Chapter 5 — Slicing a Break: Building Beats from Samples

### Objectives

By the end of this chapter you will be able to:

- Explain what a **break** is and why producers sample them.
- Record a break (or a loop) into a drum slot.
- Use the PO-33's **auto-slicing** to trigger different parts of the break from different steps.
- Recognise a sampled break in popular music.

### Musical concept: what is a break?

A **break** (short for **breakbeat**) is a short drum loop, usually 2–4 seconds long, that has been lifted from an existing song. Hip-hop was born when DJs in the 1970s (notably Kool Herc) realised that the **break** of a funk or soul record — the part where the band drops out and the drummer plays alone — was the most danceable part of the record. They bought two copies of the same record, played the break on both, and used a mixer to **loop** the break continuously while the dancers went wild.

Producers then took the breaks and built entire songs out of them. The **Amen break** (from The Winstons' "Color Him Father", 1969) is the most sampled drum loop in history — it has been used in over 7,000 songs, from N.W.A. to The Prodigy to Skrillex.

The PO-33's drum auto-slicing (F-024 in our firmware) is a direct descendant of this practice. You record a break, and the PO-33 splits it into 16 equal slices. Each step of the sequencer plays one slice. Pad 9 plays the first 1/16th of the break, pad 10 plays the second 1/16th, ..., pad 16 plays the last 1/16th.

> **Sidebar — the ethics of sampling.** The Amen break, the Funky Drummer break, and hundreds of other breaks have been sampled without permission for decades. Some argue this is theft; some argue it is the highest form of musical tribute. In 2024, most major breaks have been cleared for sampling through services like Tracklib and the *Beat Music Hall of Fame*. For practice, sample yourself (clap a break, sample your voice) or use a service that explicitly licenses samples for reuse. See Appendix E.

### The Amen break, deconstructed

The Amen break is 7.2 seconds long at 96 BPM. If you slice it into 16 equal slices, each slice is 0.45 seconds long. The break has:

- Slice 1 (kick on beat 1).
- Slice 2 (hi-hat on the "and" of beat 1).
- Slice 3 (snare on beat 2).
- Slice 4 (hi-hat on the "and" of beat 2).
- Slice 5 (kick on beat 3).
- Slice 6 (hi-hat on the "and" of beat 3).
- Slice 7 (snare on beat 4).
- Slice 8 (hi-hat on the "and" of beat 4).

If you play slices 1, 3, 5, 7 (kick-snare-kick-snare), you get a four-on-the-floor. If you play slices 1, 3, 7, you get a half-time beat. If you play slices 1, 4, 7, 12, you get a syncopated pattern. **The break is the raw material; the pattern is the recipe.**

### How to record a break on the PO-33

The PO-33 can record up to 10 seconds into a single slot. Most breaks are 2–4 seconds, so one slot is enough.

**To record a break:**

1. Find a break you want to sample. The PO-33 has a built-in microphone; you can:
   - Play a break on your phone and hold the PO-33 near the speaker.
   - Sing a beat into the mic ("boom, tss, boom-tss, boom, tss, boom-tss...").
   - Use a sample pack on a computer and play it through a speaker.
2. Hold REC + pad 9.
3. Play the break for ~30 seconds. (You don't need to fill 30 seconds; the PO-33 will stop when you release.)
4. Release REC.
5. Press pad 9 to play the break back.

The break is now in slot 9. Because slot 9 is a drum slot, the PO-33 will **auto-slice** it. **Pad 1 plays the first 1/16th, pad 2 plays the second, ..., pad 16 plays the last 1/16th.** All 16 pads cover the full recording — none of the sample is unreachable.

### Using the trim feature

The PO-33 has a **trim** feature (Tweak Trim mode, FX button triple-tap). Trim lets you set the **start** and **end** of the recording. This is essential for breaks: if you recorded 30 seconds, you probably only want to keep 4 seconds. Trim them down.

**To trim:**

1. Enter Tweak Trim mode (hold FX, triple-tap to cycle TONE → FILTER → TRIM).
2. Turn **Knob A** to move the start point.
3. Turn **Knob B** to move the end point.
4. Press the FX button again to exit trim mode.

For a 4-second break, set the start to the first kick and the end to the last snare. The PO-33's screen shows a waveform; you can see the kicks and snares as big spikes.

> **Sidebar — Knob A and Knob B.** Knob A on the real PO-33 adjusts the **start** in Tweak Trim mode. Knob B adjusts the **end**. The RavinePhoenix firmware maps them the same way. The two knobs are wired to GPIO 2 and GPIO 46 on the ESP32-S3 board.

### Writing a break beat

To write a beat from a sliced break:

1. Enter write mode. Press PLAY.
2. On step 1: press pad 9 (first slice, usually the first kick).
3. On step 5: press pad 11 (third slice, usually the first snare).
4. On step 9: press pad 13 (fifth slice, usually the second kick).
5. On step 13: press pad 15 (seventh slice, usually the second snare).
6. Press WRITE to exit.

You have just written a four-on-the-floor kick-snare pattern out of a sliced break. The kick and snare are not separate recordings — they are **slices of the same break**. This is how producers made beats in the 1990s.

### Guided exercise: build a beat from a 4-second break

You will need: a recorded break in slot 9 (or you can record one in this exercise).

**Success criteria:** you have a beat that uses four different slices of the break, played on steps 1, 5, 9, and 13.

1. Find or create a 4-second break. (Sing "boom, tss, boom, tss, boom, tss, boom, tss" into the mic for 4 seconds. That's a break.)
2. Hold REC + pad 9. Record 4 seconds of the break. Release REC.
3. Trim the recording: enter Tweak Trim mode, set start to the first kick, end to the last snare.
4. Press WRITE. Press PLAY.
5. Press pad 9 on step 1 (first slice). Press pad 11 on step 5 (third slice). Press pad 13 on step 9 (fifth slice). Press pad 15 on step 13 (seventh slice).
6. Press WRITE to exit.

You have built a beat from a sliced break. The same break could be sliced differently to make different beats.

### Creative challenge: chop the Amen

If you have access to the Amen break (it's on YouTube, free to sample), record 4 seconds of it into slot 9. Trim it tight. Then write **three** different beats using different slices:

1. **Standard**: slices 1, 3, 5, 7 (kick-snare-kick-snare).
2. **Half-time**: slices 1, 7 (kick on 1, snare on 9).
3. **Syncopated**: slices 1, 4, 7, 12 (kick on 1, hat on 4, snare on 9, hat on 12).

Listen to each at 140 BPM. Notice how the same 4-second break becomes three different songs.

### Listening assignment

Listen to:

- **N.W.A., "Straight Outta Compton"** (1988). The drums are not three separate recordings — they are a single sampled break, sliced and rearranged. Listen for the "thwack" of the snare: it's not a clean studio snare, it's a sampled break with character.
- **The Prodigy, "Firestarter"** (1996). The drums are the Amen break, chopped and rearranged at 145 BPM. One of the most iconic drum patterns in electronic music, and it is built from a single 7-second sample.
- **Skrillex, "Scary Monsters and Nice Sprites"** (2010). The drop is the Amen break, but chopped into tiny pieces (1/32 or 1/64 of the original) and rearranged into a totally new pattern. Modern dubstep uses breaks the same way the 1990s did.

### Summary and checkpoint

**Key takeaways:**

- A break is a short drum loop, usually 2–4 seconds, often sampled from a funk or soul record.
- The PO-33's auto-slicing splits a break into 16 equal slices, one per pad.
- Trim removes silence and unwanted parts of a recording.
- The same break becomes different songs depending on how you slice and step it.

**Self-check:**

- [ ] Can you record a 4-second break into slot 9?
- [ ] Can you trim the recording to keep only the best parts?
- [ ] Can you write a beat using four different slices?
- [ ] Do you understand why the Amen break has been sampled 7,000 times?

If you answered "yes" to all four, move on to Chapter 6.

---

## Chapter 6 — Syncopation, Swing, and the Human Groove

### Objectives

By the end of this chapter you will be able to:

- Explain what **syncopation** is and why it makes beats feel groovy.
- Use the PO-33's **swing** feature to add shuffle to a beat.
- Recognise swing in popular music.
- Combine syncopation and swing to create a "human" feel.

### Musical concept: what is syncopation?

**Syncopation** is the placement of a note **off the beat**. If a kick drum normally plays on step 1 (the downbeat), a syncopated kick plays on step 1.5 (the "and" of beat 1) or step 0.75 (the "e" of beat 4). The note lands where the listener doesn't expect it, which creates tension and release.

Hewitt (*Music Theory for Computer Musicians*) defines syncopation as "the disruption of the regular pulse by emphasizing a normally unaccented beat or subdivision of the beat". Reggaeton's "dem bow" rhythm is a textbook syncopation — the kick plays on the "and" of every beat, not on the beat itself.

> **Sidebar — PO-33 and syncopation.** The PO-33's 16-step grid has steps at every sixteenth-note. Step 1 is the downbeat of beat 1. Step 2 is the "e" of beat 1. Step 3 is the "and" of beat 1. Step 4 is the "a" of beat 1. Step 5 is the downbeat of beat 2. A syncopated kick on step 3 ("and of 1") is a sixteenth-note ahead of the downbeat.

### Syncopation in popular music

Listen to any song you love. There is a good chance it has at least one syncopated element. Examples:

- **Michael Jackson, "Billie Jean"**: the bassline syncopates against the kick. The kick is on the beat; the bass is on the "and"s.
- **OutKast, "Hey Ya!"**: the kick plays on every step, but the snare plays on the "and"s of beats 2 and 4, not on the beats themselves. The snare is "late" by an eighth-note.
- **Reggaeton, "Gasolina"**: the kick is on every eighth-note (steps 1, 3, 5, 7, 9, 11, 13, 15), not on every beat. This is a syncopated kick that lands on the "and"s.

Syncopation is what makes a beat "groove". A perfectly quantised beat (every kick on every step) sounds robotic. A syncopated beat sounds human.

### What is swing?

**Swing** is a specific kind of timing shift applied to **even-numbered** steps. Instead of step 3 (the "and" of beat 1) playing exactly halfway between steps 1 and 5, swing pushes step 3 **toward** step 5 — usually by 1/3 to 1/2 of the way. The result is that even-numbered steps (the "ands") play late, and odd-numbered steps (the "downs") play early. The beat **shuffles**.

Hewitt (*Music Theory for Computer Musicians*) describes swing as "the unequal subdivision of the beat". A swing beat divides each beat into a long-short pattern instead of an equal pattern. The ratio is usually 2:1 (the long note is twice as long as the short note) but can be 3:1 for "heavy swing".

The PO-33 has **8 swing levels** (0 to 7). Level 0 is no swing; level 7 is extreme shuffle (the "ands" play very late). Level 2 or 3 is typical for hip-hop. Level 4 is typical for house. Level 6 is typical for shuffle / jazz.

> **Sidebar — jazz vs. electronic swing.** Jazz swing pushes the "ands" of beats 2 and 4 (the snare hits) late. Electronic swing (house, hip-hop) often pushes the "ands" of all beats (the hi-hat hits) late. The PO-33's swing is electronic-style: it affects every even-numbered step.

### The math of swing

Without swing, step 3 plays at exactly 0.125 seconds after step 1 (at 120 BPM). With 50% swing, step 3 plays at 0.1667 seconds after step 1 — one-third of the way to step 5 instead of halfway. The "long-short" subdivision is 2:1.

The PO-33's swing levels are roughly:

- Level 0: no swing (1:1 subdivision).
- Level 1: 51% swing (subdivision ≈ 1.04:1).
- Level 2: 55% swing (1.22:1).
- Level 3: 60% swing (1.5:1).
- Level 4: 67% swing (2:1).
- Level 5: 73% swing (2.7:1).
- Level 6: 80% swing (4:1).
- Level 7: 88% swing (7:1).

### PO-33 Playback: setting swing

**To set swing:**

1. Hold **BPM**.
2. Turn **Knob A**. The screen shows the swing level (0–7).
3. Release BPM.

Swing is global — it applies to every step of every pattern. You can't have a "no swing" pattern and a "swing" pattern at the same time. But you can change swing between patterns if you chain patterns (Chapter 15).

### How swing sounds

Try this:

1. Build a basic beat from Chapter 4: kick on 1+9, snare on 5+13, hi-hat on every odd step.
2. Set swing to 0. Listen. The beat is straight, robotic.
3. Set swing to 3. Listen. The hi-hat pushes forward, the kick and snare pull back. The beat shuffles.
4. Set swing to 6. Listen. The hi-hat is way late. This is "trap" or "juke" style.
5. Set swing to 7. Listen. The hi-hat is so late it sounds broken. Don't use this in a real song unless you mean it.

### Syncopation without swing

You can also create syncopation by simply **not placing sounds on every step**. For example:

- A kick on steps 1 and 11 (boom-bap).
- A snare on steps 5 and 13 (backbeat).
- A hi-hat on steps 1, 3, 5, 7, 9, 11, 13, 15 (every eighth-note).

This is the J Dilla pattern: kick on 1 and 11 (not 1 and 9), snare on 5 and 13. The kick is **late** by one sixteenth-note on beat 3. This is a syncopation, and it is what made Dilla's beats famous.

To make a Dilla-style beat, just remove the kick from step 9 and add it to step 11. Easy. The challenge is making **every** beat sound like Dilla's, which is beyond this book — but the principle (kick on 11, not 9) is the seed.

### Guided exercise: the same beat at three swing levels

**Success criteria:** you can hear the difference between swing 0, swing 3, and swing 6 on the same beat.

1. Build a basic beat: kick on 1+9, snare on 5+13, hi-hat on every odd step.
2. Hold BPM, turn Knob A to 0. Press PLAY. Listen for 30 seconds.
3. Turn Knob A to 3. Listen for 30 seconds.
4. Turn Knob A to 6. Listen for 30 seconds.
5. Turn Knob A back to 0 (or 2, if you want a "default" hip-hop swing).

Notice how the hi-hat changes the most. The kick and snare change less (because they are on the odd steps). Swing affects every step, but the human ear notices the change most on the **even-numbered** steps.

### Creative challenge: the Dilla beat

Build a beat that uses only steps 1, 5, 11, and 13 for the kick (the "Dilla kick"), and only steps 5 and 13 for the snare. Add a hi-hat that skips every other eighth-note (1, 5, 9, 13 — a half-time hi-hat). Set swing to 3.

Listen. If it doesn't immediately feel "Dilla-esque", remove the snare from step 13 and add it to step 15. Then move the kick from 11 to 12. Keep adjusting until it grooves.

### Listening assignment

Listen to:

- **J Dilla, "Workinonit"** from *Donuts* (2006). The kick is on steps 1 and 11. The snare is on steps 5 and 13. The hi-hat is sparse, with rests. Swing is heavy. This is the "Dilla beat" in its purest form.
- **Madlib, "Accordion"** from *Madvillainy* (2004). The kick plays only on step 1. The snare plays only on step 13. A single accordion sample loops. This is "less is more" at its most extreme.
- **Travis Scott, "SICKO MODE"** (2018). The beat uses heavy swing (level 5 or 6). The hi-hat lands late. This is modern trap, descended from the Dilla tradition.
- **Daft Punk, "Around the World"** (1997). Almost no swing (level 1). The beat is straight, mechanical, but **also** groovy because of the layering. Notice how a "no swing" beat can still groove.

### Summary and checkpoint

**Key takeaways:**

- Syncopation = notes off the beat. Swing = even-numbered steps play late.
- Swing makes a beat feel groovy; no swing makes it feel mechanical.
- The PO-33 has 8 swing levels (0–7). Level 3 is good for hip-hop; level 4 for house.
- Dilla-style beats use kick on 1+11 (not 1+9) and heavy swing.

**Self-check:**

- [ ] Can you set the PO-33's swing to 3 and hear the difference from swing 0?
- [ ] Can you write a Dilla-style beat (kick on 1+11, snare on 5+13)?
- [ ] Can you explain why a kick on step 11 is "late"?
- [ ] Do you understand the difference between syncopation (note placement) and swing (timing shift)?

If you answered "yes" to all four, you are ready for Part III: melody and harmony.

---


# Part III — Melody and Harmony

Up to now your beats have been drums only. A drum-only loop is a **rhythm track**. A **song** has melody too: notes that go up and down, chords that change, a bassline that anchors the harmony. Part III introduces the melodic slots (slots 1–8), the chromatic pad layout, scales, intervals, basslines, and simple chord progressions. By the end of Part III you will have a beat with kick, snare, hat, bass, and lead — a full arrangement.

---

## Chapter 7 — Pitches, Notes, and the Chromatic Pad

### Objectives

By the end of this chapter you will be able to:

- Explain what **pitch** is and how it relates to frequency.
- Identify the 16 notes of the chromatic scale (C, C#, D, D#, E, F, F#, G, G#, A, A#, B, C, C#, D, D#).
- Play a melody on the PO-33 by tapping different pads in sequence.
- Recognise the difference between a stepwise melody and a leaping melody.

### Musical concept: what is pitch?

**Pitch** is how high or low a note sounds. A bass guitar is low-pitched. A piccolo is high-pitched. Pitch is measured in **Hertz (Hz)**, which is cycles per second. Middle C (C4) is 261.63 Hz. The A above middle C (A4) is 440 Hz. The C below middle C (C3) is 130.81 Hz — half the frequency of C4.

Hewitt (*Music Theory for Computer Musicians*) introduces pitch as "the perceived frequency of a sound". Lower frequencies sound lower; higher frequencies sound higher. The relationship between pitches is what we call **music**.

Two pitches that are **twice** the frequency of each other sound the same — they are an **octave** apart. C3 (130.81 Hz) and C4 (261.63 Hz) are an octave apart. C4 and C5 (523.25 Hz) are an octave apart. The octave is the most fundamental interval in music.

> **Sidebar — the chromatic scale.** Between C and the next C there are **12 equal steps** on the modern Western scale. These 12 steps are called **semitones**. C → C# is one semitone, C# → D is one semitone, ..., B → C is one semitone. The 12 semitones of one octave are: C, C#, D, D#, E, F, F#, G, G#, A, A#, B. The 13th note (the next C) is the start of the next octave.

### The PO-33's chromatic pad layout

The PO-33's melodic slots (1–8) play a chromatic scale one octave wide. Specifically, the firmware's auto-mapping helper (F-005) assigns:

- Pad 1: C4 (middle C).
- Pad 2: C#4.
- Pad 3: D4.
- Pad 4: D#4.
- Pad 5: E4.
- Pad 6: F4.
- Pad 7: F#4.
- Pad 8: G4.
- Pad 9: G#4.
- Pad 10: A4.
- Pad 11: A#4.
- Pad 12: B4.
- Pad 13: C5 (one octave above middle C).
- Pad 14: C#5.
- Pad 15: D5.
- Pad 16: D#5.

So pressing pads in sequence 1, 3, 5, 6, 8, 10, 13 plays the notes C, D, E, F, G, A, C — the C major scale in root position.

> **Sidebar — the original PO-33 vs. RavinePhoenix.** The original PO-33 has the same mapping: pad 1 = C4, pad 16 = D#5. The RavinePhoenix firmware's `amy_bridge_auto_note_for_step()` helper implements this. We use MIDI note numbers 60 (C4) to 75 (D#5).

### Stepwise motion vs. leaps

A **stepwise** melody moves by one or two semitones at a time. Pad 1 → pad 2 → pad 3 is stepwise (C → C# → D). A **leaping** melody jumps by 5+ semitones. Pad 1 → pad 8 is a leap (C to G, a perfect fifth).

Stepwise motion is calm and singable. Leaping motion is dramatic. Most melodies use a mix of steps and leaps. A melody that only steps is boring. A melody that only leaps is exhausting.

### PO-33 Playback: playing a melody

To play a melody on the PO-33, you simply **press pads in sequence**. Each pad is a different note.

**To write a melody into the sequencer:**

1. Record a melodic sound into slot 1 (hold REC + pad 1, sing a single note, release REC).
2. Press WRITE. Press PLAY.
3. On step 1, press pad 1 (C4).
4. On step 3, press pad 3 (D4).
5. On step 5, press pad 5 (E4).
6. On step 7, press pad 8 (G4).
7. On step 9, press pad 6 (F4).
8. On step 11, press pad 5 (E4).
9. On step 13, press pad 3 (D4).
10. On step 15, press pad 1 (C4).
11. Press WRITE to exit.

You have just written "Twinkle Twinkle Little Star" into the PO-33. The pattern is: C, D, E, F, G, F, E, D, C. This is the simplest melody in Western music — and now it lives on your PO-33.

### Trim and tuning

The PO-33's **Tweak Tone** mode lets you adjust the **pitch** and **volume** of a melodic slot. By default, the slot plays at its recorded pitch (C4 if you sang a C, A4 if you sang an A). You can shift the pitch up or down by an octave with Knob A or B.

To enter Tweak Tone mode: hold FX, single-tap to enter TONE mode (the first mode after power-up). Knob A adjusts pitch (up/down by semitone). Knob B adjusts volume. The screen shows the current pitch in semitones.

> **Sidebar — pitch shifting.** If you recorded a vocal at A3 (220 Hz) but you want it to play at A4 (440 Hz), shift the pitch up by 12 semitones (one octave). The PO-33 does pitch shifting via AMY's resampler. Quality degrades at extreme shifts (more than one octave), so keep shifts small.

### Guided exercise: write "Mary Had a Little Lamb"

**Success criteria:** you have a melody that plays the notes E, D, C, D, E, E, E, D, D, D, E, G, G (the "Mary Had a Little Lamb" melody in C major).

1. Record a melodic sound into slot 1 (sing a single note — any pitch is fine; the PO-33 will remap it to C4).
2. Press WRITE, PLAY.
3. Step 1: pad 5 (E).
4. Step 3: pad 3 (D).
5. Step 5: pad 1 (C).
6. Step 7: pad 3 (D).
7. Step 9: pad 5 (E).
8. Step 11: pad 5 (E).
9. Step 13: pad 5 (E).
10. Step 15 (wait — there are 8 notes left; pad 3 (D) twice, pad 3 (D) twice, pad 5 (E) once, pad 7 (G) once, pad 7 (G) once).

Actually, "Mary Had a Little Lamb" has 13 notes. The PO-33 has 16 steps. Use steps 1–13. The last three steps (14, 15, 16) are empty. The pattern will loop every 16 steps, but only 13 of those steps have notes.

11. Press WRITE to exit.

You have written "Mary Had a Little Lamb". Listen to it at 80 BPM (Hip Hop). It sounds...wrong. Why? Because the melody is meant to be played at 100–120 BPM, and at 80 BPM each note is held too long.

12. Tap BPM to cycle to Disco (120). Now it sounds more like a song.

### Creative challenge: write your own melody

Pick a melody you know by heart (a nursery rhyme, a TV theme, a pop song hook). Write it into the PO-33 by tapping the pads in sequence.

If you don't know any melodies, sing "do re mi fa sol la ti do" (the solfège scale) and write that. It's eight notes, one per odd step.

### Listening assignment

Listen to:

- **The Beatles, "Yesterday"** (1965). The melody is stepwise: F → E → F → E → F → E → D → C → B → A → G → F. Almost every interval is a step. The song is famous because the melody is so singable.
- **The Beatles, "A Hard Day's Night"** (1964). The famous opening chord is a chord, not a single note — we'll get to chords in Chapter 10. But the melody that follows is leaping: G → F# → G → A → G → F# → G → A. Big leaps.
- **Beethovern, "Für Elise"** (1810). The melody is mostly stepwise, but it includes one big leap (the "E-D#-E" motif). Most classical melodies mix steps and leaps.

### Summary and checkpoint

**Key takeaways:**

- Pitch is frequency. Middle C is 261.63 Hz. Octaves double the frequency.
- The chromatic scale has 12 semitones per octave. PO-33 pads 1–16 play C4 through D#5.
- Stepwise melodies are singable. Leaping melodies are dramatic. Mix them.
- Tweak Tone mode shifts pitch (Knob A) and volume (Knob B).

**Self-check:**

- [ ] Can you name the 12 notes of the chromatic scale?
- [ ] Can you play "Mary Had a Little Lamb" on the PO-33 pads?
- [ ] Can you explain why stepwise melodies are singable?
- [ ] Can you use Tweak Tone to shift the pitch of a recorded sample?

If you answered "yes" to all four, move on to Chapter 8.

---

## Chapter 8 — Scales and Intervals: Why Some Notes Sound Good Together

### Objectives

By the end of this chapter you will be able to:

- Explain what an **interval** is and name the common intervals.
- Build a **major scale** and a **minor scale** on the PO-33's chromatic pad.
- Explain why some notes "go together" and others clash.
- Recognise major and minor scales by ear.

### Musical concept: what is an interval?

An **interval** is the distance between two pitches. The smallest interval (a single semitone, like C to C#) is called a **minor second** or **semitone**. The largest interval in a single octave (twelve semitones, like C to C) is an **octave**.

The most important intervals are:

- **Minor 2nd (1 semitone)**: C → C#. Dissonant — sounds tense.
- **Major 2nd (2 semitones)**: C → D. Consonant — sounds happy.
- **Minor 3rd (3 semitones)**: C → D#. Consonant — sounds sad.
- **Major 3rd (4 semitones)**: C → E. Consonant — sounds happy.
- **Perfect 4th (5 semitones)**: C → F. Consonant — sounds open.
- **Tritone (6 semitones)**: C → F#. Dissonant — sounds "spooky".
- **Perfect 5th (7 semitones)**: C → G. Consonant — sounds stable.
- **Minor 6th (8 semitones)**: C → G#. Dissonant.
- **Major 6th (9 semitones)**: C → A. Consonant.
- **Minor 7th (10 semitones)**: C → A#. Dissonant.
- **Major 7th (11 semitones)**: C → B. Dissonant.
- **Octave (12 semitones)**: C → C. Consonant — sounds the same.

Allen (*Music Theory for Electronic Music Producers*) calls the tritone "the most dissonant interval in Western music". The tritone appears in jazz (the "Devil's interval") and in heavy metal. In pop music, tritones are usually avoided.

> **Sidebar — consonance and dissonance.** These are not absolute categories. They are cultural and historical. Medieval listeners found major thirds dissonant; modern listeners find them consonant. The intervals themselves haven't changed; our ears have. Listen to a piece of medieval polyphony and notice how the major thirds sound "tense" — that's because your ear has been trained by modern music to hear them as consonant.

### Scales

A **scale** is a selection of 7 notes (out of the 12 in an octave) that "go together". The most common scales are:

- **Major scale**: a pattern of intervals that sounds "happy". The C major scale is C, D, E, F, G, A, B, C. Intervals: 2-2-1-2-2-2-1 (semitones from C).
- **Natural minor scale**: a pattern of intervals that sounds "sad". The C minor scale is C, D, D#, F, G, G#, A#, C. Intervals: 2-1-2-2-1-2-2.
- **Pentatonic scale**: a 5-note scale that "always sounds good". The C major pentatonic is C, D, E, G, A. No semitones.
- **Blues scale**: a 6-note scale with a "blue" note. C, D, D#, E, G, A.

The major and minor scales are the foundation of Western music. Pentatonic and blues scales are the foundation of popular music (rock, blues, country, folk).

### Building a major scale on the PO-33

The C major scale on PO-33 pads:

- Pad 1: C.
- Pad 3: D.
- Pad 5: E.
- Pad 6: F.
- Pad 8: G.
- Pad 10: A.
- Pad 12: B.
- Pad 13: C (one octave up).

That's 8 notes (the 13th is the octave). To play the scale, tap pads in order: 1, 3, 5, 6, 8, 10, 12, 13.

### Building a minor scale on the PO-33

The C natural minor scale on PO-33 pads:

- Pad 1: C.
- Pad 3: D.
- Pad 4: D#.
- Pad 6: F.
- Pad 8: G.
- Pad 9: G#.
- Pad 11: A#.
- Pad 13: C.

To play the minor scale: 1, 3, 4, 6, 8, 9, 11, 13.

### Why some notes "go together"

Notes "go together" because they share **overtones** (also called **harmonics**). When you play a single note, you hear not just the fundamental frequency but also its overtones — multiples of the fundamental (2×, 3×, 4×, ...). Two notes that share overtones sound consonant. Two notes that don't, sound dissonant.

For example, C and G share overtones 2×, 3×, 4× — many shared partials. C and C# share only overtone 2× — fewer shared partials, hence more dissonance.

This is why the **perfect fifth** (C to G) is so consonant: it has the most shared overtones of any interval except the octave.

> **Sidebar — Hewitt on harmonics.** Hewitt (*Music Theory for Computer Musicians*) has an entire chapter on harmonics. He explains how the overtone series (2×, 3×, 4×, ...) gives rise to the intervals of Western music. The fifth (3:2 ratio) and the fourth (4:3 ratio) are the simplest, hence the most consonant. The tritone (45:32 ratio) is the most complex, hence the most dissonant.

### PO-33 Playback: writing a melody that stays in key

To write a melody that sounds "right" (i.e., not jarring), restrict yourself to the notes of a single scale. Pick C major: pads 1, 3, 5, 6, 8, 10, 12, 13.

Write a melody using only those pads. Any combination of those pads will sound "in key" — they will not clash.

To write a melody that sounds "wrong", use the notes outside the scale. For C major, the forbidden pads are 2, 4, 7, 9, 11, 14, 15, 16 (C#, D#, F#, G#, A#, C#5, D5, D#5). A melody that uses those pads will sound dissonant — sometimes intentionally, sometimes not.

### Guided exercise: a C major scale melody

**Success criteria:** a melody that uses only the 7 notes of C major (pads 1, 3, 5, 6, 8, 10, 12) and ends on pad 1 (C, the **tonic** — the "home" note).

1. Record a melodic sound into slot 1.
2. Press WRITE, PLAY.
3. Tap pads in this sequence, on steps 1, 3, 5, 7, 9, 11, 13, 15: 1, 3, 5, 6, 8, 10, 12, 13.
4. Press WRITE to exit.

This is a C major scale played as a melody. It should sound "open" and "resolved". Compare to a minor scale (pads 1, 3, 4, 6, 8, 9, 11, 13) — it should sound "sadder".

### Creative challenge: a melody in C minor

Write the same exercise but in C minor. Use pads 1, 3, 4, 6, 8, 9, 11, 13. Listen to the difference between major and minor. The difference is **one note** (the third: E in major, D# in minor). One note changes happy to sad. This is the most powerful tool in music.

### Listening assignment

Listen to:

- **The Beatles, "Let It Be"** (1970). The chord progression is C → G → Am → F (we will get to chords in Chapter 10). The melody is mostly stepwise in C major. The song sounds "hopeful" — major scale, simple chords.
- **Adele, "Someone Like You"** (2011). The chord progression is C → G → Am → F → C → G → F → G...wait, same chords as "Let It Be"! But Adele's version sounds "sad". Why? The melody uses notes from C minor (not C major). Same chords, different scale — that's the difference between happy and sad.
- **Led Zeppelin, "Stairway to Heaven"** (1971). The intro is in A minor. The famous chord progression descends: Am → G → F → E → D → C → D → Am. Each chord is a step lower. The song moves from minor (sad) to major (resolved) over 8 minutes.

### Summary and checkpoint

**Key takeaways:**

- An interval is the distance between two pitches. Common intervals are named (minor 2nd, major 3rd, etc.).
- A scale is a selection of 7 notes. Major scales sound happy; minor scales sound sad.
- Pentatonic scales (5 notes) "always sound good". The C major pentatonic is pads 1, 3, 5, 8, 10.
- Notes "go together" because they share overtones.

**Self-check:**

- [ ] Can you build a C major scale on the PO-33 pads?
- [ ] Can you build a C minor scale on the PO-33 pads?
- [ ] Can you write a melody using only the notes of C major?
- [ ] Do you understand why the perfect fifth is so consonant?

If you answered "yes" to all four, move on to Chapter 9.

---

## Chapter 9 — Basslines: The Foundation Under Everything

### Objectives

By the end of this chapter you will be able to:

- Explain what a **bassline** is and why every song needs one.
- Write a simple bassline using the PO-33's melodic slots.
- Use the **root** and **fifth** of a chord in your bassline.
- Recognise a bassline in popular music.

### Musical concept: what is a bassline?

A **bassline** is a low-pitched melody that runs underneath the main melody. The bassline provides **harmonic foundation** — it tells the listener what chord is playing. The kick drum provides **rhythmic foundation**; the bassline provides **harmonic foundation**.

In a 4-piece band (bass, drums, guitar, vocals), the bass plays the bassline. In a 4-voice PO-33 (kick, snare, hi-hat, melody), the bass plays the bassline — using one of the melodic slots.

The bassline is usually played in **octaves below** the main melody. If the main melody plays C4 (middle C), the bass plays C3 (one octave below). The PO-33's melodic slots play at their recorded pitch by default. To make a slot sound "bass-like", record a low-pitched sound (a low vocal, a sampled bass guitar, a synth bass patch) into it. Or use Tweak Tone to pitch-shift a recorded sample down.

> **Sidebar — Allen on basslines.** Allen (*Music Theory for Electronic Music Producers*) emphasises that the bassline is what holds the song together. A great melody with a weak bassline sounds thin. A weak melody with a great bassline sounds full. The bassline is more important than the melody.

### The root and the fifth

Most basslines use only two notes: the **root** and the **fifth** of the chord. In C major:

- **Root**: C.
- **Fifth**: G.

A bassline that alternates C and G will work for almost any chord in C major. This is the simplest possible bassline — and it is the foundation of half of all pop music.

For a more interesting bassline, add the **octave** (C → C, one octave up) and the **third** (C → E for major, C → D# for minor).

The classic pattern: root → fifth → octave → fifth. In C major on PO-33 pads:

- Pad 1 (C): root.
- Pad 8 (G): fifth.
- Pad 13 (C, one octave up): octave.
- Pad 8 (G): fifth.

Played as a sixteenth-note pattern: 1, 8, 13, 8, 1, 8, 13, 8. This is the bassline of "Another One Bites the Dust" by Queen (1980) — almost exactly.

### How to write a bassline on the PO-33

The PO-33's bassline is just another melodic slot. Use slot 2 for the bass (slot 1 is reserved for the lead melody; we will reserve slot 3 for a pad chord in Chapter 10).

**To write a bassline:**

1. Record a low-pitched sound into slot 2 (hum a low note, sample a bass guitar, etc.).
2. Use Tweak Tone to lower the pitch by 12 semitones (one octave) if needed. Knob A in Tweak Tone mode shifts pitch.
3. Press WRITE, PLAY.
4. On step 1, press pad 1 (root).
5. On step 5, press pad 8 (fifth).
6. On step 9, press pad 13 (octave).
7. On step 13, press pad 8 (fifth).
8. Press WRITE to exit.

You have written a basic bassline. It plays under whatever other patterns you have in slots 9, 10, 11 (drums) and slot 1 (lead melody).

> **Sidebar — the PO-33's voice limit.** The PO-33 plays 4 voices simultaneously. If you have kick + snare + hi-hat + bass, that's 4 voices — no headroom for a melody. To add a melody, **either** remove the hi-hat (and play hats on a separate pattern) **or** skip the bassline and let the bass be the melody. We'll discuss voice management in Chapter 16.

### Guided exercise: the C-G bassline

**Success criteria:** a bassline that plays C → G → C → G → C → G → C → G over 8 steps.

1. Record a low sound into slot 2. If you don't have a bass, hum a low note and pitch-shift it down.
2. Press WRITE, PLAY.
3. On steps 1, 3, 5, 7, 9, 11, 13, 15: press pad 1 (C).
4. Press WRITE to exit.

Wait, that's just a constant C. Let me give you a better exercise:

1. Record a bass sound into slot 2.
2. Press WRITE, PLAY.
3. Step 1: pad 1 (C).
4. Step 3: pad 8 (G).
5. Step 5: pad 13 (C).
6. Step 7: pad 8 (G).
7. Step 9: pad 1 (C).
8. Step 11: pad 8 (G).
10. Step 13: pad 13 (C).
11. Step 15: pad 8 (G).
12. Press WRITE to exit.

This is an 8-note bassline that alternates root and fifth. It will sound "steady" and "anchored". Pair it with a drum beat from Chapter 4 (kick on 1+9, snare on 5+13, hat every odd step) and a melody from Chapter 7 (a C major scale melody). You have a complete song.

### Creative challenge: the walking bassline

A **walking bassline** is a bassline that plays a different note every beat. In jazz, walking basslines are the norm. In pop, walking basslines are less common but can add movement.

Write a walking bassline in C major:

- Step 1: pad 1 (C).
- Step 3: pad 3 (D).
- Step 5: pad 5 (E).
- Step 7: pad 6 (F).
- Step 9: pad 8 (G).
- Step 11: pad 10 (A).
- Step 13: pad 12 (B).
- Step 15: pad 13 (C).

This is a "scale-walking" bassline. It moves stepwise up the C major scale. Pair it with a slow tempo (80 BPM) and a chord progression (we'll learn this in Chapter 10) and you have a jazz-flavoured track.

### Listening assignment

Listen to:

- **Queen, "Another One Bites the Dust"** (1980). The bassline is C → G → C → G → C → G → C → G. It is exactly the bassline we just wrote. The song is built on that bassline.
- **Daft Punk, "Around the World"** (1997). The bassline is a single note repeated — G, G, G, G. Sometimes it adds an F. It is the simplest possible bassline, and the song is one of the biggest dance hits of all time.
- **Pink Floyd, "Money"** (1973). The bassline is in 7/4 time (an unusual time signature) and uses a minor scale. It is one of the most famous basslines in rock history.
- **Michael Jackson, "Billie Jean"** (1982). The bassline plays the root and fifth of the chord, alternating every beat. Classic disco-funk.

### Summary and checkpoint

**Key takeaways:**

- A bassline is a low-pitched melody that provides harmonic foundation.
- The most common bassline notes are the root and fifth of the chord.
- On the PO-33, the bassline lives in a melodic slot (slot 2 is a good choice).
- Watch the 4-voice polyphony: bass + 3 drums fills the device.

**Self-check:**

- [ ] Can you write a C → G → C → G bassline?
- [ ] Can you record a low-pitched sound into slot 2?
- [ ] Do you understand why the bass plays the root and fifth?
- [ ] Can you pair a bassline with a drum beat and a melody without exceeding 4 voices?

If you answered "yes" to all four, move on to Chapter 10.

---

## Chapter 10 — Chords and Chord Progressions: Harmony for One Octave

### Objectives

By the end of this chapter you will be able to:

- Explain what a **chord** is and how it is built.
- Build a major triad, a minor triad, and a 7th chord on the PO-33.
- Write a 4-chord progression in the key of C major.
- Recognise common chord progressions in popular music.

### Musical concept: what is a chord?

A **chord** is **three or more notes played at the same time**. The most basic chord is a **triad** — three notes stacked in thirds. A major triad is built from the root, the major third, and the perfect fifth. A minor triad is built from the root, the minor third, and the perfect fifth.

In C major:

- **C major triad**: C, E, G (pads 1, 5, 8).
- **C minor triad**: C, D#, G (pads 1, 4, 8).

In C major:

- **F major triad**: F, A, C (pads 6, 10, 13).
- **G major triad**: G, B, D (pads 8, 12, 15).

The notes of the C major scale can be combined into 7 triads:

- C major (pads 1, 5, 8).
- D minor (pads 3, 6, 10).
- E minor (pads 5, 8, 12).
- F major (pads 6, 10, 13).
- G major (pads 8, 12, 15).
- A minor (pads 10, 13, 1 — but 1 is the next octave, so use pad 1 instead).
- B diminished (pads 12, 15, 4 — but 4 is dissonant; skip).

Notice that **three of the seven chords are major** (C, F, G) and **three are minor** (D, E, A). The pattern is: in any major key, the I, IV, and V chords are major; the ii, iii, and vi chords are minor; the vii° chord is rare.

> **Sidebar — Roman numerals.** Allen (*Music Theory for Electronic Music Producers*) uses Roman numerals to label chord progressions. The chord built on the first note of the scale (C in C major) is **I**. The chord built on the fourth note (F) is **IV**. The chord built on the fifth note (G) is **V**. So the progression I → IV → V is C → F → G. The progression vi → IV → I is Am → F → C. Roman numerals are universal: "I → V → vi → IV" is the same progression in every key.

### The I-V-vi-IV progression

The most famous chord progression in modern pop is **I → V → vi → IV**. In C major, this is:

- I = C major (pads 1, 5, 8).
- V = G major (pads 8, 12, 15).
- vi = A minor (pads 10, 13, 1).
- IV = F major (pads 6, 10, 13).

Songs that use this progression: "Let It Be" (Beatles), "Someone Like You" (Adele), "No Woman No Cry" (Bob Marley), "Africa" (Toto), "Don't Stop Believin'" (Journey), "Despacito" (Luis Fonsi), and over 1,000 other songs. It is **the** chord progression.

### The PO-33 and chords

The PO-33 can play chords in a melodic slot by triggering three pads at the same time. But the PO-33 has only 4 voices of polyphony, so a 3-note chord uses 3 of those voices — leaving only 1 for the bass or the drums.

**To write a chord progression:**

1. Record a sustained melodic sound into slot 3 (a synth pad, a vocal "ahh", a guitar chord).
2. Press WRITE, PLAY.
3. Step 1: press pads 1, 5, 8 simultaneously (C major chord).
4. Step 5: press pads 8, 12, 15 simultaneously (G major chord).
5. Step 9: press pads 10, 13, 1 simultaneously (A minor chord).
6. Step 13: press pads 6, 10, 13 simultaneously (F major chord).
7. Press WRITE to exit.

This is the I-V-vi-IV progression in C major. It will loop every 16 steps.

> **Sidebar — sustained notes.** If your sample is short (1 second), the chord will cut off after 1 second. To make a chord last the whole 4 beats, your sample needs to be at least 0.5 seconds long. Longer is better — the chord will sustain across all 4 beats. Use a sustained sound ("ahhh", a synth pad).

### Voice management

The PO-33 has 4 voices. A typical track might use:

- 1 voice: kick drum (slot 9).
- 1 voice: snare drum (slot 10).
- 1 voice: hi-hat (slot 11).
- 1 voice: bassline (slot 2).

That's 4 voices. Adding a chord progression uses 3 voices for the chord alone — too many.

**Solutions:**

1. **Skip the hi-hat**. The hi-hat is the most expendable voice. Replace it with a cymbal sample that plays only every 4 steps.
2. **Use a one-note bassline**. If the bassline plays only one note (the root), you can use it as part of the chord. The bass voice plays the root while the chord plays the third and fifth.
3. **Drop the chord**. The bassline + drums is enough for many songs. Add the chord only on certain steps.

In Part V we'll learn pattern chaining, which gives you 8 voices of *two patterns* (16 voices of total polyphony across both). The PO-33 is not a multitrack DAW; you have to manage voices.

### Guided exercise: the I-V-vi-IV progression

**Success criteria:** a 4-chord progression that cycles C → G → Am → F every 4 steps.

1. Record a sustained sound into slot 3 (sing "ahhh" and let it ring for 2 seconds).
2. Press WRITE, PLAY.
3. Step 1: press pads 1, 5, 8 (C major).
4. Step 5: press pads 8, 12, 15 (G major).
5. Step 9: press pads 10, 13, 1 (A minor).
6. Step 13: press pads 6, 10, 13 (F major).
7. Press WRITE to exit.

Listen to it loop. You should hear a clear chord progression that resolves into the F major chord at step 13. The chord returns to C on the next loop.

### Creative challenge: the I-IV-V-I progression

The classic "three chords and a resolution":

- I = C major (pads 1, 5, 8).
- IV = F major (pads 6, 10, 13).
- V = G major (pads 8, 12, 15).
- I = C major (pads 1, 5, 8).

Write this progression into steps 1, 5, 9, 13 of slot 3. Listen. This is the chord progression of "Twist and Shout", "La Bamba", and 80% of blues music.

### Listening assignment

Listen to:

- **The Beatles, "Let It Be"** (1970). I → V → vi → IV in C major. The same progression as the one we just wrote.
- **Adele, "Someone Like You"** (2011). Same progression as "Let It Be", but the melody uses notes from C minor. Same chords, different melody = different feel.
- **Journey, "Don't Stop Believin'"** (1981). I → V → vi → IV in E major. The same progression, transposed to a different key. The progression works in every key.
- **Toto, "Africa"** (1982). The verse uses I → V → vi → IV. The chorus uses a different progression. The verse progression is the I-V-vi-IV we're writing here.

### Summary and checkpoint

**Key takeaways:**

- A chord is three or more notes played at the same time. A triad is the most basic chord.
- Major triads sound happy. Minor triads sound sad.
- The I-V-vi-IV progression is the most famous in pop music. In C major: C-G-Am-F.
- The PO-33 has only 4 voices. Manage them carefully.

**Self-check:**

- [ ] Can you build a C major triad (pads 1, 5, 8)?
- [ ] Can you write a I-V-vi-IV progression?
- [ ] Do you understand why the same progression sounds happy in C major and sad in C minor?
- [ ] Can you manage voices (skip the hi-hat if you have bass + chord + drums)?

If you answered "yes" to all four, you are ready for Part IV: groove and genre.

---


# Part IV — Groove and Genre

Part III gave you the building blocks of melody and harmony. Part IV puts them into **genre contexts** — the specific combinations of BPM, swing, drum pattern, bassline, and chord progression that define a style of music. Each chapter is a different genre, a different feel, and a different recipe. By the end of Part IV you will have four complete beats in four different genres.

---

## Chapter 11 — Hip-Hop at 80 BPM

### Objectives

By the end of this chapter you will be able to:

- Set the PO-33 to Hip Hop mode (80 BPM, swing 2).
- Write a boom-bap drum pattern.
- Layer a J Dilla-style syncopated bassline.
- Recognise boom-bap in popular music.

### What is hip-hop?

**Hip-hop** is a genre of popular music that originated in the Bronx in the 1970s. It is built on **samples** — short recordings of other songs, usually funk or soul, chopped and rearranged into a new beat. Hip-hop is the most sample-heavy genre in popular music; the Amen break alone has been sampled over 7,000 times.

Hewitt (*Music Theory for Computer Musicians*) characterises hip-hop rhythm as **syncopated** — the kick lands on the "and"s of beats, not on the beats themselves. The snare plays the backbeat (steps 5 and 13), but the kick plays **between** the snare hits, not on the beats.

The classic hip-hop tempo is **80–95 BPM**. The PO-33's Hip Hop preset is 80 BPM, which is perfect for boom-bap.

### The boom-bap drum pattern

The boom-bap drum pattern is the most influential hip-hop drum pattern. It is built from:

- **Kick on steps 1 and 11** (boom).
- **Snare on steps 5 and 13** (bap).
- **Hi-hat on every odd step** (1, 3, 5, 7, 9, 11, 13, 15).

The kick is **late** — it lands on step 11 instead of step 9. This is the syncopation that defines boom-bap.

To write boom-bap:

1. Set BPM to Hip Hop (80).
2. Set swing to 2.
3. Press WRITE, PLAY.
4. Step 1: pad 9 (kick).
5. Step 3: pad 11 (hi-hat).
6. Step 5: pad 10 (snare).
7. Step 7: pad 11 (hi-hat).
8. Step 9: pad 11 (hi-hat).
9. Step 11: pad 9 (kick) and pad 11.
11. Step 13: pad 10 (snare) and pad 11.
12. Step 15: pad 11.
13. Press WRITE to exit.

This is the boom-bap. You should hear the kick "BOOM" on step 1, then "bap" on step 5, then "boom" on step 11 (one beat after the snare on step 9 — the hi-hat continues), then "bap" on step 13.

### The J Dilla bassline

J Dilla's basslines are mostly the **root** of the chord, played **once per measure**. In C major, the bassline plays C on step 1 and C on step 9 — that's it.

Some of Dilla's beats have more elaborate basslines (with the fifth or octave), but the simplest and most iconic is **one note, twice per loop**.

To write a Dilla bassline:

1. Record a bass sound into slot 2.
2. Press WRITE, PLAY.
3. Step 1: pad 1 (C, root).
4. Step 9: pad 1 (C, root).
5. Press WRITE to exit.

The bassline plays C twice. Underneath the boom-bap beat, this is enough. Don't add more. Dilla didn't.

### The sample-based melody

Hip-hop melodies are usually **samples** of other songs, played back in a loop. To make a hip-hop melody on the PO-33:

1. Record a melodic sample (a vocal phrase, a guitar riff, a synth chord) into slot 1.
2. Press WRITE, PLAY.
3. On step 1: pad 1 (plays the sample at C4).
4. On step 9: pad 1 again (plays the sample again).
5. Press WRITE to exit.

The sample plays twice per loop, at the same pitch. This is "loop-based" music. Most hip-hop melodies are exactly this: a sample, looped.

> **Sidebar — clearing samples.** For copyright reasons, you should not sample commercial music without permission. Use samples you recorded yourself, or use a service like Tracklib that licenses samples for commercial use. See Appendix E.

### Putting it together: a complete boom-bap beat

You have:

- Kick on 1+11 (slot 9).
- Snare on 5+13 (slot 10).
- Hi-hat on every odd step (slot 11).
- Bass on 1+9 (slot 2, pad 1).
- Sample on 1+9 (slot 1, pad 1).

That's 5 voices playing simultaneously, but the PO-33 has only 4 voices of polyphony. **You have to choose 4.**

**The classic hip-hop voice allocation:**

- Kick (1 voice).
- Snare (1 voice).
- Hi-hat (1 voice).
- Bass (1 voice).

The sample is **not** playing while the bass and drums are. To hear the sample, you can either:

1. **Drop the hi-hat** and replace it with the sample.
2. **Use a separate pattern** for the sample. Pattern 1 = drums + bass. Pattern 2 = sample alone (no drums). Chain them together (Chapter 15).
3. **Use the PO-33's voice stealing** — the newest sound "steals" the voice from the oldest sound. This can work if the sample is short.

For now, let's go with option 1: drop the hi-hat. Replace it with the sample.

**The voice allocation for our boom-bap beat:**

- Kick (slot 9) on steps 1, 11.
- Snare (slot 10) on steps 5, 13.
- Bass (slot 2, pad 1) on steps 1, 9.
- Sample (slot 1, pad 1) on steps 1, 9.

The hi-hat is gone, but the kick-snare-bass-sample combo fills the same frequency range as a typical boom-bap record. The lack of hi-hat actually makes the beat feel **heavier** and **more spacious**.

To write this:

1. Set BPM to Hip Hop (80). Set swing to 2.
2. Erase the existing pattern (hold WRITE + erase).
3. Press WRITE, PLAY.
4. Step 1: pad 9 (kick), pad 1 (bass), pad 1 (sample — same pad, same step; this is two voices triggered by one tap).
5. Step 5: pad 10 (snare).
6. Step 9: pad 1 (bass), pad 1 (sample).
7. Step 11: pad 9 (kick).
8. Step 13: pad 10 (snare).
9. Press WRITE to exit.

You have just written a complete boom-bap beat. It has kick, snare, bass, and sample — the four voices of classic hip-hop.

### Guided exercise: your first complete hip-hop beat

**Success criteria:** a beat with kicks on 1+11, snares on 5+13, bass on 1+9, sample on 1+9. Plays at 80 BPM. Loops cleanly.

1. Record a kick into slot 9.
2. Record a snare into slot 10.
3. Record a bass into slot 2 (or use a sample from a sample pack).
4. Record a melodic sample into slot 1.
5. Set BPM to Hip Hop (80). Set swing to 2.
6. Write the pattern above.
7. Press PLAY. Listen for 1 minute.

The beat should sound "heavy" and "spacious" — that's the boom-bap feel.

### Creative challenge: the Dilla beat

Modify the boom-bap beat to be more Dilla-style:

- Move the kick from step 11 to step 12.
- Add a hi-hat on step 7 and step 12 (use slot 11).
- Increase swing to 3.
- Drop the sample from step 9 (keep it only on step 1).

You have just made the beat feel "half-time" — the kick is even later, and the sample plays only once per loop. This is the Dilla feel.

### Listening assignment

Listen to:

- **J Dilla, "Workinonit"** from *Donuts* (2006). The beat is exactly the boom-bap we just wrote, with extra swing and a Dilla-style hi-hat. Listen for the "thump" of the kick on step 11.
- **A Tribe Called Quest, "Can I Kick It?"** (1991). Classic boom-bap. The bassline is a sampled Lou Reed loop. The drums are a sampled break. Listen for the snare on steps 5 and 13.
- **Madlib, "Accordion"** from *Madvillainy* (2004). Almost no drums at all — just a kick on step 1, a snare on step 13, and the accordion sample playing in a loop. The "less is more" approach.
- **Travis Scott, "SICKO MODE"** (2018). Modern trap, descended from boom-bap. The kick is on 1+11 (same as boom-bap), but the snare is on 5+17 (off the grid — we don't have that on the PO-33, so use 5+13). Heavy swing.

### Summary and checkpoint

**Key takeaways:**

- Hip-hop is built on samples, syncopated drums, and a heavy kick.
- Boom-bap = kick on 1+11, snare on 5+13, hat every odd step.
- J Dilla basslines are simple — root, twice per loop.
- The PO-33 has 4 voices. Drop the hi-hat to fit bass + sample + drums.

**Self-check:**

- [ ] Can you write a boom-bap pattern at 80 BPM?
- [ ] Can you layer a sample on top of the drums and bass?
- [ ] Do you understand why Dilla's basslines are simple?
- [ ] Can you modify the boom-bap to be more Dilla-style?

If you answered "yes" to all four, move on to Chapter 12.

---

## Chapter 12 — House and Disco at 120 BPM

### Objectives

By the end of this chapter you will be able to:

- Set the PO-33 to Disco mode (120 BPM, swing 1).
- Write a four-on-the-floor drum pattern.
- Layer a disco-style bassline (root, every beat).
- Recognise house and disco drum patterns.

### What is house / disco?

**Disco** is a genre of dance music that emerged in the 1970s. It is built on **four-on-the-floor** drum patterns (kick on every beat), **disco string stabs** (orchestral hits), and **syncopated hi-hats**. Disco is the foundation of house music.

**House** is a genre of dance music that emerged in Chicago in the 1980s. It is essentially disco sped up slightly (120 BPM is typical) and stripped of the orchestral stabs, replaced with synthesised basslines and piano chords. House is the most popular genre of dance music in the world.

Hewitt (*Music Theory for Computer Musicians*) characterises house rhythm as **steady and metronomic** — the kick is on every beat, the hi-hat is on every off-beat. There is almost no syncopation in house. The music is **steady** so that dancers can **count** it.

> **Sidebar — 4/4 vs. 3/4.** Disco and house are in 4/4 time. Waltz is in 3/4 time. Most popular music (pop, rock, hip-hop, house, techno) is in 4/4. The PO-33's 16-step sequencer is a 4/4 sequencer — it doesn't have a 3/4 mode. To make a waltz, you'd have to use the 6/8 quantize effect (FX 5 on the PO-33), which is rarely used in modern music.

### The four-on-the-floor drum pattern

The four-on-the-floor is the most important drum pattern in dance music. It is built from:
- **Kick on every beat**: steps 1, 5, 9, 13.
- **Snare on beats 2 and 4**: steps 5, 13.
- **Hi-hat on every eighth-note**: steps 1, 3, 5, 7, 9, 11, 13, 15. Or every off-beat (3, 7, 11, 15) for a more open feel.

To write four-on-the-floor:

1. Set BPM to Disco (120).
2. Set swing to 1 (almost no swing).
3. Press WRITE, PLAY.
4. Step 1: pad 9 (kick), pad 11 (hi-hat).
5. Step 3: pad 11.
6. Step 5: pad 9 (kick), pad 10 (snare), pad 11.
7. Step 7: pad 11.
8. Step 9: pad 9 (kick), pad 11.
9. Step 11: pad 11.
10. Step 13: pad 9 (kick), pad 10 (snare), pad 11.
11. Step 15: pad 11.
12. Press WRITE to exit.

This is the four-on-the-floor. Notice how steady it is. The kick lands like a clock: step 1, 5, 9, 13, 1, 5, 9, 13, ... forever.

### The disco bassline

Disco basslines are usually **one note per beat** (root of the chord). In C major, the bassline plays C on every beat. The kick and the bass play together — a **four-on-the-floor with a bass kick**.

To write a disco bassline:

1. Record a bass sound into slot 2.
2. Press WRITE, PLAY.
3. Step 1: pad 1 (C, root).
4. Step 5: pad 1 (C, root).
5. Step 9: pad 1 (C, root).
6. Step 13: pad 1 (C, root).
7. Press WRITE to exit.

The bass plays four times per loop, on every beat. Combined with the kick, this gives the "thump-thump-thump-thump" feel of disco.

### A simple house chord progression

House music usually uses simple chord progressions: I-vi-IV-V or I-IV-V-I. In C major:

- I = C major (pads 1, 5, 8).
- vi = A minor (pads 10, 13, 1).
- IV = F major (pads 6, 10, 13).
- V = G major (pads 8, 12, 15).

To write a 4-chord progression in slot 3:

1. Record a sustained sound (a synth pad) into slot 3.
2. Press WRITE, PLAY.
3. Step 1: pads 1, 5, 8 (C major).
4. Step 5: pads 10, 13, 1 (A minor).
5. Step 9: pads 6, 10, 13 (F major).
6. Step 13: pads 8, 12, 15 (G major).
7. Press WRITE to exit.

This is I-vi-IV-V in C major — the "50s progression" (doo-wop), also the basis of countless house records.

### Voice management for house

A typical house track uses 4 voices:

- Kick (slot 9).
- Hi-hat (slot 11).
- Bass (slot 2).
- Chord (slot 3).

That's 4. **No snare?** Many house tracks have no snare — they just use hi-hat and kick. The hi-hat provides the backbeat (steps 3, 7, 11, 15 give the "tick tick" that dancers use to count).

If you want a snare, drop the chord (use only bass + kick + snare + hi-hat). The PO-33 cannot play kick + snare + hi-hat + bass + chord simultaneously. Choose.

For our exercise, let's drop the snare and use: kick + hi-hat + bass + chord.

### Guided exercise: a complete house loop

**Success criteria:** a beat with kick on 1+5+9+13, hi-hat every off-beat, bass on 1+5+9+13, chord progression I-vi-IV-V. Plays at 120 BPM.

1. Record kick (slot 9), hi-hat (slot 11), bass (slot 2, pad 1), chord pad (slot 3).
2. Set BPM to Disco (120). Set swing to 1.
3. Write the four-on-the-floor pattern, but remove the snare.
4. Write the disco bassline (pad 1 on 1, 5, 9, 13).
5. Write the chord progression (steps 1, 5, 9, 13).
6. Press PLAY. Listen for 1 minute.

You should hear a clear house groove. The kick and bass lock together. The chord progression cycles underneath. The hi-hat ticks off every off-beat.

### Creative challenge: the disco string stab

Add a "string stab" — a short orchestral hit — on steps 3 and 11. Record a "BRAMM" or "BWAAAM" sound into slot 4. On step 3 and step 11, trigger pad 5 (any pad — it will play the slot 4 sample). 

This is the classic disco string stab: a short orchestral hit on the off-beats. It gives the track the "disco" feel.

> **Sidebar — voice management, again.** Adding a string stab is a 5th voice. You need to drop something. Drop the chord progression, or drop the bass. A disco track with kick + snare + hi-hat + string stab (no bass, no chord) is also valid.

### Listening assignment

Listen to:

- **Donna Summer, "I Feel Love"** (1977). Pure four-on-the-floor. No snare. Just kick, hi-hat, and a synth bassline. This song changed dance music forever.
- **Daft Punk, "Around the World"** (1997). Four-on-the-floor. Single-note bassline. I-vi-IV-V chord progression (transposed). The textbook house song.
- **Frankie Knuckles, "Your Love"** (1987). The track that gave house music its name. Four-on-the-floor, sustained chords, simple bassline.
- **Disclosure, "Latch"** (2012). Modern house. Four-on-the-floor with a swung hi-hat. Notice how the swung hi-hat makes the beat feel more "human".

### Summary and checkpoint

**Key takeaways:**

- House and disco are 4/4 dance music at 120 BPM.
- Four-on-the-floor = kick on every beat.
- Disco basslines are simple: root, every beat.
- Many house tracks have no snare — just kick, hi-hat, bass, chord.

**Self-check:**

- [ ] Can you write a four-on-the-floor at 120 BPM?
- [ ] Can you layer a disco bassline (pad 1 on every beat)?
- [ ] Can you write a I-vi-IV-V progression?
- [ ] Do you understand why four-on-the-floor is "danceable"?

If you answered "yes" to all four, move on to Chapter 13.

---

## Chapter 13 — Techno and "Tencho" at 140 BPM

### Objectives

By the end of this chapter you will be able to:

- Set the PO-33 to Tencho mode (140 BPM).
- Write a techno drum pattern.
- Layer a synth bassline (often acid-style).
- Recognise techno drum patterns.

### What is techno?

**Techno** is a genre of electronic dance music that emerged in Detroit in the 1980s. It is the fastest of the three PO-33 BPMs (140 BPM is typical). It is built on **four-on-the-floor** drum patterns (same as house) but with **acid synths**, **robotic feel**, and **longer tracks**.

Hewitt (*Music Theory for Computer Musicians*) characterises techno as **repetitive and hypnotic** — the beat is the same for 4–8 minutes, with subtle changes (a filter opening, a snare roll, a hi-hat pattern shift) that build tension and release.

> **Sidebar — "Tencho".** The PO-33 calls its 140 BPM preset "Tencho" (with a typo, intentional). This is Teenage Engineering's joke — "techno" with a typo. Most techno is actually 130–150 BPM, so 140 is right in the middle.

### The techno drum pattern

Techno uses the same four-on-the-floor as house (kick on every beat), but with more **hi-hat activity** and **snare rolls**. A simple techno pattern:

- **Kick on every beat**: steps 1, 5, 9, 13.
- **Snare on steps 5 and 13** (or every 8th-note for "machine" techno).
- **Hi-hat on every eighth-note**: steps 1, 3, 5, 7, 9, 11, 13, 15. Or with skips: 1, 5, 9, 13 (every beat).
- **Open hi-hat on step 15**: a longer hi-hat sound that rings into the next bar.

To write a basic techno pattern:

1. Set BPM to Tencho (140). Set swing to 0 (no swing — techno is robotic).
2. Press WRITE, PLAY.
3. Step 1: pad 9 (kick), pad 11 (hi-hat).
4. Step 3: pad 11.
5. Step 5: pad 9, pad 10, pad 11.
6. Step 7: pad 11.
7. Step 9: pad 9, pad 11.
8. Step 11: pad 11.
9. Step 13: pad 9, pad 10, pad 11.
10. Step 15: pad 11 (closed hi-hat).
11. Press WRITE to exit.

To add an **open hi-hat** on step 15: record a longer hi-hat sound into slot 12 (a "tssssh" that rings for 0.5 seconds). Use pad 12 for step 15 instead of pad 11.

### The acid bassline

Techno's signature sound is the **acid bassline** — a squelchy, resonant synth bass that mimics the Roland TB-303. The PO-33 cannot generate a real 303, but you can fake one by:

1. Recording a short synth bass note into slot 2.
2. Pitch-shifting it down (Tweak Tone mode, Knob A down by 12 semitones).
3. Adding a low-pass filter (FX 1: low-pass filter). Hold FX + pad 1 to apply the filter.
4. Twisting Knob A while the note plays to "sweep" the filter.

For a sequence:

1. Step 1: pad 1 (C).
2. Step 3: pad 5 (E).
3. Step 5: pad 8 (G).
4. Step 7: pad 5 (E).
5. Step 9: pad 1 (C).
6. Step 11: pad 5 (E).
7. Step 13: pad 8 (G).
8. Step 15: pad 13 (C, octave up).

This is a "rolling" acid bassline that moves in fourths and fifths.

### Techno arrangement

A techno track is usually:

- 1 bar of intro (drums only).
- 4 bars of build (drums + filter sweep).
- 16 bars of full groove (drums + bass + synth).
- 4 bars of breakdown (drums only or filtered).
- 16 bars of peak (drums + bass + synth + filter open).
- 1 bar of outro (drums fade).

On the PO-33, you can't do filter sweeps dynamically while the pattern plays, but you can **chain patterns** (Chapter 15) to make a multi-section track.

### Voice management for techno

A typical techno track uses 4 voices:

- Kick (slot 9).
- Hi-hat (slot 11).
- Bass (slot 2).
- Synth chord or lead (slot 1 or slot 3).

If you want a snare, you have to drop one of these. Most techno tracks have snare on steps 5 and 13 — they are part of the drum pattern. So you'd be:

- Kick + snare + hi-hat = 3 voices.
- Bass + 1 voice = 4 voices.

That's the maximum. **No chord pad** in a techno track.

### Guided exercise: a complete techno loop

**Success criteria:** a beat with kick on 1+5+9+13, snare on 5+13, hi-hat every off-beat, acid bassline on every odd step. Plays at 140 BPM.

1. Record kick (slot 9), snare (slot 10), hi-hat (slot 11), bass (slot 2).
2. Set BPM to Tencho (140). Set swing to 0.
3. Write the techno drum pattern above.
4. Write the acid bassline (pads 1, 5, 8, 5, 1, 5, 8, 13 on steps 1, 3, 5, 7, 9, 11, 13, 15).
5. Press PLAY. Listen for 1 minute.

You should hear a clear techno groove. The bass moves. The kick is steady. The hi-hat ticks.

### Creative challenge: the filter sweep

Apply the low-pass filter (FX 1) to the bassline. Hold FX + pad 1 while the bass plays. While the filter is engaged, turn Knob A from 0 to 255 over 4 bars (16 cycles of the pattern). This is the classic "filter sweep" — the bass starts muffled and opens up to full brightness.

To do this on the PO-33 with chained patterns (Chapter 15), write the same pattern 4 times, with the filter cutoff increasing each time. Pattern 1: filter 0. Pattern 2: filter 64. Pattern 3: filter 128. Pattern 4: filter 192. Chain them.

> **Sidebar — FX in Tweak mode.** The PO-33 has 16 punch-in FX. FX 1 (low-pass filter) is the most important for techno. FX 16 (swing) is global. FX 2 (reverb) is good for ambient tracks. The full list is in Appendix B.

### Listening assignment

Listen to:

- **Rhythm Is Rhythm, "Strings of Life"** (1987). Detroit techno. Four-on-the-floor, acid bassline, string sample. This is the "Tencho" preset at 140 BPM.
- **Plastikman, "Spastik"** (1993). Minimal techno. The bassline is a single squelchy note. The drums are pure four-on-the-floor. This is what an acid techno track sounds like.
- **Jeff Mills, "The Bells"** (1997). The bassline is a single tone. The hi-hat is a clap. The kick is steady. This is the essence of techno.
- **Bicep, "Glue"** (2017). Modern techno. Notice how the kick and bass are perfectly locked. The hi-hat has subtle swing. This is the "humanised techno" approach.

### Summary and checkpoint

**Key takeaways:**

- Techno is 4/4 dance music at 140 BPM.
- Same drum pattern as house (four-on-the-floor) but faster and more robotic.
- Acid basslines move in fourths and fifths; filter sweeps are essential.
- Most techno tracks use exactly 4 voices.

**Self-check:**

- [ ] Can you write a four-on-the-floor at 140 BPM?
- [ ] Can you write a rolling acid bassline?
- [ ] Can you apply the low-pass filter to the bass?
- [ ] Do you understand why techno is "hypnotic"?

If you answered "yes" to all four, move on to Chapter 14.

---

## Chapter 14 — Lo-Fi, Experimental, and the Sound of Constraint

### Objectives

By the end of this chapter you will be able to:

- Set the PO-33 to a "lo-fi" tempo (70–90 BPM).
- Use the PO-33's effects to create lo-fi textures.
- Write an experimental beat using unusual sounds.
- Recognise lo-fi and experimental music.

### What is lo-fi?

**Lo-fi** (short for **low-fidelity**) is a genre that deliberately uses low-quality audio — tape hiss, vinyl crackle, bitcrushed drums, distorted samples. The aesthetic is "warm", "imperfect", and "nostalgic". Lo-fi hip-hop (the "lo-fi beats to study to" genre) is built on:

- Slow tempos (70–85 BPM).
- Jazz or soul samples.
- Heavily filtered, pitched, and degraded audio.
- Repetitive, hypnotic drum patterns.

The PO-33 is, in some ways, the ultimate lo-fi instrument. Its sample memory is small, its effects are crunchy, and its polyphony is limited. Embrace the constraints.

> **Sidebar — bitcrushing.** The PO-33 has a **bitcrush** effect (FX 14). Bitcrushing reduces the bit depth of a sample, making it sound "8-bit" or "videogame". This is the classic lo-fi effect.

### A lo-fi hip-hop beat

To build a lo-fi beat:

1. Set BPM to Hip Hop (80). Set swing to 3.
3. Record a jazz sample into slot 1 (a vinyl crackle, a piano loop, a vocal phrase).
4. Record a soft kick into slot 9 (a "thud" rather than a "boom").
5. Record a snare with reverb into slot 10 (use FX 2: reverb).
7. Record a closed hi-hat into slot 11.
8. Press WRITE, PLAY.
9. Step 1: pad 9 (kick), pad 11 (hat), pad 1 (sample at C4).
10. Step 5: pad 10 (snare), pad 11.
11. Step 9: pad 11 (hat).
12. Step 11: pad 9 (kick), pad 11.
13. Step 13: pad 10 (snare), pad 11.
14. Step 15: pad 11.
15. Press WRITE to exit.

Apply bitcrush (FX 14) to the sample (slot 1). Hold FX + pad 14. The sample should now sound "8-bit".

This is a lo-fi beat. The drums are sparse. The sample is degraded. The swing is heavy.

### Experimental music on the PO-33

**Experimental** music on the PO-33 means using the device in unintended ways. Some ideas:

- **Record a 30-second example**: pad the audio, push the device closer to a speaker to capture hum, sample your breathing.
- **Pitch shift everything down**: in Tweak Tone mode, shift the melodic slots down by 12–24 semitones. The result sounds "deep" and "ambient".
- **Apply reverse (FX 9)**: hold FX + pad 9 to reverse a sample. The result sounds "spooky".
- **Use the PO-33 as a guitar pedal**: route a guitar through the line-in, apply effects, record the output.
- **Chain patterns with parameter locks** (Chapter 16): each pattern has a different filter cutoff, creating an evolving texture.
- **Randomise everything**: write a beat by rolling dice and assigning pads to steps. Embrace chaos.

> **Sidebar — the aesthetics of constraint.** Brian Eno, the ambient music producer, has said "the studio is an instrument". The PO-33, with its 40 seconds of memory, 16 slots, 16 steps, and 16 effects, is a **studio**. The constraints force creativity. Don't see them as obstacles.

### The PO-33 as a sketchpad

The PO-33 is most useful as a **sketchpad** — a device to capture a 4-bar idea, then move it to a computer DAW for full production. But the PO-33 can also be a finished instrument, especially for lo-fi and experimental music where the constraints **are** the aesthetic.

Some famous lo-fi and experimental artists who embrace the PO-33's constraints:

- **J Dilla** (1974–2006): worked with the MPC, but his philosophy — "less is more, swing is everything" — applies perfectly to the PO-33.
- **Burial** (1990–): UK garage / dubstep producer known for chopped vocal samples and crackling vinyl textures. The PO-33 is his spiritual instrument.
- **Madlib** (1973–): hip-hop producer who builds entire beats from one sample. The PO-33 has the same ethos.
- **Oneohtrix Point Never** (1980–): experimental electronic artist who builds tracks from samples and effects. The PO-33's bitcrush is his bread and butter.

### Guided exercise: a lo-fi beat from scratch

**Success criteria:** a beat with a jazz sample, soft drums, heavy swing, and a bitcrushed texture. Plays at 80 BPM.

1. Find or record a jazz sample (a piano loop, a vocal phrase, a sax note). Put it in slot 1.
2. Record a soft kick, snare, and hi-hat. Put them in slots 9, 10, 11.
3. Set BPM to 80. Set swing to 3.
4. Apply bitcrush (FX 14) to slot 1. Hold FX + pad 14.
5. Write a pattern: sample on steps 1, 9; kick on steps 1, 11; snare on 5, 13; hat every odd step.
6. Press PLAY. Listen.

If it sounds too clean, increase the swing to 5. If it sounds too quiet, increase the velocity (Tweak Volume mode, Knob B).

### Creative challenge: the field recording

Record a 10-second field recording — your street, your kitchen, a coffee shop — into slot 1. Use it as the "sample" in your beat. Apply FX 9 (reverse) to half the loop, FX 1 (low-pass) to the other half. You have just made a **hauntological** beat — a beat that sounds like a memory.

The genre is "hauntology" (named by music critic Mark Fisher). It is built on degraded samples of the past. The PO-33 is the perfect instrument for it.

### Listening assignment

Listen to:

- **Burial, "Archangel"** (2007). A chopped vocal sample ("Tell me I belong") over a slow garage beat. Heavy pitch-shifting. This is the PO-33's native genre.
- **Madlib, "Movie Fight"** from *Madvillainy* (2004). Two samples layered: a horn stab and a vocal phrase. Minimal drums. The "less is more" approach.
- **J Dilla, "Don't Cry"** from *Donuts* (2006). A jazz sample (the Pretty Things' "Don't Bring Me Down") pitched up, chopped, and looped. The Dilla aesthetic.
- **Boards of Canada, "Olson"** (2002). A warped, degraded sample with heavy filter sweep. The PO-33's filter (FX 1) can do this.

### Summary and checkpoint

**Key takeaways:**

- Lo-fi music embraces low-quality audio as an aesthetic.
- The PO-33's bitcrush (FX 14) is the lo-fi effect.
- Experimental music uses the PO-33 in unintended ways: pitch-shifting, reversing, randomising.
- The PO-33 is a "sketchpad" — capture an idea in 4 bars, then move to a DAW.

**Self-check:**

- [ ] Can you build a lo-fi beat with bitcrush and heavy swing?
- [ ] Can you apply FX 9 (reverse) to a sample?
- [ ] Can you write an experimental beat using unusual sounds?
- [ ] Do you understand why lo-fi and constraint go together?

If you answered "yes" to all four, you are ready for Part V: full tracks and performance.

---


# Part V — Full Tracks and Performance

By the end of Part IV you have four complete beats in four genres. Each beat is one bar (16 steps) long and loops forever. That's a **loop**, not a **song**. A song has structure: intro, verse, chorus, bridge, outro. Part V teaches you to extend your loops into songs using **pattern chaining** (multiple loops linked together), **parameter locks** (different settings on each step), and **live performance** techniques.

---

## Chapter 15 — Song Structure: Intro, Verse, Chorus, Bridge

### Objectives

By the end of this chapter you will be able to:

- Explain the four-part song structure: intro, verse, chorus, bridge.
- Write four patterns that progress through these sections.
- Chain the patterns into a 4-section song.
- Recognise song structure in popular music.

### Musical concept: what is a song?

A **song** is a piece of music with structure. It has a beginning, a middle, and an end. Most popular songs have a structure like this:

- **Intro**: 4–16 bars. Introduces the beat, builds tension.
- **Verse**: 8–16 bars. Tells the story. Lower energy than the chorus.
- **Chorus**: 8–16 bars. The "hook". Higher energy, melody returns.
- **Bridge**: 4–8 bars. A contrast — different chord progression, different energy.
- **Outro**: 4–16 bars. Returns to the verse theme, ends.

Allen (*Music Theory for Electronic Music Producers*) calls this the **dance-music song structure**: intro → breakdown → build → peak → breakdown → peak → outro. The breakdown is the verse. The peak is the chorus. The build is the bridge. Electronic dance music (EDM) tends to use longer sections than pop music, but the principle is the same.

Hewitt (*Music Theory for Computer Musicians*) describes the **classical song form** as ABAB or AABA: section A (verse), section B (chorus), section A again, section B again. The PO-33's pattern chaining gives you 16 patterns (A, B, C, D, ...), which is enough to build an AABA form.

> **Sidebar — pattern capacity.** The PO-33 has **16 patterns** in memory. Each pattern is one bar (16 steps). So you can chain up to 16 bars in a song, or 16 different one-bar loops that you can arrange in any order. Real PO-33 songs are usually 16–64 bars long. RavinePhoenix firmware supports up to 16 patterns per project.

### The four patterns of a song

Let's build a four-pattern song:

- **Pattern 1: Intro** (drums only, slow build).
- **Pattern 2: Verse** (drums + bass + sample, medium energy).
- **Pattern 3: Chorus** (drums + bass + sample + chord, full energy).
- **Pattern 4: Bridge** (drums + sample only, contrast).

### Writing the four patterns

**Pattern 1: Intro** — kick only on step 1.

1. Erase the pattern.
2. Press WRITE, PLAY.
3. Step 1: pad 9 (kick).
4. Press WRITE to exit.

**Pattern 2: Verse** — kick + snare + bass + sample.

1. Erase the pattern.
2. Press WRITE, PLAY.
3. Step 1: pad 9 (kick), pad 1 (bass), pad 1 (sample).
4. Step 5: pad 10 (snare).
5. Step 9: pad 1 (bass), pad 1 (sample).
6. Step 11: pad 9 (kick).
7. Step 13: pad 10 (snare).
8. Press WRITE to exit.

**Pattern 3: Chorus** — kick + snare + bass + sample + chord.

1. Erase the pattern.
2. Press WRITE, PLAY.
3. Step 1: pad 9 (kick), pad 1 (bass), pad 1 (sample), pad 1 (chord root).
4. Step 5: pad 10 (snare), pad 1 (chord — same note, sustained).
5. Step 9: pad 1 (bass), pad 1 (sample), pad 1 (chord).
6. Step 11: pad 9 (kick).
7. Step 13: pad 10 (snare), pad 1 (chord).
8. Press WRITE to exit.

Note: we used pad 1 for the bass, sample, and chord. That's because they all play the same note (C, the root). The PO-33's voice-stealing algorithm prioritises the most recent triggers. If the chord sustains, it will mask the bass and sample. This is fine — we want the chord to dominate the chorus.

**Pattern 4: Bridge** — kick + sample only.

1. Erase the pattern.
2. Press WRITE, PLAY.
3. Step 1: pad 9 (kick), pad 1 (sample).
4. Step 9: pad 1 (sample).
5. Press WRITE to exit.

### Chaining the patterns

To chain the patterns on the PO-33:

1. Hold **PATTERN** (the PO-33's "⠛" key).
2. Press pad **1** (select pattern 1, append to chain).
3. Press pad **2** (select pattern 2, append).
4. Press pad **3** (select pattern 3, append).
5. Press pad **4** (select pattern 4, append).
6. Release PATTERN.

The chain now plays pattern 1 → 2 → 3 → 4 → 1 → 2 → 3 → 4 → ... forever.

To stop the chain, press the **STOP** button (a square symbol).

> **Sidebar — chain length.** The PO-33's chain can hold up to 16 patterns. Our chain has 4. We could add more (e.g., 8 verses, 4 choruses) by pressing the same pattern number multiple times.

### Live transitions

The PO-33 lets you **jump** to a different pattern while the chain is playing. This is how you transition between sections live:

- During the verse (pattern 2), hold PATTERN + press pad 3 to jump to the chorus (pattern 3).
- During the chorus (pattern 3), hold PATTERN + press pad 1 to jump back to the intro (pattern 1).

This is "live arrangement" — you decide the song structure in real time.

### Guided exercise: a four-section song

**Success criteria:** a song that cycles through intro → verse → chorus → bridge → intro → ...

1. Write four patterns as described above.
2. Hold PATTERN, press pads 1, 2, 3, 4 in sequence. Release PATTERN.
3. Press PLAY. Listen for 1 minute.
4. The song should cycle through the four sections every 4 bars (about 8 seconds at 120 BPM).

### Creative challenge: a song with dynamics

Make the chorus louder than the verse. In Tweak Volume mode (FX + triple-tap to TRIM, then triple-tap again to VOLUME — actually, on the PO-33, Volume mode is reached by triple-tapping FX, but our firmware doesn't have VOLUME mode; instead, you adjust velocity per step).

To adjust velocity on the PO-33:

1. Enter WRITE mode.
2. Press PLAY.
3. Wait for the step you want to adjust. Hold the pad (e.g., pad 1) and **twist a knob** (Knob A or B). The screen shows the velocity (0–127).

For the chorus pattern, set the kick velocity to 120 (loud). For the verse pattern, set it to 80 (quiet). The chorus will feel "bigger".

### Listening assignment

Listen to:

- **Daft Punk, "One More Time"** (2000). The intro is 16 bars of just the vocal sample ("One more time"). Then the verse enters with the beat. Then the chorus with the full arrangement. Classic EDM song structure.
- **The Beatles, "A Hard Day's Night"** (1964). Intro (chord) → verse (vocals + drums + bass) → chorus (vocals + drums + bass + guitar) → verse → chorus → bridge (different chords) → chorus. Textbook AABA form.
- **Kraftwerk, "Trans-Europe Express"** (1977). The song structure is incredibly repetitive — the same 16-bar loop for 6 minutes. But it works because the loop is so hypnotic.
- **Burial, "Archangel"** (2007). The song structure is a single 4-bar loop that repeats for the entire track. There is no "verse" or "chorus" — just a loop. This is a valid song structure for electronic music.

### Summary and checkpoint

**Key takeaways:**

- A song has structure: intro, verse, chorus, bridge, outro.
- The PO-33 has 16 patterns. Chain them with PATTERN + pad.
- Live transitions: hold PATTERN + jump to a different pad.
- Dynamics: adjust velocity per step (Knob A in WRITE mode).

**Self-check:**

- [ ] Can you write four patterns (intro, verse, chorus, bridge)?
- [ ] Can you chain them with PATTERN + pad?
- [ ] Can you adjust the velocity of a step?
- [ ] Do you understand why "less is more" applies to song structure too?

If you answered "yes" to all four, move on to Chapter 16.

---

## Chapter 16 — Pattern Chaining, Parameter Locks, and Variation

### Objectives

By the end of this chapter you will be able to:

- Explain what a **parameter lock** (plock) is and why it matters.
- Set a parameter lock on a single step.
- Use parameter locks to vary patterns without writing new ones.
- Recognise parameter locks in popular music.

### What is a parameter lock?

A **parameter lock** (often abbreviated **plock**) is a setting that overrides the default value of a parameter on a **single step**. For example:

- Step 1: kick at velocity 100. Step 5: kick at velocity 120 (the plock). Step 9: kick at velocity 80.
- Step 1: snare with no FX. Step 9: snare with FX 14 (bitcrush).

Parameter locks are how producers add variation to a beat without rewriting the whole pattern. J Dilla's beats are full of them — a slightly louder kick here, a slightly different snare there.

> **Sidebar — Allen on parameter locks.** Allen (*Music Theory for Electronic Music Producers*) calls parameter locks the "secret weapon" of the piano roll. They allow you to introduce micro-variations that keep a beat interesting over time. Without them, a 4-bar loop sounds robotic. With them, it sounds alive.

### How to set a parameter lock on the PO-33

The PO-33 supports parameter locks via **Knob A** and **Knob B** in WRITE mode:

1. Press WRITE, PLAY.
2. Wait for the step you want to lock.
3. Hold the pad for that slot (e.g., pad 9 for kick).
4. While holding, turn **Knob A** (sets velocity, 0–127).
5. While holding, turn **Knob B** (sets pitch, ±12 semitones).
6. Release the pad.

The plock is now set. When the pattern plays, that step will use the locked values.

> **Sidebar — firmware support.** The RavinePhoenix firmware implements parameter locks for velocity and pitch. The real PO-33 also supports FX plocks (apply an FX to one step). We will add FX plocks in a future firmware version.

### An example: a velocity-ramped kick

A classic drum programming technique is the **velocity ramp** — the kick gets louder as the beat progresses, then drops back. To ramp the kick from velocity 80 to 120 over 8 steps:

1. Step 1: kick velocity 80.
2. Step 5: kick velocity 100.
3. Step 9: kick velocity 120.
4. Step 13: kick velocity 100.

The kick swells through the loop. This is a parameter lock on every step.

### An example: a pitch-shifted snare

A classic dub technique is to **pitch the snare down** by a few semitones on certain steps. To pitch-shift the snare down 3 semitones on step 13:

1. Press WRITE, PLAY.
2. Wait for step 13.
3. Hold pad 10 (snare).
4. Turn Knob B down by 3 (the screen shows -3 semitones).
5. Release pad 10.

The snare on step 13 will play 3 semitones lower than the snare on step 5. The result is a "wobble" — the snare sounds like it's bouncing.

### Using parameter locks to vary patterns

Parameter locks let you create variation without writing new patterns. Consider this trick:

- Pattern 1: kick on steps 1, 5, 9, 13 at velocity 100.
- Pattern 2: same kick pattern, but velocity 120 on step 5 and velocity 80 on step 13 (via plocks).

Patterns 1 and 2 sound almost identical, but the second has subtle dynamic variation. Chain them together and the kick will swell and dip across the chain.

### Guided exercise: a kick with parameter locks

**Success criteria:** a kick pattern with velocity 80 on step 1, velocity 100 on step 5, velocity 120 on step 9, velocity 100 on step 13.

1. Erase the pattern.
2. Press WRITE, PLAY.
3. Step 1: hold pad 9. Turn Knob A until screen shows "80". Release pad 9.
4. Step 5: hold pad 9. Turn Knob A until screen shows "100". Release pad 9.
5. Step 9: hold pad 9. Turn Knob A until screen shows "120". Release pad 9.
6. Step 13: hold pad 9. Turn Knob A until screen shows "100". Release pad 9.
7. Press WRITE to exit.

Play the pattern. The kick should swell across the loop. This is a "humanised" kick — it sounds like a real drummer hitting harder on the downbeats.

### Creative challenge: a pitched snare roll

Pitch-shift the snare down 2 semitones on every odd step (1, 3, 5, 7, ...). The result is a "wobble snare" that sounds like the snare is bouncing.

1. Press WRITE, PLAY.
2. Step 1: hold pad 10. Turn Knob B down by 2. Release.
3. Step 5: hold pad 10. Turn Knob B down by 2. Release.
4. Step 9: hold pad 10. Turn Knob B down by 2. Release.
5. Step 13: hold pad 10. Turn Knob B down by 2. Release.

Now play the beat. It should sound "underwater" — the snare is pitched below the rest of the drums.

### Listening assignment

Listen to:

- **J Dilla, "Workinonit"** (2006). The kick has subtle velocity variation on every step. Listen for the "swell" of the kick — it's slightly louder on some beats, quieter on others. This is parameter locks.
- **Aphex Twin, "Windowlicker"** (1999). The drums have pitch-shifted elements — the snare drops and rises across the track. Listen for the "wobble" on certain snare hits.
- **Burial, "Archangel"** (2007). The vocal sample has velocity variation on every step. Some syllables are louder, some quieter. This is what makes the vocal feel "human".
- **Autechre, "Gantz Graf"** (2002). Extreme parameter locks. Every step has a different setting. The result is "robotic" and "human" at the same time.

### Summary and checkpoint

**Key takeaways:**

- A parameter lock overrides the default value of a parameter on a single step.
- The PO-33 supports velocity and pitch plocks.
- Plocks add variation to a beat without rewriting the pattern.
- Plocks are how producers make beats sound "human".

**Self-check:**

- [ ] Can you set a velocity plock on a kick?
- [ ] Can you set a pitch plock on a snare?
- [ ] Can you create a velocity-ramped kick?
- [ ] Do you understand why J Dilla's beats feel "alive"?

If you answered "yes" to all four, move on to Chapter 17.

---

## Chapter 17 — Live Performance and Recording Your Music

### Objectives

By the end of this chapter you will be able to:

- Perform a live arrangement on the PO-33.
- Record your live performance to a computer.
- Mix your PO-33 track with a DAW.
- Share your music with the world.

### What is a live performance on the PO-33?

The PO-33 is not a multitrack DAW. It is a **live instrument**. You don't arrange it on a timeline; you play it. The patterns chain automatically, but the **transitions** between sections are under your control. A good PO-33 performance is a series of:

1. **Pattern selections** (jump to a different pattern).
2. **Knob tweaks** (adjust filter cutoff, volume, swing).
3. **FX punches** (apply an effect to the next note).
4. **Sample triggers** (play a sample that's not in the pattern).

The PO-33 is essentially a groovebox that you "play" like an instrument. Each performance is different, because your timing, your transitions, and your knob tweaks are unique.

> **Sidebar — groovebox vs. sequencer.** A **sequencer** plays a fixed sequence. A **groovebox** plays a fixed sequence but lets you modify it in real time. The PO-33 is somewhere between — it's a sequencer with groovebox-style real-time controls.

### A live arrangement exercise

Let's perform a 4-minute live set with the patterns we wrote in Chapter 15:

- **0:00–0:30**: Pattern 1 (intro). Slowly turn Knob A from 0 to 255 to open the filter on the sample.
- **0:30–1:00**: Jump to pattern 2 (verse). Apply FX 1 (low-pass filter) to the bass.
- **1:00–2:00**: Jump to pattern 3 (chorus). Remove the filter (twist Knob A back to 0).
- **2:00–2:30**: Jump to pattern 4 (bridge). Apply FX 14 (bitcrush) to the sample.
- **2:30–3:30**: Jump to pattern 3 (chorus). Remove bitcrush.
- **3:30–4:00**: Jump to pattern 1 (intro). Fade out by turning the master volume down (Tweak Volume mode, Knob B).

This is a 4-minute arrangement with 5 transitions. Each transition is a different feel — filter opening, FX applied, FX removed, FX reapplied, fade out.

To jump between patterns, hold PATTERN + the new pad number. The chain follows.

### Recording your performance

The PO-33 has a **line-out** jack. Plug it into a computer's audio input (or a USB audio interface) and record the output to your DAW.

**Recommended DAWs for PO-33 recordings:**

- **Audacity** (free, simple).
- **GarageBand** (Mac, free).
- **Ableton Live** (industry standard, paid).
- **Reaper** (cross-platform, paid but cheap).

The PO-33 outputs mono audio. The recording will be a single mono track.

### Mixing your PO-33 track

A raw PO-33 recording sounds "thin". To make it sound "professional", mix it in a DAW:

1. **EQ**: cut the lows below 30 Hz (PO-33 rumble), boost the highs at 8 kHz (adds "air").
2. **Compression**: a 2:1 ratio with a slow attack. This evens out the loud and quiet parts.
3. **Reverb**: a small room reverb (5–15% wet). Adds space.
4. **Saturation**: a subtle tape saturation. Adds "warmth".

A "PO-33 mix" preset in your DAW will give you a starting point.

### The PO-33 as a sketchpad

The PO-33 is most useful as a **sketchpad** — capture a 4-bar idea, then move it to a DAW for full production. To move an idea to a DAW:

1. **Sample the PO-33's audio output** into the DAW (record the line-out).
3. **Slice the audio** in the DAW (use Ableton's Simpler, Logic's Quick Sampler, etc.).
5. **Replay the slices** on a MIDI keyboard or pad controller.
7. **Arrange the slices** into a full track in the DAW.

This is how professional producers work. The PO-33 is the **first step** — the sketchpad. The DAW is the **second step** — the full production.

### The PO-33 as a finished instrument

For lo-fi and experimental music, the PO-33 is the **finished instrument**. Don't move the audio to a DAW. Record the PO-33 directly, mix it minimally, and release it. The "raw" sound of the PO-33 is part of the aesthetic.

Artists who release PO-33 tracks as finished music:

- **Burial**: every track is "sketchpad quality", and that's the appeal.
- **J Dilla**: he released the beat tapes *Donuts* and *The Beat Generation* as raw beats. No DAW.
- **Madlib**: his *Beat Konducta* series is a collection of raw beats.
- **Oneohtrix Point Never**: some of his tracks are built from single samples.

If your beat sounds good on the PO-33, it's good enough.

### Guided exercise: a 4-minute live set

**Success criteria:** a 4-minute live performance with at least 5 transitions.

1. Write 4 patterns (intro, verse, chorus, bridge).
2. Chain them: PATTERN + pads 1, 2, 3, 4.
3. Press PLAY.
4. At 0:30, jump to pattern 2 (PATTERN + pad 2).
5. At 1:00, jump to pattern 3 (PATTERN + pad 3).
6. At 2:00, jump to pattern 4 (PATTERN + pad 4).
7. At 2:30, jump to pattern 3 (PATTERN + pad 3).
8. At 3:30, jump to pattern 1 (PATTERN + pad 1).
9. Press STOP at 4:00.

You have just performed a 4-minute live set. The transitions are abrupt — that's the PO-33's aesthetic. Embrace it.

### Creative challenge: record and release

Record your 4-minute performance to your DAW. Apply the mixing chain (EQ, compression, reverb, saturation). Upload the result to SoundCloud, Bandcamp, or your favourite platform. Share it with a friend. You are now a music producer.

### Listening assignment

Listen to:

- **J Dilla, *Donuts*** (2006). A 40-minute album of raw beats, played on the MPC and mixed minimally. The PO-33 is the spiritual descendant of the MPC.
- **Madlib, *Beat Konducta in India*** (2007). A full album of beats recorded in India with local musicians. The PO-33 can do this — record local musicians, chop their samples, build beats.
- **Burial, *Untrue*** (2007). An album built from chopped R&B vocal samples and pitched garage beats. The PO-33 is Burial's instrument.
- **Autechre, *Amber*** (1994). An album of pure electronic music with no human "performance" — just sequences. The PO-33's chained patterns can do this.

### Summary and checkpoint

**Key takeaways:**

- The PO-33 is a live instrument. Perform it like a groovebox.
- Live arrangements use pattern jumps, knob tweaks, FX punches, and sample triggers.
- Record the PO-33's line-out to a DAW for mixing.
- The PO-33 can be a finished instrument (lo-fi) or a sketchpad (full production).

**Self-check:**

- [ ] Can you perform a 4-minute live set with pattern jumps?
- [ ] Can you record the PO-33's line-out to a DAW?
- [ ] Can you mix a PO-33 recording in a DAW?
- [ ] Do you understand the difference between "sketchpad" and "finished instrument"?

If you answered "yes" to all four, you have completed the book. Congratulations.

---

# Appendices

---

## Appendix A — Glossary of Musical Terms

This glossary defines every musical term used in the book. Terms are listed in alphabetical order.

**Accent**: emphasis on a note, usually achieved by making it louder.

**Amen break**: a 7-second drum break from The Winstons' "Color Him Father" (1969). The most sampled loop in music history, used in over 7,000 songs.

**Attack**: the beginning of a sound. A snare has a sharp attack (immediate loud sound). A pad has a slow attack (gradual onset).

**Bar**: a group of beats. In 4/4 time, one bar = 4 beats = 16 sixteenth-notes = 16 PO-33 steps.

**Bass drum**: see **kick**.

**Bassline**: a low-pitched melody that provides harmonic foundation.

**Beat**: the regular pulse of music. "On the beat" means aligned with the pulse. "Off the beat" means between pulses.

**Bitcrush**: an audio effect that reduces the bit depth of a sample, creating a "videogame" or "lo-fi" sound.

**BPM (beats per minute)**: the tempo. 120 BPM = 120 beats per minute = 0.5 seconds per beat.

**Break**: a short drum loop, usually 2–4 seconds, often sampled from a funk or soul record.

**Bridge**: a contrasting section in a song. Often features different chords, different energy.

**Chord**: three or more notes played simultaneously.

**Chorus**: the "hook" of a song. The section that returns and is most memorable.

**Chromatic scale**: the 12 semitones in an octave (C, C#, D, D#, E, F, F#, G, G#, A, A#, B).

**Consonance**: the quality of notes that sound "stable" together. Major thirds, perfect fifths, octaves.

**Cymbal**: a drum with a metallic sound. The hi-hat is a cymbal.

**DAW (digital audio workstation)**: software for recording and mixing audio (Ableton, Logic, FL Studio).

**Decay**: the gradual decrease in volume after the attack of a sound.

**Dissonance**: the quality of notes that sound "tense" together. Tritones, minor seconds.

**Drum machine**: an electronic device that plays drum sounds. The PO-33 is a sampler/drum hybrid.

**Dynamics**: variations in loudness and intensity. Loud and quiet sections.

**Envelope**: the shape of a sound over time (attack, decay, sustain, release).

**EQ (equalisation)**: adjusting the volume of specific frequencies.

**FX (effects)**: modifications applied to a sound (reverb, delay, filter, bitcrush).

**Filter**: an effect that removes certain frequencies. Low-pass = removes highs. High-pass = removes lows.

**Frequency**: cycles per second, measured in Hertz (Hz). Middle C = 261.63 Hz.

**Fundamental**: the lowest frequency of a sound. Other frequencies are overtones.

**Groove**: the rhythmic "feel" of a beat. A "groovy" beat has swing, syncopation, and variation.

**Half-time**: a feel where the snare hits every 2 beats instead of every beat.

**Harmony**: the combination of notes to form chords.

**Harmonics**: see **overtones**.

**Hertz (Hz)**: cycles per second. The unit of frequency.

**Hi-hat**: a cymbal pair played with a foot pedal. Closed hi-hat = short "tss". Open hi-hat = longer "tssssh".

**Hook**: a memorable melody, often the chorus.

**House**: a genre of dance music at 120 BPM, four-on-the-floor.

**Intro**: the first section. Builds tension before the verse.

**Interval**: the distance between two pitches.

**Kick (bass drum)**: a low-pitched drum with a sharp attack. The heartbeat of a beat.

**Lo-fi**: short for "low-fidelity". Music that embraces low-quality audio as an aesthetic.

**Loop**: a sequence that repeats.

**Major scale**: a 7-note scale that sounds "happy". C major = C, D, E, F, G, A, B.

**Melody**: a sequence of notes played one after another.

**Meter**: the grouping of beats into bars. 4/4 = 4 beats per bar.

**Minor scale**: a 7-note scale that sounds "sad". C minor = C, D, D#, F, G, G#, A#.

**Mixing**: balancing the volume and EQ of multiple tracks.

**MIDI note**: a numerical representation of a pitch. Middle C = 60.

**Mono**: single-channel audio. The PO-33 outputs mono.

**Note value**: the duration of a note. Whole note = 4 beats. Half note = 2 beats. Quarter note = 1 beat. Eighth note = 0.5 beats. Sixteenth note = 0.25 beats.

**Octave**: an interval where the second pitch is twice the frequency of the first. C4 and C5 are an octave apart.

**Outro**: the final section of a song. Returns to the verse theme, ends.

**Overtones**: frequencies above the fundamental. All sounds have two frequencies.

**Parameter lock (plock)**: a setting that overrides the default value on a single step.

**Pentatonic scale**: a 5-note scale that "always sounds good". C major pentatonic = C, D, E, G, A.

**Pitch**: how high or low a note sounds. Measured in Hz.

**Polyphony**: the number of sounds that can play simultaneously. The PO-33 has 4 voices of polyphony.

**Quantise**: snap to a grid. The PO-33's sequencer quantises notes to steps.

**Reverb**: an effect that simulates the sound of a room.

**Rhythm**: the pattern of beats and silences.

**Root**: the first note of a chord. The "home" note.

**Sampler**: a device that records sound and plays it back.

**Scale**: a selection of notes from the chromatic scale.

**Semitone**: the smallest interval in Western music. One semitone = 1/12 of an octave.

**Sequencer**: a device that plays a sequence of notes automatically.

**Snare**: a mid-pitched drum with a sharp attack. The backbeat of a beat.

**Step**: one sixteenth-note in the PO-33's sequencer. 16 steps per bar.

**Stereo**: two-channel audio (left and right). The PO-33 outputs mono.

**Swing**: a timing shift applied to even-numbered steps, making them play late.

**Syncopation**: placing notes off the beat.

**Synth (synthesiser)**: an electronic instrument that generates sounds.

**Tape saturation**: a subtle distortion that mimics tape. Adds "warmth".

**Tempo**: the speed of music, measured in BPM.

**Tone**: 1) the quality of a sound (bright, dark, warm). 2) In PO-33 Tweak mode, the pitch and volume of a sample.

**Transpose**: shift the pitch of a melody up or down.

**Tritone**: an interval of 6 semitones (e.g., C to F#). The most dissonant interval.

**Velocity**: how hard a note is played. Affects volume and brightness.

**Verse**: the main body of a song. Tells the story.

**Vibrato**: a slight pitch modulation. Adds expression.

**Volume**: the loudness of a sound.

**Walk (walking bassline)**: a bassline that plays a different note every beat, usually stepwise.

**Wave**: a repeating pattern of air pressure. Sound is a wave.

---

## Appendix B — PO-33 Button Cheat Sheet

This appendix is a one-page reference for every button combination on the PO-33 and the RavinePhoenix firmware.

### Buttons

| Button | Symbol | Function |
|---|---|---|
| REC | ★ (star) | Hold + pad to record into a slot. |
| WRITE | · (dot) | Enter / exit write mode. |
| PLAY | ▶ (triangle) | Start / stop the sequencer. |
| STOP | (square) | Stop playback. |
| BPM | BPM text | Tap to cycle presets. Hold + knob to fine-tune. |
| FX | FX text | Hold + pad to apply FX. Triple-tap to cycle Tweak modes. |
| SOUND | S | Hold + pad to select a slot (instead of triggering). |
| PATTERN | ⠛ (PO-33 pattern) | Hold + pad to select / chain a pattern. |
| ERASE | (backspace) | Hold + pad to erase that step. |

### Pads (1–16)

| Pad | Default | With SOUND held | With FX held |
|---|---|---|---|
| 1 | Step 1 / slot 1 | Slot 1 (melodic) | FX 1 |
| 2 | Step 2 / slot 2 | Slot 2 (melodic) | FX 2 |
| 3 | Step 3 / slot 3 | Slot 3 (melodic) | FX 3 |
| 4 | Step 4 / slot 4 | Slot 4 (melodic) | FX 4 |
| 5 | Step 5 / slot 5 | Slot 5 (melodic) | FX 5 |
| 6 | Step 6 / slot 6 | Slot 6 (melodic) | FX 6 |
| 7 | Step 7 / slot 7 | Slot 7 (melodic) | FX 7 |
| 8 | Step 8 / slot 8 | Slot 8 (melodic) | FX 8 |
| 9 | Step 9 / slot 9 | Slot 9 (drum) | FX 9 |
| 10 | Step 10 / slot 10 | Slot 10 (drum) | FX 10 |
| 11 | Step 11 / slot 11 | Slot 11 (drum) | FX 11 |
| 12 | Step 12 / slot 12 | Slot 12 (drum) | FX 12 |
| 13 | Step 13 / slot 13 | Slot 13 (drum) | FX 13 |
| 14 | Step 14 / slot 14 | Slot 14 (drum) | FX 14 |
| 15 | Step 15 / slot 15 | Slot 15 (drum) | FX 15 |
| 16 | Step 16 / slot 16 | Slot 16 (drum) | FX 16 (swing) |

### FX list (1–16)

| FX | Effect |
|---|---|
| 1 | Low-pass filter |
| 2 | Reverb |
| 3 | Delay |
| 4 | Chorus |
| 5 | 6/8 quantise |
| 6 | Loop 16 |
| 7 | Loop shorter |
| 8 | Loop short |
| 9 | Reverse |
| 10 | Octave down |
| 11 | Octave up |
| 12 | Unison |
| 13 | Filter sweep |
| 14 | Bitcrush |
| 15 | Scratch fast |
| 16 | Swing (global) |

### Tweak modes (FX triple-tap)

| Mode | Knob A | Knob B |
|---|---|---|
| TONE | Pitch (semitones) | Volume |
| FILTER | Cutoff (0–8000 Hz) | Resonance |
| TRIM | Start point | End point |

### BPM presets

| Preset | BPM |
|---|---|
| Hip Hop | 80 |
| Disco | 120 |
| Tencho | 140 |

Fine-tune: 60–240 BPM via Knob B.

### Swing

| Knob A position | Swing level | Feel |
|---|---|---|
| 0 | 0 | Straight |
| 1 | 1 | Slight shuffle |
| 2 | 2 | Hip-hop |
| 3 | 3 | Heavy hip-hop |
| 4 | 4 | House |
| 5 | 5 | Heavy house |
| 6 | 6 | Trap / juke |
| 7 | 7 | Extreme |

### Pad-to-note mapping (F-005)

| Pad | MIDI | Note |
|---|---|---|
| 1 | 60 | C4 (middle C) |
| 2 | 61 | C#4 |
| 3 | 62 | D4 |
| 4 | 63 | D#4 |
| 5 | 64 | E4 |
| 6 | 65 | F4 |
| 7 | 66 | F#4 |
| 8 | 67 | G4 |
| 9 | 68 | G#4 |
| 10 | 69 | A4 |
| 11 | 70 | A#4 |
| 12 | 71 | B4 |
| 13 | 72 | C5 |
| 14 | 73 | C#5 |
| 15 | 74 | D5 |
| 16 | 75 | D#5 |

---

## Appendix C — Four-Week Practice Plan

This is a structured 4-week plan that takes you from novice to competent PO-33 musician. Each week has 7 days of practice (about 30–60 minutes per day).

### Week 1: Orientation and rhythm

- **Day 1**: Read Chapter 1. Record your first sound. Listen to J Dilla's *Donuts*.
- **Day 2**: Read Chapter 2. Build a one-sound, four-step beat.
- **Day 3**: Read Chapter 3. Set up the BPM. Match the tempo to a song.
- **Day 4**: Read Chapter 4. Record kick, snare, hi-hat. Build a basic rock beat.
- **Day 5**: Modify the basic rock beat: try four-on-the-floor, boom-bap, half-time, double-time.
- **Day 6**: Read Chapter 5. Record a 4-second break. Slice it. Build a beat from the slices.
- **Day 7**: Listen to 5 hip-hop tracks. Identify the kick-snare-hi-hat pattern in each.

### Week 2: Melody and harmony

- **Day 8**: Read Chapter 7. Play "Mary Had a Little Lamb" on the PO-33.
- **Day 9**: Read Chapter 8. Build a C major scale and a C minor scale. Listen to the difference.
- **Day 10**: Write a melody using only the notes of C major.
- **Day 11**: Read Chapter 9. Write a C-G bassline. Pair it with a drum beat.
- **Day 12**: Read Chapter 10. Write an I-vi-IV-V chord progression.
- **Day 13**: Combine bass + chord + drums. Manage the 4-voice polyphony.
- **Day 14**: Listen to 5 pop songs. Identify the chord progression in each (most will be I-V-vi-IV).

### Week 3: Genre

- **Day 15**: Read Chapter 11. Build a boom-bap beat.
- **Day 16**: Read Chapter 12. Build a house beat.
- **Day 17**: Read Chapter 13. Build a techno beat.
- **Day 18**: Read Chapter 14. Build a lo-fi beat.
- **Day 19**: Modify one of your beats. Add parameter locks. Make it feel "alive".
- **Day 20**: Compare your four beats. Which one do you like best? Why?
- **Day 21**: Listen to 4 songs, one in each genre. Match the BPM on your BO-33.

### Week 4: Full tracks and performance

- **Day 22**: Read Chapter 15. Write 4 patterns (intro, verse, chorus, bridge).
- **Day 23**: Chain the patterns. Perform a 1-minute song.
- **Day 24**: Read Chapter 16. Add parameter locks to your song.
- **Day 25**: Read Chapter 17. Perform a 4-minute live set. Record it.
- **Day 26**: Mix the recording in a DAW. Apply EQ, compression, reverb.
- **Day 27**: Upload the recording to SoundCloud or Bandcamp.
- **Day 28**: Share the recording with a friend. Listen to their feedback. Make another one.

### Beyond week 4

- Build a 16-pattern song.
- Sample other artists (with permission).
- Perform live in front of an audience.
- Buy a real PO-33 K.O! (they are excellent).
- Read the Allen and Hewitt theory books in full.
- Make a track on every BPM, every swing setting.

---

## Appendix D — Troubleshooting Common Issues

This appendix covers the most common problems you'll encounter on the PO-33 and the RavinePhoenix firmware.

### Problem: My recording sounds distorted.

**Cause**: the recording was too loud. The PO-33's microphone is sensitive. The internal limiter clips at high volumes.

**Fix**: record the sound again, but quieter. Move the sound source farther from the mic. Or use the line-in instead of the mic.

### Problem: My recording has silence at the start.

**Cause**: you held REC + pad, then took a moment to make the sound. The PO-33 recorded the silence.

**Fix**: use Tweak Trim mode (FX triple-tap to TRIM). Turn Knob A to move the start point. Trim the silence off.

### Problem: My beat sounds robotic.

**Cause**: no swing. Swing is what makes a beat feel "human".

**Fix**: hold BPM + turn Knob A to 2 or 3. The beat will feel "groovy".

### Problem: My kick and snare are on the same step (step 5, step 13), and one of them doesn't play.

**Cause**: the PO-33 has only 4 voices of polyphony. If you have kick + snare + hi-hat + bass all on step 5, the PO-33 may not have a voice for the snare.

**Fix**: drop one voice. For a beat with kick + snare + hi-hat + bass, drop the hi-hat on the kick-snare steps (steps 1, 5, 9, 13).

### Problem: My chord doesn't sound right.

**Cause**: the sample is too short. A chord that lasts 1 second will cut off after 1 second.

**Fix**: use a sustained sound ("ahhh", a synth pad, a guitar chord). Trim the sample to start at the attack, not at the silence.

### Problem: My drum sample is in slot 9 but the wrong slice plays when I press pad 9.

**Cause**: the auto-slicing splits the recording into 16 equal slices. If your recording is short, the slices are also short. If the recording is 4 seconds, each slice is 0.25 seconds.

**Fix**: trim the recording to a power-of-2 length (2 seconds, 4 seconds, 8 seconds). The slices will be cleaner.

### Problem: My parameter lock doesn't stick.

**Cause**: you didn't hold the pad long enough. Parameter locks are set when you hold the pad and turn a knob, then release.

**Fix**: hold the pad for at least 0.5 seconds while turning the knob. Release the pad AFTER the knob is at the desired position.

### Problem: The PO-33 doesn't respond to button presses.

**Cause**: the firmware is locked. The PO-33 has a "key lock" feature that prevents accidental presses.

**Fix**: hold the lock button (or any modifier) + tap the unlock sequence. On the RavinePhoenix firmware, the unlock is "hold WRITE for 2 seconds".

### Problem: My song doesn't loop properly.

**Cause**: the chain has gaps. You added pattern 1, then pattern 3, then pattern 2 (instead of 1, 2, 3). The chain plays in the order you added them.

**Fix**: clear the chain (hold PATTERN + REC). Re-add the patterns in the correct order.

### Problem: I can't remember which pad is which slot.

**Cause**: you haven't labelled your slots. The PO-33's slots are all named "slot 1" through "slot 16" by default.

**Fix**: use the SOUND key to preview slots. Hold SOUND + pad to play the slot without writing it to a step. Listen to identify the sound.

### Problem: My recording won't save.

**Cause**: the PO-33's sample memory is full. 40 seconds of samples fills up quickly.

**Fix**: delete unused slots. Hold REC + the pad for 2 seconds to delete a slot's sample.

---

## Appendix E — Sampling Ethics, Sources, and Further Reading

### Sampling ethics

The Amen break has been sampled over 7,000 times. Some artists have paid royalties. Many have not. The legal landscape is murky. The ethical landscape is clearer:

- **Sample yourself**: clap, sing, hit a desk, record your own voice. You own the copyright.
- **Sample with permission**: services like Tracklib (https://tracklib.com) license samples for commercial use.
- **Use royalty-free samples**: services like Splice (https://splice.com) and Loopmasters (https://loopmasters.com) sell royalty-free samples.
- **Use Creative Commons samples**: sites like Free Music Archive (https://freemusicarchive.org) and ccMixter (http://ccmixter.org) host samples with permissive licenses.

For commercial release, you must clear every sample. For practice and learning, sample freely and ethically.

### Sample sources

- **Tracklib**: commercial samples, cleared for use.
- **Splice**: subscription-based sample library.
- **Loopmasters**: per-sample purchase.
- **Free Music Archive**: Creative Commons samples.
- **ccMixter**: Creative Commons remix samples.
- **Freesound.org**: Creative Commons sound effects.

### Further reading

#### Music theory

- **J. Anthony Allen, *Music Theory for Electronic Music Producers*** (Slam Academy, 2018). The book this course draws from. Covers harmony, chord progressions, and song structure for electronic music.
- **Michael Hewitt, *Music Theory for Computer Musicians*** (Cengage, 2008). The other book this course draws from. Covers rhythm, melody, harmony, and song structure for producers.
- **Mark Levine, *The Jazz Piano Book*** (Sher Music, 1989). For those who want to go deeper into harmony.

#### Production

- **Rick Snoman, *Dance Music Manual*** (Routledge, 2019). The textbook for dance music production.
- **Mike Senior, *Mixing Secrets for the Small Studio*** (Routledge, 2011). The textbook for mixing in a home studio.

#### History

- **Jeff Chang, *Can't Stop Won't Stop*** (Picador, 2005). The history of hip-hop.
- **Tim Lawrence, *Love Saves the Day*** (Duke University Press, 2004). The history of dance music.
- **Simon Reynolds, *Energy Flash*** (Faber and Faber, 1998). The history of rave culture.

#### PO-33-specific

- **Teenage Engineering PO-33 K.O! Manual**. The official manual. Fold-out sheet of paper.
- **RavinePhoenix DESIGN.md**. The firmware design document for the RavinePhoenix project (this is what you are reading this book alongside).
- **Teenage Engineering YouTube channel**. Videos of PO-33 artists at work.

#### Listening

- **Spotify "lo-fi beats" playlist**. The best of lo-fi hip-hop.
- **YouTube "PO-33 K.O!" playlist**. Videos of PO-33 musicians at work.
- **Bandcamp "PO-33" tag**. PO-33 tracks uploaded by users.

### Final word

You have finished the book. You can now make beats on the PO-33. You can build beats in hip-hop, house, techno, and lo-fi genres. You can layer drums, bass, melody, and chords. You can perform a 4-minute live set. You can record your music to a DAW. You can mix and master your track.

The next step is to **make a track**. Pick a genre. Pick a sample. Pick a chord progression. Make a beat. Record it. Release it. Share it. You are a music producer.

The PO-33 is a small device. Its memory is 40 seconds. Its polyphony is 4 voices. Its sequencer is 16 steps. These are not limitations — they are **boundaries**. Within those boundaries, you have made music. Within those boundaries, you have built beats that sound like the records you love.

The boundaries are small, but the music is yours.

Good luck.

---

*End of Part 5 (final). The book is complete. Total word count: approximately 21,000 words.*


---

*End of book. Edition 1.*
