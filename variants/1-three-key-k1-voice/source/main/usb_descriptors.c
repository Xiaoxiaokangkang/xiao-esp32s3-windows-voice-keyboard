#include <string.h>

#include "tusb.h"
#include "uac_descriptors.h"

enum {
    ITF_NUM_AUDIO_CONTROL = 0,
    ITF_NUM_AUDIO_STREAMING_MIC,
    ITF_NUM_HID_KEYBOARD,
    ITF_NUM_TOTAL,
};

#define EPNUM_AUDIO_IN 0x81
#define EPNUM_HID_IN   0x82

static tusb_desc_device_t const device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A,
    .idProduct = 0x4010,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&device_descriptor;
}

static uint8_t const hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return hid_report_descriptor;
}

#define CONFIG_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + TUD_AUDIO_DEVICE_DESC_LEN + TUD_HID_DESC_LEN)

static uint8_t const configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_AUDIO_MIC_DESCRIPTOR(ITF_NUM_AUDIO_CONTROL, 4, EPNUM_AUDIO_IN),
    TUD_HID_DESCRIPTOR(ITF_NUM_HID_KEYBOARD, 6, HID_ITF_PROTOCOL_KEYBOARD,
                       sizeof(hid_report_descriptor), EPNUM_HID_IN, 8, 10),
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return configuration_descriptor;
}

static char const *const string_descriptors[] = {
    (const char[]){0x09, 0x04},
    "Seeed Studio",
    "XIAO Voice Keyboard",
    "XIAO-S3-VOICE-01",
    "Voice Keyboard Audio",
    "INMP441 Microphone",
    "Voice Hotkey Keyboard",
};

static uint16_t string_buffer[32];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    uint8_t count;

    if (index == 0) {
        memcpy(&string_buffer[1], string_descriptors[0], 2);
        count = 1;
    } else {
        if (index >= sizeof(string_descriptors) / sizeof(string_descriptors[0])) {
            return NULL;
        }
        const char *text = string_descriptors[index];
        count = (uint8_t)strlen(text);
        if (count > 31) {
            count = 31;
        }
        for (uint8_t i = 0; i < count; ++i) {
            string_buffer[1 + i] = (uint8_t)text[i];
        }
    }

    string_buffer[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * count + 2));
    return string_buffer;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t requested_len)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)requested_len;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer, uint16_t bufsize)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}
