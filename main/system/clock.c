/*
 * clock.c — lightweight RTC clock using esp_timer offset stored in NVS.
 */
#include "clock.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sntp.h"
#include <stdio.h>

static const char *TAG = "clock";

static int64_t s_epoch_offset_us = 0;
static uint16_t s_alarm_hh = 0xFFFF;
static uint16_t s_alarm_mm = 0;
static uint8_t  s_alarm_slot = 0xFF;

static void persist_offset(void);

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
    ESP_LOGI(TAG, "Clock ready, epoch offset %lld us", s_epoch_offset_us);
    return ESP_OK;
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
    ESP_LOGI(TAG, "Alarm set %02u:%02u slot %u", hh, mm, slot);
    return ESP_OK;
}