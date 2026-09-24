/*
 * i2s_driver.c — full-duplex I2S using ESP-IDF v6.0 channel API.
 *
 *   I2S_NUM_0 → TX → external DAC (PCM5102A)
 *   I2S_NUM_1 → RX → external mic ADC (INMP441)
 */
#include "i2s_driver.h"
#include "config.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"

static const char *TAG = "i2s";
static i2s_chan_handle_t s_tx_handle = NULL;
static i2s_chan_handle_t s_rx_handle = NULL;

esp_err_t audio_i2s_init(void)
{
    ESP_LOGI(TAG, "Initializing I2S0 TX + I2S1 RX @ %d Hz", I2S_SAMPLE_RATE);

    /* ── TX channel ─────────────────────────────────────────── */
    i2s_chan_config_t tx_cfg = {
        .id            = I2S_OUT_PORT,
        .role          = I2S_ROLE_MASTER,
        .dma_desc_num  = I2S_DMA_BUF_COUNT,
        .dma_frame_num = I2S_DMA_BUF_LEN,
        .auto_clear    = true,
    };
    ESP_ERROR_CHECK(i2s_new_channel(&tx_cfg, &s_tx_handle, NULL));

    i2s_std_config_t tx_std = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                       I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_OUT_BCLK_GPIO,
            .ws   = I2S_OUT_LRCK_GPIO,
            .dout = I2S_OUT_DATA_GPIO,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    /* mono source → stereo frame */
    tx_std.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(s_tx_handle, &tx_std));
    ESP_ERROR_CHECK(i2s_channel_enable(s_tx_handle));

    /* ── RX channel ─────────────────────────────────────────── */
    i2s_chan_config_t rx_cfg = {
        .id            = I2S_IN_PORT,
        .role          = I2S_ROLE_MASTER,
        .dma_desc_num  = I2S_DMA_BUF_COUNT,
        .dma_frame_num = I2S_DMA_BUF_LEN,
        .auto_clear    = false,
    };
    ESP_ERROR_CHECK(i2s_new_channel(&rx_cfg, NULL, &s_rx_handle));

    i2s_std_config_t rx_std = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                       I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_IN_BCLK_GPIO,
            .ws   = I2S_IN_LRCK_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din  = I2S_IN_DATA_GPIO,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    rx_std.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;   /* INMP441 left channel */
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(s_rx_handle, &rx_std));
    ESP_ERROR_CHECK(i2s_channel_enable(s_rx_handle));

    ESP_LOGI(TAG, "I2S ready.");
    return ESP_OK;
}

void audio_i2s_deinit(void)
{
    if (s_tx_handle) {
        i2s_channel_disable(s_tx_handle);
        i2s_del_channel(s_tx_handle);
        s_tx_handle = NULL;
    }
    if (s_rx_handle) {
        i2s_channel_disable(s_rx_handle);
        i2s_del_channel(s_rx_handle);
        s_rx_handle = NULL;
    }
}

size_t audio_i2s_write(const int16_t *samples, size_t num_frames)
{
    if (!s_tx_handle || !samples || num_frames == 0) return 0;

    /* Duplicate mono into L=R stereo frames in-place into a small PSRAM
     * scratch buffer. For very small blocks (<512 frames) we use stack/heap. */
    static int16_t *scratch = NULL;
    static size_t   scratch_cap = 0;
    if (num_frames * 2 > scratch_cap) {
        if (scratch) free(scratch);
        scratch_cap = num_frames * 2;
        scratch = heap_caps_malloc(scratch_cap * sizeof(int16_t),
                                   MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (!scratch) return 0;
    }
    for (size_t i = 0; i < num_frames; i++) {
        scratch[2 * i + 0] = samples[i];
        scratch[2 * i + 1] = samples[i];
    }

    size_t bytes_written = 0;
    esp_err_t err = i2s_channel_write(s_tx_handle, scratch,
                                      num_frames * 2 * sizeof(int16_t),
                                      &bytes_written, pdMS_TO_TICKS(50));
    if (err != ESP_OK) return 0;
    return bytes_written / (sizeof(int16_t) * 2);   /* frames */
}

size_t audio_i2s_read(int16_t *out, size_t max_frames)
{
    if (!s_rx_handle || !out || max_frames == 0) return 0;

    int16_t stereo[256];
    size_t total = 0;
    while (total < max_frames) {
        size_t bytes_read = 0;
        size_t want = (max_frames - total) > 128 ? 128 : (max_frames - total);
        esp_err_t err = i2s_channel_read(s_rx_handle, stereo,
                                         want * 2 * sizeof(int16_t),
                                         &bytes_read, pdMS_TO_TICKS(20));
        if (err != ESP_OK || bytes_read == 0) break;
        size_t frames = bytes_read / (sizeof(int16_t) * 2);
        for (size_t i = 0; i < frames && total < max_frames; i++) {
            out[total++] = stereo[2 * i];   /* take left channel */
        }
    }
    return total;
}