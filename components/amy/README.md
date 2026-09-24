# AMY Component Wrapper

This directory wraps [AMY](https://github.com/shorepine/amy) (MIT-licensed
fixed-point music synthesizer library) as an ESP-IDF component.

## Vendoring the upstream sources

The actual AMY C/H sources are **not committed** to this repo. Fetch them
once after cloning:

```bash
# Option A — git submodule
git submodule add https://github.com/shorepine/amy.git components/amy-src
cp -r components/amy-src/src/* components/amy/src/
rm -rf components/amy-src

# Option B — shallow clone (no submodule)
git clone --depth 1 https://github.com/shorepine/amy.git /tmp/amy
cp -r /tmp/amy/src/* components/amy/src/
rm -rf /tmp/amy
```

After vendoring, `components/amy/src/` should contain (among others):
`amy.c amy.h pcm.c sequencer.c sequencer.h i2s.c patches.c algorithms.c
envelope.c filters.c oscillators.c interp_partials.c log2_exp2.c
transfer.c delay.c custom.c midi_mappings.c cv_trigger.c note_output.c
api.c libminiaudio-audio.c amy_midi.c amy_fixedpoint.h`.

The wrapper `CMakeLists.txt` in this directory adds the platform flags
needed for ESP32-S3 (Octal PSRAM, new I²S channel API, no baked-in PCM
banks).

## Version pinned

Target upstream version: **1.2.x main branch** as of vendoring time. The
public API surface we depend on (`amy_config_t`, `amy_event`, `amy_start`,
`amy_add_event`, `amy_set_external_input_buffer`, `amy_get_input_buffer`)
has been stable since 1.0.