/*
 * buttons.c — 4×4 matrix scan + 7 dedicated GPIO buttons.
 *
 *   The 4x4 matrix holds the 16 polymorphic step / slot / effect buttons.
 *   The 7 dedicated GPIOs hold the PO-33 modifier buttons (SOUND, PATTERN,
 *   BPM, REC, FX, PLAY, WRITE).
 *
 *   Both groups share the same debounce + short/long-press detection,
 *   and both push events into a single FreeRTOS queue.
 */
#include "buttons.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string.h>

static const char *TAG = "btns";

/* Matrix pins (16 step buttons). */
static const gpio_num_t s_rows[BTN_ROW_COUNT] = BTN_ROW_PINS;
static const gpio_num_t s_cols[BTN_COL_COUNT] = BTN_COL_PINS;

/* Dedicated GPIO buttons (7 modifier buttons, laid out PO-33 style).
 *
 * Order matches the physical layout:
 *   top row (left to right):    SOUND, PATTERN, BPM
 *   right column (top-to-bottom, under Knob B):
 *                               REC, FX, PLAY, WRITE
 */
typedef struct {
    gpio_num_t gpio;
    uint8_t    id;        /* logical BTN_* ID */
    bool       state;     /* current debounced state */
    int64_t    press_ms;  /* when the current press started */
    bool       long_fired;
} gpio_btn_t;

static gpio_btn_t s_gpio_btns[BTN_GPIO_COUNT] = {
    /* Top row */
    { BTN_SOUND_GPIO,   BTN_SOUND,   false, 0, false },
    { BTN_PATTERN_GPIO, BTN_PATTERN, false, 0, false },
    { BTN_BPM_GPIO,     BTN_BPM,     false, 0, false },
    /* Right column (under Knob B) */
    { BTN_REC_GPIO,     BTN_REC,     false, 0, false },
    { BTN_FX_GPIO,      BTN_FX,      false, 0, false },
    { BTN_PLAY_GPIO,    BTN_PLAY,    false, 0, false },
    { BTN_WRITE_GPIO,   BTN_WRITE,   false, 0, false },
};

/* Matrix state (16 step buttons). */
static bool    s_state[BTN_ROW_COUNT][BTN_COL_COUNT];
static int64_t s_press_time[BTN_ROW_COUNT][BTN_COL_COUNT];
static bool    s_long_fired[BTN_ROW_COUNT][BTN_COL_COUNT];

static QueueHandle_t s_queue = NULL;

static int64_t now_ms(void)
{
    return esp_timer_get_time() / 1000;
}

static void push_event(uint8_t btn_id, bool long_press)
{
    button_event_t ev = { .btn_id = btn_id, .long_press = long_press };
    xQueueSend(s_queue, &ev, 0);
}

esp_err_t buttons_init(void)
{
    /* --- Matrix: rows are outputs, columns are inputs with pull-ups --- */
    for (int r = 0; r < BTN_ROW_COUNT; r++) {
        gpio_reset_pin(s_rows[r]);
        gpio_set_direction(s_rows[r], GPIO_MODE_OUTPUT);
        gpio_set_level(s_rows[r], 1);   /* idle high */
    }
    for (int c = 0; c < BTN_COL_COUNT; c++) {
        gpio_reset_pin(s_cols[c]);
        gpio_set_direction(s_cols[c], GPIO_MODE_INPUT);
        gpio_pullup_en(s_cols[c]);
    }
    memset(s_state,     0, sizeof(s_state));
    memset(s_press_time, 0, sizeof(s_press_time));
    memset(s_long_fired, 0, sizeof(s_long_fired));

    /* --- 7 dedicated GPIO buttons: inputs with pull-ups --- */
    for (int i = 0; i < BTN_GPIO_COUNT; i++) {
        gpio_reset_pin(s_gpio_btns[i].gpio);
        gpio_set_direction(s_gpio_btns[i].gpio, GPIO_MODE_INPUT);
        gpio_pullup_en(s_gpio_btns[i].gpio);
        s_gpio_btns[i].state      = false;
        s_gpio_btns[i].press_ms   = 0;
        s_gpio_btns[i].long_fired = false;
    }

    s_queue = xQueueCreate(64, sizeof(button_event_t));
    if (!s_queue) return ESP_ERR_NO_MEM;

    ESP_LOGI(TAG, "Buttons ready: %dx%d matrix + %d modifier GPIOs = %d total",
             BTN_ROW_COUNT, BTN_COL_COUNT, BTN_GPIO_COUNT,
             BTN_ROW_COUNT * BTN_COL_COUNT + BTN_GPIO_COUNT);
    return ESP_OK;
}

/* Matrix button IDs start at BTN_STEP1. Row-major: btn = STEP1 + r*4 + c. */
static uint8_t matrix_btn_id(int r, int c)
{
    return (uint8_t)(BTN_STEP1 + r * BTN_COL_COUNT + c);
}

static void scan_gpio_buttons(void)
{
    int64_t t = now_ms();
    for (int i = 0; i < BTN_GPIO_COUNT; i++) {
        gpio_btn_t *b = &s_gpio_btns[i];
        bool pressed = (gpio_get_level(b->gpio) == 0);  /* active-low */
        if (pressed != b->state) {
            if (pressed) {
                b->press_ms   = t;
                b->long_fired = false;
            } else {
                if (!b->long_fired &&
                    (t - b->press_ms) >= BTN_DEBOUNCE_MS) {
                    push_event(b->id, false);
                }
            }
            b->state = pressed;
        } else if (pressed && !b->long_fired) {
            if ((t - b->press_ms) >= BTN_LONG_PRESS_MS) {
                b->long_fired = true;
                push_event(b->id, true);
            }
        }
    }
}

static void scan_matrix(void)
{
    int64_t t = now_ms();
    for (int r = 0; r < BTN_ROW_COUNT; r++) {
        gpio_set_level(s_rows[r], 0);
        for (int c = 0; c < BTN_COL_COUNT; c++) {
            int level = gpio_get_level(s_cols[c]);
            bool pressed = (level == 0);
            bool prev = s_state[r][c];
            if (pressed != prev) {
                if (pressed) {
                    s_press_time[r][c] = t;
                    s_long_fired[r][c]  = false;
                } else {
                    if (!s_long_fired[r][c] &&
                        (t - s_press_time[r][c]) >= BTN_DEBOUNCE_MS) {
                        push_event(matrix_btn_id(r, c), false);
                    }
                }
                s_state[r][c] = pressed;
            } else if (pressed && !s_long_fired[r][c]) {
                if ((t - s_press_time[r][c]) >= BTN_LONG_PRESS_MS) {
                    s_long_fired[r][c] = true;
                    push_event(matrix_btn_id(r, c), true);
                }
            }
        }
        gpio_set_level(s_rows[r], 1);
    }
}

void buttons_tick(void)
{
    scan_matrix();
    scan_gpio_buttons();
}

button_event_t buttons_pop(void)
{
    button_event_t ev = { .btn_id = 0xFF, .long_press = false };
    if (s_queue) xQueueReceive(s_queue, &ev, 0);
    return ev;
}

bool buttons_is_pressed(uint8_t btn_id)
{
    /* Modifier buttons (0..6) are tracked individually. */
    if (btn_id < BTN_GPIO_COUNT) {
        return s_gpio_btns[btn_id].state;
    }
    /* Step buttons live in the matrix at row-major (r*4+c). */
    if (BTN_IS_STEP(btn_id)) {
        int idx = btn_id - BTN_STEP1;
        int r = idx / BTN_COL_COUNT;
        int c = idx % BTN_COL_COUNT;
        if (r >= 0 && r < BTN_ROW_COUNT && c >= 0 && c < BTN_COL_COUNT) {
            return s_state[r][c];
        }
    }
    return false;
}