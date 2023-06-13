cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(main
main.c
accelerometer.c
temperature.c
gps.c
buzzer.c
        )
add_library(screen screen.c)
target_link_libraries(screen  pico_stdlib hardware_adc hardware_clocks hardware_rosc hardware_rtc hardware_i2c)

add_subdirectory(hardware_rosc)

# pull in common dependencies
target_link_libraries(main pico_stdlib hardware_adc hardware_clocks hardware_rosc hardware_rtc hardware_i2c hardware_i2c hardware_pwm screen)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)



