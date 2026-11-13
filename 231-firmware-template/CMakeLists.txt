cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

include(pico_sdk_import.cmake)
project(231_firmware_template)
pico_sdk_init()

add_compile_options(-Wall)

add_subdirectory(../libs ${CMAKE_BINARY_DIR}/libs)
add_subdirectory(logic ${CMAKE_BINARY_DIR}/logic)

add_executable(${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
)
target_link_libraries(${PROJECT_NAME} pico_stdlib device logging systime stdio_text_protocol profiling logic platform)

target_compile_definitions(profiling PUBLIC PROFILING_ENABLED=1)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})