cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(spielerei C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w) #compile for pico_w board

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(spielerei
spielerei.c
)

pico_generate_pio_header(spielerei ${CMAKE_CURRENT_LIST_DIR}/spielerei.pio)

target_sources(spielerei PRIVATE spielerei.c)

target_link_libraries(spielerei pico_stdlib hardware_pio)

pico_enable_stdio_usb(spielerei 1)
pico_enable_stdio_uart(spielerei 1)
pico_add_extra_outputs(spielerei)