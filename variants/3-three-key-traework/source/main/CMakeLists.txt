idf_component_register(
    SRCS "app_main.c" "usb_descriptors.c"
    INCLUDE_DIRS "."
    REQUIRES driver usb_device_uac espressif__tinyusb
)

# TinyUSB calls these application callbacks from its archive. Force the linker
# to retain our descriptor/HID callback translation unit regardless of archive
# ordering.
target_link_libraries(${COMPONENT_LIB} INTERFACE
    "-Wl,-u,tud_descriptor_device_cb"
    "-Wl,-u,tud_descriptor_configuration_cb"
    "-Wl,-u,tud_descriptor_string_cb"
    "-Wl,-u,tud_hid_descriptor_report_cb"
    "-Wl,-u,tud_hid_get_report_cb"
    "-Wl,-u,tud_hid_set_report_cb"
)
