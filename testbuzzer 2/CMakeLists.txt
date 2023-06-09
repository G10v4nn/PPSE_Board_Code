cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(pwm_led_fade
        pwm_led_fade.c
        )

# pull in common dependencies and additional pwm hardware support
target_link_libraries(pwm_led_fade pico_stdlib hardware_pwm)

# enable usb output, disable uart output
pico_enable_stdio_usb(pwm_led_fade 1)
pico_enable_stdio_uart(pwm_led_fade 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(pwm_led_fade)




