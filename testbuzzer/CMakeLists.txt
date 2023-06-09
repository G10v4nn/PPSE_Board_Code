cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(hello_pwm
        hello_pwm.c
        )

# pull in common dependencies and additional pwm hardware support
target_link_libraries(hello_pwm pico_stdlib hardware_pwm)

# enable usb output, disable uart output
pico_enable_stdio_usb(hello_pwm 1)
pico_enable_stdio_uart(hello_pwm 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(hello_pwm)



