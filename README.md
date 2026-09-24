# ESP32-S3 PO-33 K.O! — Pocket Operator Emulator Firmware

A complete firmware project that emulates the **Teenage Engineering PO-33 K.O!** micro-sampler on an **ESP32-S3-WROOM-1-N16R8** development board.

> **Status:** Targets **ESP-IDF v6.0** APIs. v6.0 is not yet GA as of writing — the code is written against the v5.x-shaped API that v6.0 inherits, with v6.0-specific changes (channel API only, opaquer driver handles, no legacy I2S path). If you build against v5.x you may need to substitute `MALLOC_CAP_SPIRAM` (already correct) and ensure your toolchain provides the `esp_lcd_ili9341` component.

---

## ✨ Features

| Spec | Implementation |
|---|---|
| Sample memory | **60 s mono** @ 22050 Hz 16-bit = **~2.6 MB PSRAM pool** |
| Sample slots | **16** (8 drum ≤ 3 s, 8 melodic ≤ 4.5 s) |
| Polyphony | 4 voices, zero-copy from PSRAM, linear-interpolation resampling |
| Recording | External I²S mic (INMP441) → live monitor + overdub |
| Trimming / slicing | Per-slot start/end, auto-slice to N parts |
| Sequencer | 16-step × **16 patterns**, 128-pattern chain mode |
| Parameter locks | 2 per step |
| Punch-in effects | 16 (loop×4, unison×2, octave×2, stutter×3, scratch, reverse, retrigger, 6/8, filter sweep, bitcrush) |
| Display | **2.4″ TFT (ILI9341, 240×320, SPI)**, custom 2D draw API, PSRAM framebuffer |
| Sync | Jam-sync pulse output on GPIO, listen on separate GPIO |
| Storage | **LittleFS** on 16 MB flash (6 MB samples + 1 MB patterns) |
| Power | 5-min idle → deep sleep, GPIO-wake |
| Clock / alarm | RTC-backed, NVS-persisted |
| Shell | UART @ 115 200 baud command interface |

---

## 🔌 Wiring

### Audio I/O (PCM5102A DAC + INMP441 mic)

| Signal | ESP32-S3 GPIO |
|---|---|
| DAC BCLK | GPIO9 |
| DAC LRCK | GPIO10 |
| DAC DIN  | GPIO8  |
| Mic BCLK | GPIO15 |
| Mic LRCK | GPIO16 |
| Mic DOUT | GPIO17 |

### 4x4 Button Matrix

A 4-row by 4-column scanned matrix (16 buttons total):

| Row / Col | GPIO |
|---|---|
| Row 0 | GPIO35 |
| Row 1 | GPIO36 |
| Row 2 | GPIO37 |
| Row 3 | GPIO38 |
| Col 0 | GPIO33 |
| Col 1 | GPIO34 |
| Col 2 | GPIO39 |
| Col 3 | GPIO40 |

Buttons are wired at the intersections of each row and column, momentary-to-GND. Internal pull-ups are enabled on the column inputs. Logical button IDs follow row-major order: `btn_id = row * 4 + col` (so btn 0 = R0C0, btn 15 = R3C3).

**Mapping** (matches `BTN_*` enum in `main/config.h`):

| ID | Function | ID | Function |
|---|---|---|---|
| 0  | REC      | 8  | STEP7    |
| 1  | PLAY     | 9  | STEP8    |
| 2  | STEP1    | 10 | FUNC     |
| 3  | STEP2    | 11 | FX       |
| 4  | STEP3    | 12 | BPM_UP   |
| 5  | STEP4    | 13 | BPM_DN   |
| 6  | STEP5    | 14 | PAT_UP   |
| 7  | STEP6    | 15 | PAT_DN   |

### 2.4″ TFT (ILI9341, SPI)

| Signal | ESP32-S3 GPIO |
|---|---|
| MOSI | GPIO6 |
| SCK  | GPIO7 |
| CS   | GPIO5 |
| DC   | GPIO4 |
| RST  | GPIO48 |
| BL   | GPIO47 (LEDC PWM) |

### Other

| Function | GPIO |
|---|---|
| Sync OUT | GPIO18 |
| Sync IN  | GPIO19 |
| LED REC  | GPIO21 |
| LED PLAY | GPIO14 |
| Battery ADC | GPIO4 (ADC1 CH3) — see `config.h` |

---

## 🏗️ Build

```bash
# 1. Get ESP-IDF v6.0 (or current RC)
git clone --recursive https://github.com/espressif/esp-idf.git -b v6.0
cd esp-idf && ./install.sh esp32s3 && source export.sh

# 2. From project root:
idf.py set-target esp32s3
idf.py menuconfig   # verify PSRAM, 240 MHz, I2S new API
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Required `menuconfig` confirmations (mostly pre-set in `sdkconfig.defaults`):

- `Serial flasher config → Flash size = 16 MB`
- `Component config → ESP PSRAM → Mode = Octal`
- `Component config → ESP PSRAM → Speed = 80 MHz`
- `Component config → Driver configurations → I2S → Use new channel API only`

---

## 🗂️ Project Structure

```
.
├── CMakeLists.txt
├── sdkconfig.defaults
├── partitions.csv
├── main/
│   ├── CMakeLists.txt
│   ├── idf_component.yml
│   ├── main.c
│   ├── config.h
│   ├── audio/
│   │   ├── audio_engine.{c,h}
│   │   ├── i2s_driver.{c,h}
│   │   ├── sample_manager.{c,h}
│   │   └── voice.{c,h}
│   ├── sequencer/
│   │   ├── pattern.{c,h}
│   │   └── sequencer.{c,h}
│   ├── effects/
│   │   ├── effects.{c,h}
│   │   ├── stutter.{c,h}
│   │   ├── reverse.{c,h}
│   │   └── bitcrush.{c,h}
│   ├── ui/
│   │   ├── buttons.{c,h}
│   │   ├── display.{c,h}
│   │   └── leds.{c,h}
│   ├── storage/
│   │   └── storage.{c,h}
│   ├── system/
│   │   ├── power_mgmt.{c,h}
│   │   ├── clock.{c,h}
│   │   └── sync.{c,h}
│   └── tests/
│       ├── test_sequencer.c
│       └── test_patterns.c
└── components/         # (placeholder for vendored libraries)
```

---

## 🧪 Tests

```bash
idf.py -C build test    # runs Unity tests under main/tests/
```

---

## 📟 UART Shell

Connect at **115 200 baud** and press <kbd>Enter</kbd>:

```
help              # show all commands
play              # start
bpm 140           # set tempo
pattern 5         # choose pattern
rec 2             # record into slot 2
free              # heap stats
save              # write samples + patterns to flash
load              # reload from flash
sleep             # enter deep sleep immediately
```

---

## 📊 Memory budget (post-init, 60 s mono pool)

| Consumer | Size |
|---|---|
| 60 s audio pool (PSRAM) | 2.65 MB |
| 2.4″ TFT framebuffer | 150 KB |
| DSP working memory | ~1 MB |
| esp_audio_effects tails | ~512 KB |
| Misc / heap overhead | ~512 KB |
| **PSRAM headroom** | **~3.25 MB free** |

---

## ⚠️ Known Limitations / Roadmap

- v6.0 is not GA; some APIs may need a one-line rename when the toolchain goes final.
- LittleFS save currently dumps the entire sample pool into one file (slot 0) — fine for v1, but per-slot files coming.
- No touch input on the TFT yet (resistive touch via XPT2046 on a separate SPI bus is straightforward to add).
- Effect library `esp_audio_effects` is referenced in `idf_component.yml`. If your component mirror doesn't have it, vendor it locally under `components/`.
- Power consumption on deep sleep ~ 10 µA — measured in v5.x, expected same on v6.0.

---

## 📝 License

See `LICENSE`.