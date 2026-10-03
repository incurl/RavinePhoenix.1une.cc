/*
 * clock.c — lightweight RTC clock using esp_timer offset stored in NVS.
 */
#include "clock.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sntp.h"
#include "audio/amy_bridge.h"
#include <stdio.h>

static const char *TAG = "clock";

static int64_t s_epoch_offset_us = 0;
static uint16_t s_alarm_hh = 0xFFFF;
static uint16_t s_alarm_mm = 0;
static uint8_t  s_alarm_slot = 0xFF;

/* F-034 alarm check-and-fire. s_last_alarm_day is the day-of-epoch
 * (days since 1970-01-01) the alarm last fired; we only fire once
 * per day per set alarm so the device doesn't loop the sample every
 * second. UE handles clearing the alarm or letting it fire again
 * tomorrow. */
static int64_t s_last_alarm_day = -1;
static esp_timer_handle_t s_alarm_timer = NULL;

static void alarm_check_tick(void *arg);
static void persist_offset(void);
static void persist_alarm(void);

esp_err_t clock_init(void)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open("clock", NVS_READONLY, &h);
    if (err == ESP_OK) {
        int64_t v = 0;
        nvs_get_i64(h, "epoch_us", &v);
        nvs_close(h);
        s_epoch_offset_us = v;
    } else {
        /* default: 2026-01-01 00:00:00 UTC = 1767225600 */
        s_epoch_offset_us = 1767225600LL * 1000000LL;
        persist_offset();
    }
    /* F-034 alarm: load the persisted HH:MM:slot if any, and start the
     * 1-second check-and-fire timer. The timer is always on -- if no
     * alarm is set (s_alarm_hh == 0xFFFF), the tick is a no-op. */
    if (nvs_open("clock", NVS_READONLY, &h) == ESP_OK) {
        uint16_t ah = 0xFFFF, am = 0;
        uint8_t as = 0xFF;
        if (nvs_get_u16(h, "alarm_hh", &ah) == ESP_OK) s_alarm_hh = ah;
        if (nvs_get_u16(h, "alarm_mm", &am) == ESP_OK) s_alarm_mm = am;
        if (nvs_get_u8(h, "alarm_slot", &as) == ESP_OK) s_alarm_slot = as;
        nvs_close(h);
    }
    const esp_timer_create_args_t cfg = {
        .name = "alarm_tick",
        .callback = alarm_check_tick,
        .arg = NULL,
        .skip_unhandled_events = true,
    };
    esp_timer_create(&cfg, &s_alarm_timer);
    esp_timer_start_periodic(s_alarm_timer, 1000000);  /* 1 s */

    ESP_LOGI(TAG, "Clock ready, epoch offset %lld us (alarm %02u:%02u slot %u)",
             s_epoch_offset_us, s_alarm_hh, s_alarm_mm, s_alarm_slot);
    return ESP_OK;
}

static void persist_alarm(void)
{
    nvs_handle_t h;
    if (nvs_open("clock", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u16(h, "alarm_hh", s_alarm_hh);
        nvs_set_u16(h, "alarm_mm", s_alarm_mm);
        nvs_set_u8(h, "alarm_slot", s_alarm_slot);
        nvs_commit(h);
        nvs_close(h);
    }
}

static void alarm_check_tick(void *arg)
{
    (void)arg;
    if (s_alarm_hh == 0xFFFF || s_alarm_slot == 0xFF) return;  /* no alarm set */

    uint16_t hh, mm;
    clock_get_hhmm(&hh, &mm);
    if (hh != s_alarm_hh || mm != s_alarm_mm) return;

    /* Within the matching minute -- fire if we haven't already today. */
    int64_t sec = (s_epoch_offset_us + esp_timer_get_time()) / 1000000;
    int64_t day = sec / 86400;
    if (day == s_last_alarm_day) return;

    s_last_alarm_day = day;
    ESP_LOGW(TAG, "ALARM firing: %02u:%02u slot %u", hh, mm, s_alarm_slot);
    /* Play the configured sample once. Middle-C, default velocity,
     * no FX, no tweak-mode filter -- a clean "wake up" tone. */
    amy_bridge_play_note(s_alarm_slot, 60, 100,
                         PO33_FX_NONE, 0, 0, 0, 0);
}

static void persist_offset(void)
{
    nvs_handle_t h;
    if (nvs_open("clock", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_i64(h, "epoch_us", s_epoch_offset_us);
        nvs_commit(h);
        nvs_close(h);
    }
}

static int64_t current_epoch_us(void)
{
    return s_epoch_offset_us + esp_timer_get_time();
}

void clock_get_hhmm(uint16_t *hh, uint16_t *mm)
{
    int64_t us = current_epoch_us();
    int64_t sec = us / 1000000;
    int64_t day = sec / 86400;
    int rem = (int)(sec % 86400);
    (void)day;
    *hh = (uint16_t)(rem / 3600);
    *mm = (uint16_t)((rem / 60) % 60);
}

void clock_set_hhmm(uint16_t hh, uint16_t mm)
{
    if (hh > 23 || mm > 59) return;
    int64_t now_us = esp_timer_get_time();
    int64_t cur_sec = (s_epoch_offset_us + now_us) / 1000000;
    int64_t cur_day = (cur_sec / 86400) * 86400;
    int64_t new_today_sec = cur_day + (int)hh * 3600 + (int)mm * 60;
    int64_t new_offset_us = new_today_sec * 1000000LL - now_us;
    s_epoch_offset_us = new_offset_us;
    persist_offset();
}

esp_err_t clock_set_alarm(uint16_t hh, uint16_t mm, uint8_t slot)
{
    if (hh > 23 || mm > 59 || slot >= 16) return ESP_ERR_INVALID_ARG;
    s_alarm_hh = hh;
    s_alarm_mm = mm;
    s_alarm_slot = slot;
    /* Reset the once-per-day guard so a newly-set alarm can fire even
     * if it was set right at HH:MM-1 today. */
    s_last_alarm_day = -1;
    persist_alarm();
    ESP_LOGI(TAG, "Alarm set %02u:%02u slot %u", hh, mm, slot);
    return ESP_OK;
}

void clock_clear_alarm(void)
{
    s_alarm_hh = 0xFFFF;
    s_alarm_mm = 0;
    s_alarm_slot = 0xFF;
    s_last_alarm_day = -1;
    persist_alarm();
    ESP_LOGI(TAG, "Alarm cleared");
}