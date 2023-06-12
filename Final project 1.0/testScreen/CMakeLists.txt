
add_library(screen main.cpp)

# rest of your project
add_subdirectory(pico-ssd1306)



# pull in common dependencies
target_link_libraries(screen pico_stdlib hardware_i2c pico_ssd1306)

target_link_directories(screen PUBLIC $(CMAKE_CURRENT_SOURCE_DIR))

