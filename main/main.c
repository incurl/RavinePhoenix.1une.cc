/*
 * main.c — ESP32-S3 PO-33 K.O! firmware entry point
 *
 * Wires up: I2S, audio engine, sample manager, sequencer, effects,
 *           UI (buttons + TFT), storage, power management, sync, clock.
 */
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_idf_version.h"
#include "nvs_flash.h"

#include "config.h"
#include "audio/amy_bridge.h"
#include "sequencer/sequencer.h"
#include "ui/buttons.h"
#include "ui/display.h"
#include "ui/encoder.h"
#include "ui/knobs.h"
#include "ui/leds.h"
#include "ui/input.h"
#include "storage/storage.h"
#include "system/power_mgmt.h"
#include "system/clock.h"
#include "system/sync.h"

static const char *TAG = "po33";

static void button_scan_task(void *arg);
static void display_task(void *arg);
static void po33_shell_task(void *arg);

static TaskHandle_t s_button_task_handle = NULL;
static TaskHandle_t s_display_task_handle = NULL;

void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP32-S3 PO-33 K.O! boot ===");
    ESP_LOGI(TAG, "ESP-IDF %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Internal free: %u B",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "PSRAM free:    %u B",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    /* 1. NVS */
    ESP_ERROR_CHECK(nvs_flash_init());

    /* 2. Storage */
    ESP_ERROR_CHECK(storage_init());
    ESP_ERROR_CHECK(storage_mount());

    /* 3. Power management early */
    power_mgmt_init();

    /* 4. Audio (all handled by AMY) */
    ESP_ERROR_CHECK(amy_bridge_init());

    /* 5. Sequencer */
    sequencer_init();

    /* 6. Sync, clock, LEDs */
    sync_init();
    clock_init();
    leds_init();

    /* 7. UI */
    ESP_ERROR_CHECK(display_init());
    ESP_ERROR_CHECK(buttons_init());
    ESP_ERROR_CHECK(knobs_init());
    /* Encoder is REQUIRED -- the sketch picker relies on it for
     * scroll. If PCNT unit 0 can't be allocated we halt boot. */
    ESP_ERROR_CHECK(encoder_init());

    /* 8. Boot splash */
    display_show_boot_screen();

    /* 9. Spawn tasks. AMY runs its own render task on core 1. */
    BaseType_t ok;

    ok = xTaskCreate(button_scan_task, "button_scan",
                     4096, NULL, 10, &s_button_task_handle);
    configASSERT(ok == pdPASS);

    ok = xTaskCreate(display_task, "display",
                     4096, NULL, 5, &s_display_task_handle);
    configASSERT(ok == pdPASS);

    ok = xTaskCreate(po33_shell_task, "shell",
                     4096, NULL, 3, NULL);
    configASSERT(ok == pdPASS);

    ESP_LOGI(TAG, "Boot complete.");
}

/* ─── Button scan ─────────────────────────────────────────────── */
static void button_scan_task(void *arg)
{
    (void)arg;
    const TickType_t period = pdMS_TO_TICKS(10);
    TickType_t last = xTaskGetTickCount();

    while (1) {
        buttons_tick();
        input_drain();
        knobs_tick();
        encoder_tick();
        amy_bridge_pump_capture();
        vTaskDelayUntil(&last, period);
    }
}

/* ─── Display refresh ─────────────────────────────────────────── */
static void display_task(void *arg)
{
    (void)arg;
    const TickType_t period = pdMS_TO_TICKS(50);
    TickType_t last = xTaskGetTickCount();

    while (1) {
        display_tick();
        vTaskDelayUntil(&last, period);
    }
}

/* ─── UART shell ──────────────────────────────────────────────── */
static void po33_shell_help(void)
{
    printf("Commands:\n"
           "  help              show this help\n"
           "  status            print state\n"
           "  play              start sequencer\n"
           "  stop              stop sequencer\n"
           "  bpm <60..240>     set tempo\n"
           "  rec <0..15>       record into slot\n"
           "  stoprec           stop recording\n"
           "  pattern <0..15>   select pattern\n"
           "  free              show free heap\n"
           "  save              save sketches + samples (v1)\n"
           "  load              reload from flash (v1)\n"
           "  sketch new        save active as a new sketch (v2)\n"
           "  sketch save       same as `sketch new` (alias)\n"
           "  sketch load <id>  load sketch by 4-hex id (v2)\n"
           "  sketch del <id>   delete sketch by 4-hex id (v2)\n"
           "  sketch list       list sketches with count and ids (v2)\n"
           "  sleep             enter deep sleep\n");
}

static void po33_shell_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "Shell ready. Type 'help'.");
    char line[128];
    int idx = 0;

    while (1) {
        int c = fgetc(stdin);
        if (c == EOF) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        if (c == '\r' || c == '\n') {
            line[idx] = '\0';
            idx = 0;
            if (line[0] == '\0') continue;

            if (strcmp(line, "help") == 0) {
                po33_shell_help();
            } else if (strcmp(line, "status") == 0) {
                sequencer_print_status();
            } else if (strcmp(line, "play") == 0) {
                sequencer_play();
            } else if (strcmp(line, "stop") == 0) {
                sequencer_stop();
            } else if (strcmp(line, "free") == 0) {
                printf("internal=%u psram=%u\n",
                       (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                       (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
            } else if (strcmp(line, "save") == 0) {
                storage_save_all();
            } else if (strcmp(line, "load") == 0) {
                storage_load_all();
            } else if (strcmp(line, "sleep") == 0) {
                power_mgmt_enter_deep_sleep();
            } else if (strncmp(line, "bpm ", 4) == 0) {
                int bpm = atoi(line + 4);
                if (bpm >= MIN_BPM && bpm <= MAX_BPM) {
                    sequencer_set_bpm((uint16_t)bpm);
                    printf("BPM=%d\n", bpm);
                } else {
                    printf("invalid BPM\n");
                }
            } else if (strncmp(line, "rec ", 4) == 0) {
                int s = atoi(line + 4);
                if (s >= 0 && s < SLOT_COUNT) {
                    amy_bridge_start_record((uint8_t)s);
                    printf("recording into slot %d\n", s);
                } else {
                    printf("invalid slot\n");
                }
            } else if (strcmp(line, "stoprec") == 0) {
                amy_bridge_stop_record();
                printf("recording stopped\n");
            } else if (strncmp(line, "pattern ", 8) == 0) {
                int p = atoi(line + 8);
                if (p >= 0 && p < PATTERN_COUNT) {
                    sequencer_set_pattern((uint8_t)p);
                    printf("pattern=%d\n", p);
                } else {
                    printf("invalid pattern\n");
                }
            } else if (strcmp(line, "sketch new") == 0 ||
                       strcmp(line, "sketch save") == 0) {
                esp_err_t e = storage_sketch_save_active();
                if (e == ESP_OK) printf("sketch saved\n");
                else            printf("sketch save failed: %s\n",
                                        esp_err_to_name(e));
            } else if (strncmp(line, "sketch load ", 11) == 0) {
                const char *id = line + 11;
                /* Allow "sketch load 0001" -- require a 4-hex id. */
                if (strlen(id) != 4) {
                    printf("usage: sketch load <4-hex id>\n");
                } else {
                    esp_err_t e = storage_sketch_load(id);
                    if (e == ESP_OK) printf("sketch %s loaded\n", id);
                    else            printf("sketch load failed: %s\n",
                                            esp_err_to_name(e));
                }
            } else if (strncmp(line, "sketch del ", 10) == 0) {
                const char *id = line + 10;
                if (strlen(id) != 4) {
                    printf("usage: sketch del <4-hex id>\n");
                } else {
                    esp_err_t e = storage_sketch_delete(id);
                    if (e == ESP_OK) printf("sketch %s deleted\n", id);
                    else            printf("sketch del failed: %s\n",
                                            esp_err_to_name(e));
                }
            } else if (strcmp(line, "sketch list") == 0) {
                uint8_t n = 0;
                char ids[SKETCHES_MAX][SKETCH_ID_LEN + 1];
                esp_err_t e = storage_sketch_list(&n, ids);
                if (e != ESP_OK) {
                    printf("sketch list failed: %s\n", esp_err_to_name(e));
                } else {
                    printf("count=%u\n", (unsigned)n);
                    for (int i = 0; i < n; i++) {
                        printf("  %s\n", ids[i]);
                    }
                }
            } else {
                printf("unknown: '%s'\n", line);
            }
            continue;
        }
        if (idx < (int)sizeof(line) - 1) {
            line[idx++] = (char)c;
        }
    }
}