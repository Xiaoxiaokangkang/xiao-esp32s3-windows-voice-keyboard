set(priv_requires esp_timer)

if(${IDF_VERSION_MAJOR} LESS 6)
    list(APPEND priv_requires usb)
endif()

idf_component_register(
    SRCS usb_device_uac.c
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "tusb_uac"
    PRIV_REQUIRES ${priv_requires}
)

idf_build_get_property(build_components BUILD_COMPONENTS)
if(tinyusb IN_LIST build_components)
    set(tinyusb_name tinyusb)
else()
    set(tinyusb_name espressif__tinyusb)
endif()

idf_component_get_property(tusb_lib ${tinyusb_name} COMPONENT_LIB)
target_include_directories(
    ${tusb_lib} PUBLIC
    "${COMPONENT_DIR}/tusb_uac"
    "${PROJECT_DIR}/main"
)

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
