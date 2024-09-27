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

add_executable(spielerei)

target_sources(spielerei PUBLIC 
spielerei.c 
gdisp_hld.c
../aimonen/gdisp_lld.c 
../mcufont/mf_encoding.c 
../mcufont/mf_font.c 
../mcufont/mf_justify.c 
../mcufont/mf_kerning.c 
../mcufont/mf_rlefont.c 
../mcufont/mf_bwfont.c 
../mcufont/mf_scaledfont.c 
../mcufont/mf_wordwrap.c
../mcufont/DejaVuSans12.c 
../mcufont/DejaVuSans12bw_bwfont.c 
../mcufont/DejaVuSans12bw.c 
../mcufont/DejaVuSerif16.c 
../mcufont/DejaVuSerif32.c 
../mcufont/fixed_5x8.c 
../mcufont/fixed_7x14.c 
../mcufont/fixed_10x20.c
)

pico_generate_pio_header(spielerei ${CMAKE_CURRENT_LIST_DIR}/ED060SC7_refresh.pio)

target_include_directories(spielerei PUBLIC aimonen mcufont)

target_link_libraries(spielerei pico_stdlib hardware_pio hardware_dma)

pico_enable_stdio_usb(spielerei 1)
pico_enable_stdio_uart(spielerei 1)
pico_add_extra_outputs(spielerei)
