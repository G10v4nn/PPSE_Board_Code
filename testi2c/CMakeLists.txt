cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(i2c_bus_scan
        bus_scan.c
        )


# pull in common dependencies
target_link_libraries(i2c_bus_scan pico_stdlib hardware_i2c)

# enable usb output, disable uart output
pico_enable_stdio_usb(i2c_bus_scan 1)
pico_enable_stdio_uart(i2c_bus_scan 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(i2c_bus_scan)

