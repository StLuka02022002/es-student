add_library(device STATIC src/device.c)

target_include_directories(device PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(device PRIVATE pico_stdlib pico_unique_id)
target_link_libraries(device PUBLIC pico_unique_id)

target_compile_definitions(device PRIVATE DEVICE_BOARD="${PICO_BOARD}")