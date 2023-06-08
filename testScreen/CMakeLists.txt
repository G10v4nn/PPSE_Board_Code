cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(screen
        main.cpp
        )

add_subdirectory(pico-ssd1306)


# pull in common dependencies
target_link_libraries(screen pico_stdlib hardware_i2c pico_ssd1306)

# enable usb output, disable uart output
pico_enable_stdio_usb(screen 1)
pico_enable_stdio_uart(screen 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(screen)

