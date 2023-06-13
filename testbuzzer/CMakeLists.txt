cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(buzzertest
        main.c
	buzzer.c
        )

# pull in common dependencies and additional pwm hardware support
target_link_libraries(buzzertest pico_stdlib hardware_gpio hardware_pwm hardware_clocks)

# enable usb output, disable uart output
pico_enable_stdio_usb(buzzertest 1)
pico_enable_stdio_uart(buzzertest 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(buzzertest)



