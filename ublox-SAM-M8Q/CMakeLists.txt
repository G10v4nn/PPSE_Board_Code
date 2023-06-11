cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(ublox-SAM-M8Q
        misc.c
        gps.c
        )


# pull in common dependencies
target_link_libraries(ublox-SAM-M8Q pico_stdlib hardware_uart hardware_gpio)

# enable usb output, disable uart output
pico_enable_stdio_usb(ublox-SAM-M8Q 1)
pico_enable_stdio_uart(ublox-SAM-M8Q 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(ublox-SAM-M8Q)

