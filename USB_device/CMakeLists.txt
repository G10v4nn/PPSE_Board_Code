cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(dev_lowlevel
        dev_lowlevel.c
        hello_usb.c
        )


# pull in common dependencies
target_link_libraries(dev_lowlevel PRIVATE pico_stdlib hardware_resets hardware_irq)

# enable usb output, disable uart output
# pico_enable_stdio_usb(hello_usb 1)
# pico_enable_stdio_uart(hello_usb 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(dev_lowlevel)


