cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(**project name**
        **your .c file**
        )


# pull in common dependencies
target_link_libraries(**pico_stdlib and other libraries used**)

# enable usb output, disable uart output
pico_enable_stdio_usb(**project name** 1)
pico_enable_stdio_uart(**project name** 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(**project name**)

