# PO-33 K.O! ESP32-S3 — Hardware Build Guide

> **Audience:** this guide is for someone who has never soldered electronics before but is willing to learn. Every tool is named, every part has a part number, every wire is labelled. If you have soldered before, you can skim §1–§2 and skip straight to §5 (build order).
>
> **Time required:** 2–4 hours for a first build (mostly waiting for the soldering iron to heat up and for you to read the next step carefully).
>
> **Cost:** roughly **$15–$25** for the additional parts, on top of the **$5–$10** for the ESP32-S3 dev board itself.

This guide is the build companion to **`docs/DESIGN.md`** (which explains *what* the firmware does and *why*) and to the [project website](https://ravinephoenix.1une.cc) (which has the web flasher). This document is the third leg: *how* you turn a pile of parts into a working PO-33 emulator.

---

## 1. Who this is for, and how to stay safe

### 1.1 Who this is for

This guide assumes you have:

- **Never soldered before**, OR
- Soldered before but never worked with microcontroller boards.

If you are an experienced electrical engineer, this guide will be too verbose for you — skip ahead to §5 and use the pin table in §4.4 as your wiring reference.

### 1.2 Stay safe

You will be working with:

- **A soldering iron at ~350 °C.** It will burn you instantly if touched. Treat it like a stovetop burner: never rest your hand near it, always return it to its stand, never touch the metal tip.
- **A USB-C cable.** The ESP32-S3 is powered through USB. The board is 5 V. There is no mains voltage at any point in this build.
- **A battery.** This guide's default is a single **2800 mAh protected Li-ion cell** (18650 form factor). It is *not* optional in the BOM — §3 part #8 — because the device's deep-sleep standby lifetime depends on it. If you are determined to skip the battery, run on USB only; the runtime tables in §4.10 will not apply. Safety rules: do not puncture the cell, do not short the + and − terminals together, do not charge a damaged battery, do not dispose of a spent cell in the trash.

If you are unsure about any step, **stop and ask**. The [project Discussions page](https://github.com/peter/RavinePhoenix.1une.cc/discussions) (or wherever the project is hosted) is a good place to ask. There is no urgency.

### 1.3 Read this first

Before you start, read all of §2 and §3 once. Even if you don't understand every word, knowing what is coming reduces mistakes.

---

## 2. Tools you will need

| Tool | Approximate cost | Notes |
|---|---|---|
| Soldering iron, ~30 W, temperature-controlled | $15–$40 | A "TS100" or "Pinecil V2" is a great starter; even a $15 fixed-temperature iron works but is harder. |
| Solder | $5 | Lead-free (RoHS) is safer; leaded (60/40) is easier to use. Either works for this build. |
| Wire cutters / flush cutters | $5 | For trimming component leads after soldering. |
| Tweezers | $3 | Helpful for placing surface-mount components (we don't use any in v1, but useful to have). |
| Multimeter (optional) | $10–$30 | Useful for checking connections and diagnosing problems. |
| USB-C cable, data-capable | $3 | **Important: must support data, not just charging.** Some cheap cables are power-only. |
| A small Phillips #0 screwdriver | $2 | For the screws on the dev board's header pins (if applicable). |
| A laptop running Windows, macOS, or Linux | free | For the web flasher and UART shell. |
| A small Phillips #1 screwdriver | $2 | For the speaker/amp screws. |

Total tool cost if you own nothing: **$50–$100**. Total tool cost if you already have a soldering iron: **$15–$30**.

If you don't own a soldering iron and don't want to buy one, see §8.2 for the "no-solder" alternative (a pre-soldered ESP32-S3 dev board with header pins already attached, and you use **Dupont jumper wires** to connect the parts).

---

## 3. Bill of materials (BOM)

This is the full list of parts. **Every part is required** unless the part's "Required?" column says otherwise. "Optional" parts make the build nicer but the firmware will work without them.

| # | Part | Part number | Required? | Qty | ~Cost (USD) | Where to buy |
|---|---|---|---|---|---|---|
| 1 | ESP32-S3 dev board with **Octal PSRAM** and ≥ 16 MB flash | ESP32-S3-WROOM-1-N16R8 module on a development board (e.g. ESP32-S3-DevKitC-1, Waveshare ESP32-S3-Zero, or a generic "ESP32-S3 N16R8" board on Amazon) | **Yes** | 1 | $5–$10 | Amazon, AliExpress, Mouser, DigiKey, Waveshare, Espressif |
| 2 | Audio DAC breakout | **PCM5102A** module (the common GY-PCM5102A or equivalent) | **Yes** | 1 | $2–$4 | Amazon, AliExpress, eBay |
| 3 | I²S MEMS microphone breakout | **INMP441** module (the common "CJMCU-441" or Adafruit #4466) | **Yes** | 1 | $2–$5 | Amazon, Adafruit, AliExpress, SparkFun |
| 4 | 2.4″ TFT display, ILI9341 driver, SPI | **ILI9341** 240×320 module (the common "2.4 inch SPI TFT" with an SPI interface, NOT the parallel one) | **Yes** | 1 | $4–$8 | Amazon, AliExpress, Adafruit (#1770), Waveshare |
| 5 | 4×4 matrix of 16 tactile buttons (for the **step buttons** 1–16) | Either a pre-built 4×4 membrane keypad (Amazon "4x4 matrix keypad") OR 16 individual 6 mm tactile switches + a small PCB or perfboard | **Yes** | 1 (or 16) | $2–$4 | Amazon, SparkFun, Adafruit, Digikey |
| 5b | **7 individual 6 mm tactile switches** (for the **modifier buttons**: SOUND, PATTERN, BPM, REC, FX, PLAY, WRITE) | Any 6×6 mm tactile switch; one per button | **Yes** | 7 | < $1 | Amazon, SparkFun, Adafruit, Digikey |
| 6 | Speaker | 4 Ω or 8 Ω small loudspeaker, 0.5 W–3 W | Optional | 1 | $2–$5 | Amazon, any electronics store; you can also cannibalize from an old set of powered speakers |
| 7 | I²S class-D amplifier (drives the speaker) | **MAX98357A** breakout (Adafruit #3006 or SparkFun) | Optional (only if you add a speaker) | 1 | $6–$10 | Adafruit, SparkFun, Amazon |
| 7b | **Two 10 kΩ linear potentiometers** + 2 panel-mount knobs (for "Knob A" and "Knob B") | Any 10 kΩ linear-taper pot (Bourns PTV09A, Alpha RV16AF-10K, or equivalent). **Linear** taper (not audio/log) — small adjustments near one end need to feel uniform across the range. | Recommended | 2 | $1–$3 | Amazon, Mouser, Digikey, SparkFun |
| 8 | LiPo battery, **single cell, 3.7 V nominal (4.2 V max), 2800 mAh** | A 18650-size protected Li-ion cell (e.g. **Panasonic NCR18650B**, **Samsung INR18650-30Q**, or an Adafruit #328-equivalent LiPo pouch). Must include a built-in protection circuit (PCM/BMS) against over-discharge, over-charge, and short circuit. | Recommended (chosen for this guide) | 1 | $5–$12 | Amazon, Adafruit, SparkFun, eBay, 18650BatteryStore, Illumn |
| 9 | LiPo charging module (if you use a battery) | **TP4056** module (with protection, no separate load-share needed) | Optional (only with battery) | 1 | $1 | Amazon, AliExpress |
| 10 | USB-C breakout (only if your dev board has micro-USB, not USB-C) | Generic USB-C breakout | Only if needed | 1 | $1 | Amazon, Adafruit |
| 11 | Jumper wires (Dupont, female-to-female, 10 cm, 40-pin ribbon) | — | **Yes** | 1 strip of 40 | $1–$2 | Amazon, any electronics store |
| 12 | Breadboard (830 tie points) | — | Optional (very helpful) | 1 | $2–$5 | Amazon, Adafruit, SparkFun |

### 3.1 Total cost summary

| Configuration | Cost (USD) |
|---|---|
| **Minimal** (parts 1–5, 5b + 11): no speaker, no battery, no amp | **$16–$26** |
| **With speaker** (add parts 6, 7): | **$25–$40** |
| **With battery** (add parts 8, 9): the **2800 mAh LiPo + TP4056 charger** | **$30–$45** |
| **Everything** (parts 1–12): | **$35–$60** |

### 3.2 What to look for when ordering

- **ESP32-S3-N16R8** — verify the board is the **N16R8** variant specifically. Other variants (e.g. N4, N8, N32) have different RAM amounts and the firmware may not work. The "N16" means 16 MB flash; "R8" means 8 MB Octal PSRAM. We use the PSRAM heavily.
- **PCM5102A** — there are several boards. Make sure it has an **I²S input** (sometimes called "digital input"). Some cheap boards are I²S-only with no analog fallback. Either is fine.
- **INMP441** — verify the board has 3.3 V compatible logic. The INMP441 chip itself is 3.3 V; some breakout boards include level shifters that may interfere. The common "CJMCU-441" board works fine on 3.3 V.
- **ILI9341** — there are two common variants: SPI and parallel. We need **SPI**. Look for boards labelled "SPI TFT" or that expose SCK/MOSI/CS pins.
- **4×4 keypad** — the cheap membrane keypads from Amazon work great. Look for "4x4 matrix array 16 key" or similar. It exposes exactly 8 pins: 4 rows + 4 columns.
- **Speaker** — any small 4 Ω or 8 Ω speaker works. 0.5 W is plenty for a tabletop device.
- **MAX98357A** — Adafruit #3006 or SparkFun breakout. Verify the gain-select pin (GAIN) is broken out so you can set it to 9 dB.
- **Battery (2800 mAh LiPo / Li-ion)** — this guide picks a single 18650-format protected Li-ion cell at **2800 mAh**, which is the sweet spot of capacity, size, and cost for a tabletop device. A genuine **Panasonic NCR18650B** is rated 3400 mAh but is commonly de-rated to 2800 mAh at 0.2C continuous discharge (real-world capacity, not the marketing number). For the most honest number, use 2800 mAh as the *effective* capacity, not the printed 3400 mAh. **Always buy protected cells** (the protection circuit prevents over-discharge below ~2.5 V, which permanently damages Li-ion). Avoid cheap "3000 mAh", "4000 mAh", "5000 mAh" cells on Amazon — those numbers are routinely fabricated. Stick to name-brand cells (Panasonic, Samsung, LG, Sony, Murata) from reputable sellers. **Do not buy cells without a protection circuit.**

---

## 4. Schematic, in text

This section describes the schematic. There is no picture — text + tables is more useful for a first build, because text can be copy-pasted into a search box.

### 4.1 Power

- **USB-C 5 V** → dev board's USB-C jack. This powers everything else.
- **Battery (this guide's default):** a single **2800 mAh protected Li-ion cell** (18650 form factor, 3.7 V nominal, 4.2 V max) charges via the **TP4056** module and powers the dev board's `5V` rail. Wire as: battery `+` → TP4056's `B+` and `OUT+`; battery `−` → TP4056's `B−` and `OUT−`; TP4056's `OUT+` → dev board's `5V` pin; TP4056's `OUT−` → dev board's `GND`. The TP4056's `IN+/IN−` go to a USB-C breakout's +/−. **Do not** back-feed 5 V into the dev board's USB-C jack — power either the + (when charging) or the TP4056 (when discharging), not both simultaneously.
- If using a battery without the dev board's USB: most ESP32-S3 dev boards do not include a LiPo charger on-board; the TP4056 handles charging.
- See **§4.10 Battery: capacity, charging, life estimate** for the expected runtime on the 2800 mAh cell.

### 4.2 The audio DAC (PCM5102A)

The PCM5102A board exposes these pins (most boards label them on the silkscreen):

| PCM5102A pin | Connects to | ESP32-S3 pin | Notes |
|---|---|---|---|
| VIN | 3.3 V | 3V3 (or 3.3 V on dev board) | The DAC needs a clean 3.3 V. |
| GND | Ground | GND | Common ground is essential. |
| BCK (BCLK) | Bit clock | **GPIO 9** | I²S bit clock. |
| LCK (LRCK / WS) | Word select | **GPIO 10** | I²S word clock. |
| DIN (DATA) | Data in | **GPIO 8** | I²S data from ESP32 → DAC. |
| SCK (MCLK) | System clock | *not connected* | The PCM5102A generates its own internal PLL from BCLK; you do not need to feed MCLK. Leave it floating. |
| FMT | Format select | *not connected* (or tied to GND for I²S, not "left-justified") | Default I²S mode is correct for our firmware. |
| XSMT | Soft-mute | *not connected* | Pulled up internally on most boards; mute is off. |

Most PCM5102A boards also have an `OUT L+`, `OUT L-`, `OUT R+`, `OUT R-` for analog line output. You don't have to connect these to anything if you use the MAX98357A amp instead — but if you're using headphones or a line-in to another device, connect `OUT L+` and `OUT R+` to your output jack's tip and ring.

### 4.3 The microphone (INMP441)

| INMP441 pin | Connects to | ESP32-S3 pin | Notes |
|---|---|---|---|
| VDD | 3.3 V | 3V3 | The mic needs 3.3 V, not 5 V. |
| GND | Ground | GND | Common ground. |
| SCK | Serial clock | **GPIO 15** | I²S bit clock (shared with the DAC — both run on the same BCLK). |
| WS | Word select | **GPIO 16** | I²S word clock (shared with the DAC). |
| SD | Serial data | **GPIO 17** | I²S data from mic → ESP32. |
| L/R | Channel select | *GND for left channel* (or 3.3 V for right; we use left) | The INMP441 is mono; this pin tells it which channel of the I²S stream to put its data on. GND = left. |

### 4.4 The TFT (ILI9341, 240×320, SPI)

| TFT pin | ESP32-S3 pin | Notes |
|---|---|---|
| VCC | 3.3 V | Some boards accept 5 V; ours needs 3.3 V. |
| GND | GND | Common ground. |
| CS  | **GPIO 5**  | Chip select. |
| RESET (RST) | **GPIO 48** | Reset. |
| DC (RS) | **GPIO 4**  | Data/Command select. |
| MOSI (SDA) | **GPIO 6**  | SPI data from ESP32 → display. |
| SCK (CLK) | **GPIO 7**  | SPI clock. |
| LED (BL) | **GPIO 47** | Backlight, driven by LEDC PWM. |
| MISO | *not connected* | We don't read from the display. |

**Order matters when powering up:** the ESP32-S3 may briefly send random SPI traffic during boot. The TFT's reset pin (GPIO 48) keeps it in reset until the ESP32-S3 is ready. This is wired in the firmware, so just follow the table.

### 4.5 The step-button matrix (4 rows × 4 cols) — 16 step buttons

The 4×4 matrix holds the 16 polymorphic **step buttons** (numbered 1–16). On the real PO-33 these same physical buttons also mean "sample slot 1–16" when SOUND is held, "pattern 1–16" when PATTERN is held, "effect 1–15 (+16 = swing)" when FX is held, and so on. Our firmware uses the same matrix; whether a number means "step" or "slot" or "effect" depends on which modifier button is held.

```
         COL 0   COL 1   COL 2   COL 3
         GPIO33  GPIO34  GPIO39  GPIO40
         (input, (input, (input, (input,
         pull-up) pull-up) pull-up) pull-up)
        ┌──────┬──────┬──────┬──────┐
ROW 0   │      │      │      │      │
GPIO35  │ btn  │ btn  │ btn  │ btn  │
(drive   │  1   │  2   │  3   │  4   │
low)     │  =ST │  =ST │  =ST │  =ST │
        ├──────┼──────┼──────┼──────┤
ROW 1   │      │      │      │      │
GPIO36  │ btn  │ btn  │ btn  │ btn  │
        │  5   │  6   │  7   │  8   │
        ├──────┼──────┼──────┼──────┤
ROW 2   │      │      │      │      │
GPIO37  │ btn  │ btn  │ btn  │ btn  │
        │  9   │  10  │  11  │  12  │
        ├──────┼──────┼──────┼──────┤
ROW 3   │      │      │      │      │
GPIO38  │ btn  │ btn  │ btn  │ btn  │
        │  13  │  14  │  15  │  16  │
        └──────┴──────┴──────┴──────┘
```

How the matrix works:

1. The ESP32-S3 makes **exactly one row line low** at a time (output LOW). The other three rows are HIGH.
2. The ESP32-S3 **reads all four column inputs**.
3. If a column reads LOW, the button at that row × column intersection is pressed.

You can wire this with a pre-built membrane keypad (the 4×4 ones on Amazon come with 8 wires labelled `R1 R2 R3 R4 C1 C2 C3 C4`). Connect `R1` → GPIO35, `R2` → GPIO36, `R3` → GPIO37, `R4` → GPIO38, `C1` → GPIO33, `C2` → GPIO34, `C3` → GPIO39, `C4` → GPIO40.

If you're wiring individual tactile switches, place each switch so its two pins sit on a unique row × column wire pair, with one side going to the row trace and the other to the column trace. Most people use a small perfboard for this.

### 4.5b The 7 dedicated modifier buttons (one GPIO each)

In addition to the 4×4 matrix, the device has **seven dedicated (modifier) buttons**, each wired to a single GPIO. They mirror the dedicated buttons on the real PO-33 and are laid out the same way: **three across the top** of the panel and **four down the right-hand side**, with the top of that column tucked under Knob B.

```
      ┌────────────────────────────────────────────────────────────┐
      │              2.4"  TFT   ILI9341   240 × 320               │
      │                 SPI2   GPIO 4/5/6/7/47/48                  │
      │                                                            │
      └────────────────────────────────────────────────────────────┘


    ┌─────────┐  ┌──────────┐  ┌─────────┐   (o)         (o)
    │  SOUND  │  │ PATTERN  │  │   BPM   │  Knob A      Knob B
    └─────────┘  └──────────┘  └─────────┘ GPIO 20     GPIO 46
                                            A1_CH9      A1_CH5

    ┌───┐┌───┐┌───┐┌───┐                               ┌─────────┐
    │ 1 ││ 2 ││ 3 ││ 4 │                               │   REC   │
    └───┘└───┘└───┘└───┘                               └─────────┘
    ┌───┐┌───┐┌───┐┌───┐                               ┌─────────┐
    │ 5 ││ 6 ││ 7 ││ 8 │                               │   FX    │
    └───┘└───┘└───┘└───┘                               └─────────┘
    ┌───┐┌───┐┌───┐┌───┐                               ┌─────────┐
    │ 9 ││ 10││ 11││ 12│                               │  PLAY   │
    └───┘└───┘└───┘└───┘                               └─────────┘
    ┌───┐┌───┐┌───┐┌───┐                               ┌─────────┐
    │ 13││ 14││ 15││ 16│                               │  WRITE  │
    └───┘└───┘└───┘└───┘                               └─────────┘

    Step matrix  16 buttons on 8 GPIOs — rows 35/36/37/38   cols 33/34/39/40
    Top row      SOUND=11    PATTERN=44    BPM=13        + 2 knobs
    Right column REC=41   FX=12   PLAY=42   WRITE=43   (under Knob B)
    Knobs        Knob A = GPIO 20 (ADC1_CH9)   Knob B = GPIO 46 (ADC1_CH5)
    Totals       23 buttons + 2 knobs = 17 of 45 GPIOs   (12 free, incl. 45)
```

Each is a momentary tactile switch between the GPIO and GND (no matrix, no external resistor — the firmware enables the internal pull-up).

| Button | GPIO | What it does (PO-33 mode) | What it does (our firmware) |
|---|---|---|---|
| `SOUND`   | **GPIO 11** | hold S + number 1–16 plays / selects that slot | hold + step 1–16 = select / play a sample slot |
| `PATTERN` | **GPIO 44** | hold + number 1–16 picks a pattern | tap = next pattern, long press = previous |
| `BPM`     | **GPIO 13** | press cycles 80 / 120 / 140; hold + knob A = fine tempo | tap = +1 BPM, long press = −1 BPM |
| `REC`     | **GPIO 41** | record (hold + number records into that slot) | start / stop recording |
| `FX`      | **GPIO 12** | hold + number 1–16 applies that effect | enter FX-select mode |
| `PLAY`    | **GPIO 42** | play / stop pattern | start / stop sequencer |
| `WRITE`   | **GPIO 43** | enter write mode (·) | enter / exit write mode |

Why these GPIOs? All seven are general-purpose, none are strapping pins (GPIO 0–3 are strapping on the ESP32-S3 and we avoid them), and none conflict with I²S (8/9/10/15/16/17), the TFT (4/5/6/7/47/48), the button matrix (33/34/35/36/37/38/39/40), or the two knobs (20 and 46). **GPIO 45** — which used to carry `PAT ↓` — is now free for future expansion.

Each switch needs two wires:

```
       GPIO pin ──── one leg of the tactile switch
                      other leg ──── GND
```

The internal pull-up is enabled in firmware, so no external resistor is needed.

If you would rather wire all 23 buttons (16 matrix + 7 dedicated) onto a single perfboard with one shared GND bus, you will end up with ~8 wires from the dev board to the perfboard: 4 row + 4 column + 7 dedicated GPIO signals (the 7 GNDs share one common wire).

### 4.6 Status LEDs (optional)

| LED color | GPIO | Resistor |
|---|---|---|
| Red ("REC")    | **GPIO 21** | 220 Ω to + leg of LED; − leg to GND |
| Green ("PLAY") | **GPIO 14** | 220 Ω to + leg of LED; − leg to GND |

If you skip the LEDs, the firmware still works. The TFT shows "REC ●" and "▶ PLAY" instead.

### 4.7 Sync in / sync out

| Signal | GPIO | Connector |
|---|---|---|
| Sync OUT | **GPIO 18** | 3.5 mm jack **tip** (with 100 Ω series resistor); jack **sleeve** to GND |
| Sync IN  | **GPIO 19** | 3.5 mm jack **tip**; jack **sleeve** to GND |

Sync uses the Pocket Operator "SY2/SY3/SY4" protocol (pulse on every 16th note, max 5 Vpp). If you don't need to sync with another Pocket Operator, skip the jacks — the firmware still works without them.

### 4.8 Battery monitor (optional)

The dev board has an ADC pin exposed. Our config uses **ADC1 channel 3**, which on the ESP32-S3-DevKitC maps to **GPIO 4**. With the **2800 mAh Li-ion cell** from §3, you can read the cell voltage to display a battery percentage on the TFT (planned; not yet implemented in v1 firmware — see [DESIGN.md §3.12 F-035](../DESIGN.md)).

You need a voltage divider because a fully charged Li-ion cell is 4.2 V but the ESP32-S3's ADC tops out at 3.3 V. The simplest divider is two equal resistors (say 100 kΩ each) between the cell's + terminal and GND, with the midpoint connected to the ADC pin. We document the divider ratio as 2.0:1 in `config.h` (`BATTERY_DIVIDER_RATIO`). If you use different resistors, update that constant. The divider draws about 21 µA continuously — negligible vs the rest of the system.

> **Note:** Li-ion cells below ~3.0 V should be considered discharged. The protection circuit on a protected 18650 will cut off at ~2.5 V to prevent damage. The voltage-divider reading is not perfectly linear because the cell has a non-linear discharge curve; the firmware-side mapping (when implemented) should account for this.

### 4.9 The I²S class-D amp (MAX98357A, optional)

If you added a speaker, wire the MAX98357A as a second I²S *receiver* — it takes its own BCLK, LRCK, and data from the same ESP32-S3 pins the PCM5102A uses (BCLK = GPIO9, LRCK = GPIO10, DATA = GPIO8). The PCM5102A and MAX98357A can share the same I²S bus; only one needs to be active at a time. Our firmware currently drives only the DAC; running both simultaneously would require an analog mux. **In v1, treat the MAX98357A as future work.**

The wiring in short:

| MAX98357A pin | ESP32-S3 pin |
|---|---|
| VIN | 5 V (USB) or 3.7 V (LiPo) — the amp can run from 2.7 V to 5.5 V |
| GND | GND |
| BCLK | GPIO 9 |
| LRCK | GPIO 10 |
| DIN | GPIO 8 |
| GAIN | tied to GND (15 dB) or to 3.3 V (9 dB, recommended) |
| SD | tied to 3.3 V (always on) — but our v1 firmware doesn't drive this pin |
| OUT+ / OUT- | speaker terminals |

---

### 4.10 Battery: capacity, charging, life estimate

This section assumes you built with the **2800 mAh protected Li-ion cell** (part #8) and the **TP4056 charger** (part #9).

#### 4.10.1 Capacity, voltage, and charge cycle

- **Nominal voltage:** 3.7 V (a single Li-ion cell's average operating voltage).
- **Maximum voltage (full charge):** 4.2 V.
- **Cutoff voltage (empty, protected):** ~2.5 V. The TP4056 cuts off charging at 4.2 V; the protection circuit on the cell cuts off *discharging* at ~2.5 V.
- **Effective capacity:** **2800 mAh** at the 0.2C continuous discharge rate (about 560 mA draw). Above this current the cell delivers slightly less — see §4.10.4.
- **Charging current:** the TP4056 module's default charge rate is set by a single resistor (`R3` on most TP4056 boards). With the stock 1.2 kΩ resistor the module charges at ~1000 mA (1 A). For our 2800 mAh cell, that's a C/2.8 charge rate — safe for overnight charging, but the cell will be warm during the first hour. **If you want a gentler charge (better for cell longevity), replace R3 with a 2 kΩ resistor to drop the charge current to ~580 mA.** Then a full 0 → 100 % charge takes roughly 2800 / 580 ≈ 5 hours.
- **Charge cycle life:** name-brand protected 18650 cells (Panasonic, Samsung) are typically rated for **500–1000 full charge cycles** before capacity drops below 80 % of original. A 0.5 C charge current is gentler than 1 C and roughly doubles cycle life in real-world use.

#### 4.10.2 What consumes power

The numbers below are *typical*, drawn from datasheets + Espressif's published ESP32-S3 power numbers. **They are estimates**, not measured on this exact build. To get real numbers, put a USB power meter (~$10) between the TP4056 output and the dev board's `5V` pin.

| Subsystem | Active current | Notes |
|---|---|---|
| ESP32-S3 CPU, both cores @ 240 MHz, WiFi off, AMY rendering 4 voices | **~50 mA** | from Espressif's ESP32-S3 hardware design guide; rises slightly with active WiFi/BLE (~+200 mA peak when transmitting) |
| I²S DAC (PCM5102A), 44.1 kHz stereo out | **~10 mA** | from PCM5102A datasheet |
| TFT backlight (ILI9341), 70 % duty PWM, white pixels | **~60 mA** | dominant load; backlight is the biggest knob |
| TFT backlight, 30 % duty | **~20 mA** | |
| TFT backlight, off (e.g. while in deep sleep) | **< 1 mA** | |
| Microphone (INMP441), always-on | **~1 mA** | |
| Status LEDs (when lit) | **~5 mA each** | usually off |

**Totals** (typical operation):

| Mode | Current |
|---|---|
| **Playing music, full TFT** (default) | ~120 mA |
| **Playing music, TFT dimmed to 30 %** | ~80 mA |
| **Playing music, TFT off** (line-out to headphones only) | ~65 mA |
| **Idle but awake** (no music, screen on, button-scan running) | ~70 mA |
| **Deep sleep** (firmware idle, no rendering, only RTC + GPIO wake) | ~10 µA ESP32 + 55 µA TP4056 + 21 µA divider = **~86 µA ≈ 0.1 mA** |

#### 4.10.3 Battery life estimate (2800 mAh)

Using the totals above, here is the expected runtime on a fully-charged cell. **Divide by 1.25 if you want a conservative estimate** that accounts for real-world capacity being ~80 % of the printed number, temperature, and aging.

| Usage pattern | Average current | Battery life |
|---|---|---|
| **Always on, full TFT, playing** (worst case) | ~120 mA | **~23 hours** (≈ 1 day) |
| **Always on, TFT dimmed 30 %** | ~80 mA | **~35 hours** (≈ 1.5 days) |
| **Always on, TFT off** (line-out to amp) | ~65 mA | **~43 hours** (≈ 1.8 days) |
| **Heavy use: 4 hr/day active + 20 hr deep sleep** | ~20 mA avg | **~140 hours ≈ 5.8 days** |
| **Moderate use: 1 hr/day active + 23 hr deep sleep** | ~5 mA avg | **~560 hours ≈ 23 days** |
| **Light use: 15 min/day active + 23.75 hr deep sleep** | ~1.3 mA avg | **~2150 hours ≈ 3 months** |
| **Stored, idle, no button presses for weeks** | ~0.1 mA | **~30 000 hours ≈ 3.4 years** |
| **Standby + occasional 5-minute check-in** | ~0.1 mA | **~28 000 hours ≈ 3.2 years** |

**Summary**: on a single 2800 mAh charge, expect **2–4 weeks of regular (15-min/day) use**, **~3 weeks of moderate use**, or **3+ years of pure standby**. The 5-minute idle-to-deep-sleep timer in the firmware (`BTN_IDLE_SLEEP_MS = 5 * 60 * 1000`) is what makes the long standby numbers possible.

#### 4.10.4 What affects these numbers

- **TFT backlight brightness** is the largest variable. Dropping from 70 % (default) to 30 % nearly halves active current.
- **Number of active voices**. AMY defaults to 4 voices; each extra voice is ~1 mA on the CPU.
- **Cell temperature**. Cold cells deliver less than rated capacity; hot cells degrade faster. The numbers above assume ~20 °C ambient.
- **Cell age**. A 2-year-old 18650 may only hold 80 % of its original capacity.
- **WiFi / BLE** (not used in v1 firmware, but if added): a WiFi transmission burst pulls an extra ~200 mA. Battery life during streaming would drop by 60 %.
- **Recording**. While recording, the INMP441 mic is active and AMY runs analysis. Adds ~5 mA vs. plain playback.
- **Volume**. Higher volume on the PCM5102A's line-out does not significantly change current (it's a line-level output, not a speaker driver).

#### 4.10.5 Charging from USB-C

- **Charging time**: 0 → 100 % takes 3–5 hours depending on the TP4056's programmed charge current. The LED on the TP4056 module turns off (or changes colour) when charging is complete.
- **Charging while playing**: the TP4056 will charge the cell and power the dev board simultaneously — this is fine, but the cell may get warm. To preserve cell life, charge when the device is not in use.
- **No over-charge risk**: the TP4056 cuts off at 4.2 V. You can leave the device plugged in indefinitely.
- **Over-discharge protection**: the protection circuit on a protected 18650 cuts off at ~2.5 V. The firmware will *not* be able to boot if the cell is below ~3.0 V (because the ESP32-S3 needs 3.0 V+ on its 3V3 rail, which the dev board's LDO derives from `5V`).

#### 4.10.6 Replacing the cell

When the cell no longer holds a useful charge (after 1–3 years), open the enclosure, swap in a fresh **2800 mAh protected 18650**, and reassemble. **Do not throw the old cell in the trash** — most hardware stores and electronics retailers (Best Buy, Home Depot, Lowes) accept Li-ion batteries for recycling.

---

### 4.11 The two analog knobs (Knob A and Knob B)

The real PO-33 has two physical knobs labelled **A** and **B**. They have no semantic name on the device itself — what they do depends on the active tweak mode. We wire two 10 kΩ linear potentiometers on ADC-capable GPIOs and read them through `esp_adc_cal`.

| Knob | GPIO | ADC channel | ADC unit | Notes |
|---|---|---|---|---|
| **Knob A** | GPIO 20 | ADC1_CH9 | ADC1 | non-strapping, ADC-capable |
| **Knob B** | GPIO 46 | ADC1_CH5 | ADC1 | non-strapping, ADC-capable |

Each potentiometer is wired as a 3-terminal voltage divider:

```
3.3 V ──┐ ├─ 10 kΩ linear pot ──┐
          │                      ├─ wiper ── GPIO 20 (Knob A) or GPIO 46 (Knob B)
   GND ──┘                      │
                               │
                          (to ADC pin)
```

The firmware:

- Configures the ADC channel with `ADC_ATTEN_DB_12` (full-scale ≈ 3.3 V).
- Reads **8 samples** per `knobs_tick()` and averages them.
- Applies a small **dead-zone** of ±4 in 0..255 units, so ADC noise doesn't show as jitter.
- Exposes the value as `knobs_get_a()` / `knobs_get_b()` returning `uint8_t` (0..255).

#### What each knob does (matching the PO-33 manual)

The PO-33 names the knobs simply "A" and "B"; what they do depends on which tweak parameter is active (toggled with FX).

| Active mode | Knob A | Knob B |
|---|---|---|
| **Tone** (`ton`) | pitch (semitones) | volume |
| **Filter** (`Flt`) | low/high-pass cutoff frequency | resonance |
| **Trim** (`tri`) | sample start point | sample length |
| `BPM` held | fine tempo adjustment | cycle 3 tempo levels (Hip Hop / Disco / Techno) |
| (no mode active) | repeats the last-set tweak | repeats the last-set tweak |

In v1 firmware, only the "`BPM` held" row is wired through — knobs A and B move the tempo up/down by ±1 BPM per tick when the `BPM` modifier button is held. The tweak-mode rows are documented in `docs/DESIGN.md` §3.4 (F-016 / F-017 / F-018) and will be wired in a future revision; the data model already supports them.

#### Knob vs. modifier button: which is "right"?

The PO-33 uses knobs for fine continuous control and modifier buttons (SOUND, PATTERN, BPM, REC, FX, PLAY, WRITE) for discrete actions. We follow the same model:

- Use **knobs** when you want a smooth, continuous value (tempo, filter cutoff, pitch).
- Use **modifier buttons** when you want a discrete action (start recording, switch pattern).

The `PATTERN` and `BPM` buttons step through in whole units; the knobs would let you dial in a tempo precisely.

---

## 5. Build order

Do these steps **in order**. Don't skip ahead.

### 5.1 Step 1 — Inventory

Lay every part out on a clean desk. Check the BOM in §3 against your pile. It's much easier to discover a missing part now than halfway through step 5.

### 5.2 Step 2 — Test the ESP32-S3 dev board before wiring anything else

1. Plug the dev board into your laptop via USB-C.
2. Open a serial monitor at **115200 baud** on the dev board's USB serial port (most boards expose this as `/dev/ttyUSB0` on Linux, `COM3` on Windows, `/dev/cu.usbserial-*` on macOS). Tools: `minicom`, `PuTTY`, `moserial`, the Arduino IDE Serial Monitor, or the ESP-IDF `idf.py monitor`.
3. Press the dev board's RESET button. You should see boot messages — at minimum a chip ID line and an "ESP-ROM:..." line. If you see nothing, the board is broken or your cable is power-only — try a different cable.
4. From the [project website](https://ravinephoenix.1une.cc), flash the firmware. After flashing, you should see `=== ESP32-S3 PO-33 K.O! boot ===` followed by PSRAM and heap stats. **At this stage the firmware will fail to initialize because the I²S DAC and TFT aren't wired yet — that's expected.**

### 5.3 Step 3 — Wire the audio DAC (PCM5102A)

This is the easiest wiring step. Six wires total.

1. PCM5102A `VIN` → dev board `3V3`
2. PCM5102A `GND` → dev board `GND`
3. PCM5102A `BCK` → dev board `GPIO 9`
4. PCM5102A `LCK` → dev board `GPIO 10`
5. PCM5102A `DIN` → dev board `GPIO 8`
6. PCM5102A `SCK` (MCLK) → leave floating

After wiring, flash the firmware again. Connect headphones to the PCM5102A's `OUT L/R` jack (or to your line-out). Open the UART shell at 115200 baud. Press **Enter** to get a `po33>` prompt, then type:

```
po33> bpm 120
po33> play
```

You should hear a brief startup click (AMY's `amy_bleep` is disabled in our firmware, but the I²S init may emit a tick). Without any samples recorded yet, no further sound will play — that's expected. The fact that you got the boot log and the UART prompt means the I²S bus initialized cleanly.

If the UART shell is silent (no `po33>` prompt), check:
- Your USB cable is data-capable.
- Your serial monitor baud is 115200.
- The dev board's reset button was pressed after the monitor connected.

If the UART shell works but you hear nothing at all from the DAC, check:
- The PCM5102A is getting 3.3 V at `VIN`. Use a multimeter.
- The `BCK` and `LCK` pins aren't swapped.
- The PCM5102A's `FMT` pin is in I²S mode (default on most boards). If the board has a jumper or switch for I²S vs. left-justified, set it to I²S.

### 5.4 Step 4 — Wire the microphone (INMP441)

1. INMP441 `VDD` → dev board `3V3`
2. INMP441 `GND` → dev board `GND`
3. INMP441 `SCK` → dev board `GPIO 15`
4. INMP441 `WS` → dev board `GPIO 16`
5. INMP441 `SD` → dev board `GPIO 17`
6. INMP441 `L/R` → dev board `GND` (selects left channel)

After wiring, flash the firmware. From the UART shell:

```
po33> rec 0
po33> stoprec
```

You should see `Recorded N samples into slot 0`. If `N` is non-zero, the mic is working. If `N` is zero or the firmware crashes, check the wiring.

A quick sanity test: clap your hands near the mic while recording. The recorded buffer should contain non-zero samples (you can dump it later via `storage save` and inspect the `.bin` file with any audio editor like Audacity).

### 5.5 Step 5 — Wire the TFT (ILI9341)

This is the most wire-intensive step: 9 wires.

1. TFT `VCC` → dev board `3V3`
2. TFT `GND` → dev board `GND`
3. TFT `CS` → dev board `GPIO 5`
4. TFT `RST` → dev board `GPIO 48`
5. TFT `DC` → dev board `GPIO 4`
6. TFT `MOSI` (SDA) → dev board `GPIO 6`
7. TFT `SCK` (CLK) → dev board `GPIO 7`
8. TFT `LED` (BL) → dev board `GPIO 47`
9. TFT `MISO` → leave unconnected

After wiring, flash the firmware. On boot, the TFT should display a blue rectangle and the text "PO 33 K O" in white, then "ESP32 S3 FW" in green. If the screen is all white, the backlight is on but the SPI isn't initializing — check `CS`, `RST`, `DC`, `MOSI`, `SCK` wiring. If the screen is all black, the backlight isn't on — check the `LED`/`BL` pin.

### 5.6 Step 6 — Wire the button matrix (16 step buttons)

Eight wires from the dev board to the keypad (or 4×4 grid of switches).

| Dev board pin | Keypad / switch label | Step button |
|---|---|---|
| `GPIO 35` | Row 1 | steps 1, 2, 3, 4 |
| `GPIO 36` | Row 2 | steps 5, 6, 7, 8 |
| `GPIO 37` | Row 3 | steps 9, 10, 11, 12 |
| `GPIO 38` | Row 4 | steps 13, 14, 15, 16 |
| `GPIO 33` | Col 1 | (left column) |
| `GPIO 34` | Col 2 | |
| `GPIO 39` | Col 3 | |
| `GPIO 40` | Col 4 | (right column) |

### 5.6b Step 6b — Wire the 7 dedicated modifier buttons

Each is a single momentary tactile switch between one GPIO and GND. The firmware enables the internal pull-up — **no external resistor needed**. You can wire each as a separate lead, or run all 7 GNDs together on a single bus.

| Button | GPIO | Panel position |
|---|---|---|
| SOUND   | GPIO 11 | top row, leftmost |
| PATTERN | GPIO 44 | top row, middle |
| BPM     | GPIO 13 | top row, rightmost |
| REC     | GPIO 41 | right column, top |
| FX      | GPIO 12 | right column, 2nd |
| PLAY    | GPIO 42 | right column, 3rd |
| WRITE   | GPIO 43 | right column, bottom |

A clean way to wire all 23 buttons:

1. Mount the 16 step buttons on a single perfboard in a 4×4 grid.
2. Mount the 7 modifier buttons on the same perfboard in the PO-33 layout: **three across the top left** (SOUND, PATTERN, BPM) and **four down the right-hand side** (REC, FX, PLAY, WRITE), with the top of the column sitting under Knob B.
3. Add a single GND bus along one edge of the perfboard.
4. Row wires, column wires, and 7 modifier GPIO signals come off the perfboard to the dev board. GNDs all share one wire.

After wiring everything, flash the firmware. From the UART shell:

```
po33> free
```

You should see the heap stats. If the boot hangs, you probably have a short circuit between two GPIO pins or between a GPIO and ground. Use a multimeter in continuity mode to check.

The boot log should show: `Buttons ready: 4x4 matrix + 7 modifier GPIOs = 23 total`. If you see fewer, one of the GPIO wires is disconnected.

To test individual buttons, run the shell and press each one. The firmware doesn't echo button presses yet (that's a v2 feature), but `buttons_init()` succeeding is the smoke test.

### 5.6c Step 6c — Wire the two analog knobs

Each knob is a 3-wire device. The middle (wiper) terminal goes to the ADC pin; one end goes to 3.3 V, the other to GND.

| Knob | GPIO (ADC pin) | 3.3 V end | GND end |
|---|---|---|---|
| **Knob A** | **GPIO 20** | left terminal | right terminal |
| **Knob B** | **GPIO 46** | left terminal | right terminal |

(The "left vs. right" is arbitrary; pick either and the firmware works the same — `knobs_get_a()` returns 0 at the GND end and 255 at the 3.3 V end, and vice versa.)

After both knobs are wired, flash the firmware and test in the UART shell:

```
po33> free
```

The boot log should show: `Knobs ready (calibration: A=ok B=ok)`. If you see `calibration: A=fallback B=fallback`, the eFuse ADC calibration data wasn't found on your chip — the driver still works, just with slightly less accurate mV-to-12bit mapping.

To smoke-test the knobs without any UI binding yet: from the shell you can type a hypothetical future command like `knob A` and `knob B` (not yet implemented). For now, just verify that turning the knobs doesn't crash the firmware and the boot log shows the expected line.

### 5.7 Step 7 — (Optional) Wire the status LEDs

Two LEDs + two resistors. Each LED's + leg gets a 220 Ω resistor in series to its GPIO; the − leg goes to GND.

| Color | GPIO |
|---|---|
| Red ("REC") | GPIO 21 |
| Green ("PLAY") | GPIO 14 |

After wiring, the LEDs will light up when the firmware drives those pins (which is rare — only during active recording / playing). For a quick test, type `play` and `stop` in the shell and watch the green LED.

### 5.8 Step 8 — (Optional) Wire the speaker amp and speaker

This is the most expensive optional step and the one most likely to introduce noise if done wrong. Skip if you only want to use headphones.

Wire the MAX98357A as in §4.9. Connect the speaker to `OUT+` and `OUT-`. **In v1 firmware, the MAX98357A is not driven** — this is a v2 feature. For now, just leave the amp unpowered.

### 5.9 Step 9 — Wire the battery + charger (recommended)

This guide's default includes the **2800 mAh Li-ion cell** and TP4056 charger. The full power wiring is in §4.1; the runtime estimates are in §4.10.

1. Insert the **2800 mAh 18650 cell** into a single-cell holder (or solder to a JST-PH pigtail, observing polarity — red = +, black = −).
2. Connect the cell's + terminal to the TP4056's `B+` pad; − to `B−`. Connect `OUT+` → dev board `5V`; `OUT−` → dev board `GND`.
3. Connect the TP4056's `IN+` / `IN−` to a USB-C breakout (or to the dev board's USB-C 5 V line if you're OK with the module always being on).
4. **Before plugging the cell in, double-check polarity with a multimeter.** A reversed cell will damage the TP4056 and possibly vent the cell.
5. Power up. The TP4056's red LED should be on (charging) or green (charged). The dev board should boot normally.
6. Verify deep sleep current (optional): put a multimeter in series between the cell's + terminal and the TP4056's `B+` pad. Leave the device idle for 6 minutes (so the firmware enters deep sleep) and read the current — expect **~0.1 mA** (100 µA). Anything above ~1 mA suggests a wiring fault.

If you decide *not* to use the battery, skip this step and run on USB only — but expect to lose the deep-sleep standby runtime noted in §4.10.

### 5.10 Step 10 — Final assembly

Mount the parts in a box. Options:

- A small 3D-printed enclosure (STL files in the `enclosure/` directory of the project repo if they exist).
- A small plastic food container with holes drilled for the screen and buttons.
- A cardboard box, taped shut, with the buttons mounted on top.
- No enclosure at all — just a breadboard with the parts and wires visible. This is fine for development.

Connect USB-C. Press RESET. You should see the boot splash on the TFT, the UART prompt in your serial monitor, and the matrix-scanning boot log. **You're done.**

---

## 6. First-time power-up checklist

After §5 step 10, run through this short list once more.

- [ ] USB-C is plugged in (or a charged LiPo is connected).
- [ ] The dev board's `3V3` pin measures 3.3 V with a multimeter.
- [ ] The TFT backlight comes on at the configured brightness (70 % by default).
- [ ] The TFT shows the boot splash ("PO 33 K O" in white, "ESP32 S3 FW" in green).
- [ ] The serial monitor shows `=== ESP32-S3 PO-33 K.O! boot ===`.
- [ ] The serial monitor shows `AMY running @ 44100 Hz, block 256 frames`.
- [ ] Pressing any button does not crash the firmware. (You can verify by checking that the boot log doesn't re-print after a button press.)

If any of these checks fail, go to §7.

---

## 7. Troubleshooting

Most build problems fall into one of these categories.

### 7.1 No boot log on serial monitor

| Symptom | Likely cause | Fix |
|---|---|---|
| No output at all | USB cable is power-only | Try a different USB-C cable, one you know supports data. |
| No output at all | Wrong baud rate | Set your monitor to **115200** baud. |
| Garbled output | Wrong baud rate | Same. |
| Boot log prints but no `po33>` prompt | Firmware failed to initialize (probably I²S or TFT) | Read the log carefully; the last successful step before failure tells you which subsystem broke. |
| Boot log prints but no `po33>` prompt | Buttons task is wedged (rare) | Press RESET; if it works once, it was a transient. |

### 7.2 No sound from the DAC

| Symptom | Likely cause | Fix |
|---|---|---|
| No sound, but UART shell works | DAC not powered | Measure 3.3 V on PCM5102A `VIN`. |
| No sound, but UART shell works | DAC `FMT` pin in wrong mode | Tie `FMT` to GND for I²S. |
| No sound, but UART shell works | BCLK / LRCK swapped | Swap `BCK` and `LCK` wires on the dev board. |
| No sound, but UART shell works | Headphone plugged into wrong jack | Make sure you're in the PCM5102A's `OUT L/R` jack, not a different one. |
| Hiss but no music | I²S sample-rate mismatch (rare — AMY forces 44.1 kHz) | Check `I2S_SAMPLE_RATE` in `config.h`. |
| Buzzing sound | Missing ground between DAC and ESP32 | Verify `GND` is connected between both. |

### 7.3 No recording from the microphone

| Symptom | Likely cause | Fix |
|---|---|---|
| `rec 0` produces 0 samples | Mic not powered | Measure 3.3 V on INMP441 `VDD`. |
| `rec 0` produces 0 samples | BCLK / WS / SD swapped | Try swapping wires one at a time. |
| `rec 0` produces 0 samples | Mic `L/R` pin wrong | If `L/R` is floating, the INMP441 outputs on a random channel. Tie it to GND (left) or 3.3 V (right) — we use left. |
| Recorded audio is silent (zero PCM) but length is correct | Wrong I²S port config in firmware (very rare) | Open `main/audio/amy_bridge.c` and check `amy_config_t.capture_device_id` is set. |

### 7.4 TFT blank

| Symptom | Likely cause | Fix |
|---|---|---|
| All white | Backlight on, but SPI not initializing | Check `CS`, `RST`, `DC`, `MOSI`, `SCK`. The most common mistake is a swapped `MOSI` / `SCK`. |
| All black | Backlight off | Check the `LED`/`BL` pin. Measure voltage on the TFT's `LED` pin — should be ~3.3 V if GPIO 47 is configured as PWM output. |
| All black | Wrong display variant | If your TFT is the *parallel* ILI9341 (40-pin), not the *SPI* one (8-pin), this firmware won't drive it. Buy the SPI variant. |
| Garbled (random pixels) | SPI clock too fast | Reduce `TFT_SPI_CLK_HZ` in `config.h` from 40 MHz to 20 MHz. |

### 7.5 Buttons don't work

| Symptom | Likely cause | Fix |
|---|---|---|
| No button press registers | Matrix wires swapped | Re-check row vs. col wiring. |
| A specific button always reads pressed | Short circuit | Inspect with a multimeter in continuity mode. Look for solder bridges between adjacent pins. |
| Multiple buttons always read pressed | Two columns shorted | Same — check for shorts. |
| Buttons register as double-presses | Debounce time too short | Increase `BTN_DEBOUNCE_MS` in `config.h` from 30 to 60. |
| Button presses don't survive a reboot | Wrong matrix wiring (input vs. output swapped) | Verify rows are `gpio_set_direction(..., GPIO_MODE_OUTPUT)` and columns are inputs with pull-ups. The firmware does this automatically if the wiring is correct. |

### 7.6 Firmware won't compile

If you're building from source instead of using the web flasher:

- `idf.py set-target esp32s3` first.
- ESP-IDF version 5.0 or newer (we target v6.0 but v5.x usually works with one-line shims).
- `idf.py menuconfig` and verify PSRAM is enabled.
- The AMY sources must be vendored under `components/amy/src/`. See `components/amy/README.md`.

### 7.7 Still stuck

- Open an issue on the project repository with: (1) what step you're on, (2) what symptoms you see, (3) the full boot log from your serial monitor.
- Check the project's existing issues — chances are someone else had the same problem.
- If the firmware crashes on boot, attach the log *before* the crash, not after.

---

## 8. Modifying the design

### 8.1 Changing pin assignments

If your dev board has a different GPIO layout, edit `main/config.h` directly. Every pin used by the firmware is in this file as a `#define`. After editing, re-flash.

For example, to move the DAC's bit clock from GPIO 9 to GPIO 4:

```c
// Before
#define I2S_OUT_BCLK_GPIO  GPIO_NUM_9
// After
#define I2S_OUT_BCLK_GPIO  GPIO_NUM_4
```

Then:

```bash
idf.py build flash monitor
```

### 8.2 The "no-solder" build (Dupont wires only)

You can skip the soldering iron entirely by buying a dev board with **header pins already soldered** and using **Dupont jumper wires** (the rainbow ribbon cables on Amazon) to connect to the parts. Most breakout boards (PCM5102A, INMP441, ILI9341) come with header pins or pin holes that fit Dupont wires.

The build is the same as §5, except every wire is a removable Dupont cable instead of a soldered joint. This is **less robust** (Dupont cables work loose) but **faster** for a first build and **fully reversible** if you want to repurpose the parts.

### 8.3 Using a different ESP32-S3 module

If you have an ESP32-S3 module without Octal PSRAM (e.g. the N4 variant with 4 MB flash and no PSRAM), the firmware will fail at the `heap_caps_malloc(SAMPLE_POOL_SIZE_BYTES, MALLOC_CAP_SPIRAM)` call in `amy_bridge_init()`. Options:

- Buy an N16R8 module (recommended; $5–$10).
- Reduce `SAMPLE_TOTAL_SECONDS` from 40 to ~10 to fit in the much smaller heap (you'd lose most of the recording time).
- Add an external PSRAM chip to your board (advanced; not for beginners).

The firmware is designed around the assumption of **Octal PSRAM**. There is no graceful fallback.

### 8.4 Adding an external SD card

A microSD card adapter on SPI would let you store samples on a removable card instead of in flash. To do this:

- Pick three free GPIO pins (anywhere except the GPIOs already used for I²S, TFT, and buttons).
- Wire `CS`, `MOSI`, `MISO`, `SCK` to those pins.
- Add the FAT filesystem component (`espressif/esp_littlefs` or `espressif/fatfs`) to `main/idf_component.yml`.
- Modify `storage.c` to mount the SD card as the sample destination instead of the flash partition.

This is a v2 feature. The current firmware stores samples on the 16 MB flash chip via LittleFS, which works fine for the 40 s pool.

### 8.5 Replacing the AMY synth engine

If you want to use a different audio library (for example, you prefer FluidSynth or a custom DSP engine), the only file that talks to AMY directly is `main/audio/amy_bridge.c`. The rest of the firmware goes through `amy_bridge_play_note()`, `amy_bridge_register_slot()`, etc. — you can reimplement these in terms of your new engine without touching anything else.

The public API in `main/audio/amy_bridge.h` is the contract. Keep it stable and you can swap engines freely.

---

## 9. Where to go from here

- **Use the device.** Read [§5 of `docs/DESIGN.md`](../DESIGN.md#5-workflows-how-do-i) for "how do I record a sample" / "how do I make a beat" workflows.
- **Understand the firmware.** Read [`docs/DESIGN.md`](../DESIGN.md) — it maps every PO-33 feature to a piece of code with an honest ✅ / ⚠️ / ❌ status.
- **Understand the PO-33.** Read the [official manual](https://teenage.engineering/guides/po-33/en) — the original is short and readable.
- **Customize the firmware.** The simplest first customization: change the boot splash text. Look at `display_show_boot_screen()` in `main/ui/display.c`.
- **Report a problem.** Open an issue on the project repo with your boot log, schematic, and what you tried.
- **Contribute back.** If you write a feature, fix a bug, or build a beautiful enclosure, send a pull request.

---

*End of document. ~6,000 words. Source of truth for the firmware's pin map is [`main/config.h`](../../main/config.h); for the chip's specifications, [Espressif's ESP32-S3 product page](https://www.espressif.com/en/products/socs/esp32-s3). The schematic in §4 is intentionally text-only; if you'd like a KiCad or Fritzing source, the project repo welcomes contributions.*