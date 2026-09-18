#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tusb.h"
#include "usb_device_uac.h"

#define SAMPLE_RATE_HZ 48000
#define BUTTON_K1_GPIO GPIO_NUM_3
#define BUTTON_K2_GPIO GPIO_NUM_2
#define BUTTON_K3_GPIO GPIO_NUM_1
#define MIC_BCLK_GPIO GPIO_NUM_8
#define MIC_WS_GPIO GPIO_NUM_9
#define MIC_DATA_GPIO GPIO_NUM_7
#define USER_LED_GPIO GPIO_NUM_21
#define HOTKEY_MODIFIERS (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTGUI)

static const char *TAG = "voice_keyboard";
static i2s_chan_handle_t mic_rx_channel;
// ESP32-S3 standard-mode RX is most reliable in stereo/BOTH mode.  INMP441
// drives the left slot (L/R tied to GND), so each USB mono sample consumes one
// interleaved L/R frame and only the left word is used.
static int32_t raw_samples[1024];
static int32_t dc_estimate;

static esp_err_t microphone_input(uint8_t *buffer, size_t length,
                                  size_t *bytes_read, void *context)
{
    (void)context;
    int16_t *output = (int16_t *)buffer;
    const size_t requested_samples = length / sizeof(int16_t);
    const size_t capped_samples = requested_samples > 512 ? 512 : requested_samples;
    size_t raw_bytes = 0;

    esp_err_t error = i2s_channel_read(mic_rx_channel, raw_samples,
                                       capped_samples * 2 * sizeof(int32_t),
                                       &raw_bytes, pdMS_TO_TICKS(30));
    const size_t received_samples = raw_bytes / (2 * sizeof(int32_t));

    for (size_t i = 0; i < received_samples; ++i) {
        int32_t sample = raw_samples[i * 2];
        dc_estimate += (sample - dc_estimate) >> 9;
        sample -= dc_estimate;

        // INMP441 provides 24-bit signed samples left-aligned in a 32-bit slot.
        // Shift by 14 instead of 16 for modest digital gain, then saturate.
        sample >>= 14;
        if (sample > INT16_MAX) {
            sample = INT16_MAX;
        } else if (sample < INT16_MIN) {
            sample = INT16_MIN;
        }
        output[i] = (int16_t)sample;
    }

    if (received_samples < requested_samples) {
        memset(output + received_samples, 0,
               (requested_samples - received_samples) * sizeof(int16_t));
    }

    *bytes_read = length;
    return error == ESP_OK || error == ESP_ERR_TIMEOUT ? ESP_OK : error;
}

static void initialize_microphone(void)
{
    i2s_chan_config_t channel_config =
        I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    channel_config.dma_desc_num = 8;
    channel_config.dma_frame_num = 240;

    ESP_ERROR_CHECK(i2s_new_channel(&channel_config, NULL, &mic_rx_channel));

    i2s_std_config_t standard_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = MIC_BCLK_GPIO,
            .ws = MIC_WS_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din = MIC_DATA_GPIO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(mic_rx_channel, &standard_config));
    ESP_ERROR_CHECK(i2s_channel_enable(mic_rx_channel));

    // Discard startup samples once. The channel then stays enabled permanently,
    // so later voice sessions always receive fresh DMA data instead of zeros.
    size_t discarded = 0;
    (void)i2s_channel_read(mic_rx_channel, raw_samples, sizeof(raw_samples),
                           &discarded, pdMS_TO_TICKS(30));
}

static bool send_keyboard_report(uint8_t modifiers, uint8_t keycode)
{
    if (!tud_mounted() || !tud_hid_ready()) {
        return false;
    }

    uint8_t keycodes[6] = {keycode, 0, 0, 0, 0, 0};
    return tud_hid_keyboard_report(0, modifiers, keycodes);
}

typedef enum {
    HOTKEY_IDLE,
    HOTKEY_PRESS_CTRL,
    HOTKEY_PRESS_BOTH,
    HOTKEY_HELD,
    HOTKEY_RELEASE_GUI,
    HOTKEY_RELEASE_ALL,
} hotkey_state_t;

typedef enum {
    ACTION_NONE,
    ACTION_F13_DOWN,
    ACTION_F13_UP,
    ACTION_ENTER_DOWN,
    ACTION_ENTER_UP,
} keyboard_action_t;

typedef struct {
    gpio_num_t gpio;
    bool stable_pressed;
    bool last_sample;
    TickType_t changed_at;
} debounced_button_t;

static int update_button(debounced_button_t *button, TickType_t now)
{
    const bool sample_pressed = gpio_get_level(button->gpio) == 1;
    if (sample_pressed != button->last_sample) {
        button->last_sample = sample_pressed;
        button->changed_at = now;
    }
    if (sample_pressed != button->stable_pressed &&
        now - button->changed_at >= pdMS_TO_TICKS(20)) {
        button->stable_pressed = sample_pressed;
        return sample_pressed ? 1 : -1;
    }
    return 0;
}

static void button_task(void *argument)
{
    (void)argument;
    debounced_button_t k1 = {.gpio = BUTTON_K1_GPIO};
    debounced_button_t k2 = {.gpio = BUTTON_K2_GPIO};
    debounced_button_t k3 = {.gpio = BUTTON_K3_GPIO};
    hotkey_state_t hotkey_state = HOTKEY_IDLE;
    keyboard_action_t action = ACTION_NONE;

    while (true) {
        const TickType_t now = xTaskGetTickCount();
        const int k1_edge = update_button(&k1, now);
        const int k2_edge = update_button(&k2, now);
        const int k3_edge = update_button(&k3, now);

        if (k1_edge > 0 && hotkey_state == HOTKEY_IDLE && action == ACTION_NONE) {
            action = ACTION_F13_DOWN;
        }
        if (k2_edge != 0) {
            gpio_set_level(USER_LED_GPIO, k2.stable_pressed ? 0 : 1);
            hotkey_state = k2.stable_pressed ? HOTKEY_PRESS_CTRL
                                             : HOTKEY_RELEASE_GUI;
        }
        if (k3_edge > 0 && hotkey_state == HOTKEY_IDLE && action == ACTION_NONE) {
            action = ACTION_ENTER_DOWN;
        }

        if (!tud_mounted()) {
            hotkey_state = k2.stable_pressed ? HOTKEY_PRESS_CTRL : HOTKEY_IDLE;
            action = ACTION_NONE;
        } else if (action != ACTION_NONE) {
            switch (action) {
            case ACTION_F13_DOWN:
                if (send_keyboard_report(0, HID_KEY_F13)) action = ACTION_F13_UP;
                break;
            case ACTION_F13_UP:
                if (send_keyboard_report(0, 0)) action = ACTION_NONE;
                break;
            case ACTION_ENTER_DOWN:
                if (send_keyboard_report(0, HID_KEY_ENTER)) action = ACTION_ENTER_UP;
                break;
            case ACTION_ENTER_UP:
                if (send_keyboard_report(0, 0)) action = ACTION_NONE;
                break;
            case ACTION_NONE:
                break;
            }
        } else {
            switch (hotkey_state) {
            case HOTKEY_PRESS_CTRL:
                if (send_keyboard_report(KEYBOARD_MODIFIER_LEFTCTRL, 0)) {
                    hotkey_state = HOTKEY_PRESS_BOTH;
                }
                break;
            case HOTKEY_PRESS_BOTH:
                if (send_keyboard_report(HOTKEY_MODIFIERS, 0)) {
                    hotkey_state = HOTKEY_HELD;
                }
                break;
            case HOTKEY_RELEASE_GUI:
                if (send_keyboard_report(KEYBOARD_MODIFIER_LEFTCTRL, 0)) {
                    hotkey_state = HOTKEY_RELEASE_ALL;
                }
                break;
            case HOTKEY_RELEASE_ALL:
                if (send_keyboard_report(0, 0)) {
                    hotkey_state = HOTKEY_IDLE;
                }
                break;
            case HOTKEY_IDLE:
            case HOTKEY_HELD:
                break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void app_main(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_K1_GPIO) |
                        (1ULL << BUTTON_K2_GPIO) |
                        (1ULL << BUTTON_K3_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&button_config));

    gpio_config_t led_config = {
        .pin_bit_mask = 1ULL << USER_LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&led_config));
    gpio_set_level(USER_LED_GPIO, 1);

    initialize_microphone();

    uac_device_config_t usb_audio_config = {
        .skip_tinyusb_init = false,
        .output_cb = NULL,
        .input_cb = microphone_input,
        .set_mute_cb = NULL,
        .set_volume_cb = NULL,
        .cb_ctx = NULL,
        .spk_itf_num = -1,
        .mic_itf_num = 1,
    };
    ESP_ERROR_CHECK(uac_device_init(&usb_audio_config));

    BaseType_t created = xTaskCreate(button_task, "voice_button", 3072, NULL,
                                     7, NULL);
    ESP_ERROR_CHECK(created == pdPASS ? ESP_OK : ESP_FAIL);
    ESP_LOGI(TAG, "XIAO voice keyboard started");
}
