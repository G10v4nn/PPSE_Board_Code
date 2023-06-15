cmake_minimum_required(VERSION 3.5)

# include(pico_sdk_import.cmake)

# project(myproject)

# pico_sdk_init()

add_library(cdc_msc
	main.c
	msc_disk.c
	usb_descriptors.c
	)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(cdc_msc PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(cdc_msc PUBLIC pico_stdlib tinyusb_device tinyusb_board)
# pico_add_extra_outputs(cdc_msc)