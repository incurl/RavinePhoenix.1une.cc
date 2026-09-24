# Firmware binaries

The `.bin` files in this directory are **placeholders**. Flashing them
will fail. To produce real firmware:

1. Build the firmware with ESP-IDF v6.0:
   ```bash
   idf.py set-target esp32s3
   idf.py build
   ```
2. Copy the four output files here:
   ```bash
   cp build/bootloader/bootloader.bin                     ./bootloader.bin
   cp build/partition_table/partition-table.bin           ./partitions.bin
   cp build/boot_app0.bin                                ./boot_app0.bin
   cp build/po33.bin                                     ./firmware.bin
   ```
3. Commit and push. The GitHub Pages workflow will redeploy.

The `manifest.json` references these files with the correct
ESP32-S3 flash offsets:
- bootloader → 0x1000
- partitions → 0x8000
- boot_app0  → 0xE000
- firmware   → 0x10000