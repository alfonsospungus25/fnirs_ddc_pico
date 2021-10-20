cmake_minimum_required(VERSION 3.20)
include(pico_sdk_import.cmake)

project (fnirs_ddc_pio_shk)

pico_sdk_init()

add_executable (fnirs_ddc_pio_shk
fnirs_ddc_pio_shk.c
        pio_spi.c)

pico_generate_pio_header(fnirs_ddc_pio_shk ${CMAKE_CURRENT_LIST_DIR}/spi.pio)

target_link_libraries (fnirs_ddc_pio_shk PRIVATE  
        pico_stdlib
        hardware_clocks
        hardware_pio)


pico_enable_stdio_usb(fnirs_ddc_pio_shk 1)
pico_enable_stdio_uart(fnirs_ddc_pio_shk 0)

pico_add_extra_outputs(fnirs_ddc_pio_shk)
