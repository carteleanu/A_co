cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_PATH "C:/Projects/Ethernet/WIZnet-PICO-C/libraries/pico-sdk")
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(usb_test C CXX ASM)

set(PICO_BOARD pico2)

pico_sdk_init()

add_executable(usb_test main.c)
target_link_libraries(usb_test pico_stdlib)

pico_enable_stdio_usb(usb_test 1)
pico_enable_stdio_uart(usb_test 0)

pico_add_extra_outputs(usb_test)

