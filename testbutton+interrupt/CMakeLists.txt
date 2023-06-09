cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(hello_gpio_irq
        hello_gpio_irq.c
        )


# pull in common dependencies
target_link_libraries(hello_gpio_irq pico_stdlib)

# enable usb output, disable uart output
pico_enable_stdio_usb(hello_gpio_irq 1)
pico_enable_stdio_uart(hello_gpio_irq 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(hello_gpio_irq)

