add_library(logic STATIC api/api.c api/api-callbacks.c button-task/button-task.c led-task/led-task.c pi-task/pi-task.c)

target_include_directories(logic PUBLIC ${CMAKE_SOURCE_DIR}/logic ${CMAKE_SOURCE_DIR})

target_link_libraries(logic PUBLIC stdio_text_protocol)

target_link_libraries(logic PRIVATE device profiling systime)

add_library(platform STATIC platform/platform.c)

target_include_directories(platform PUBLIC ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR})

target_link_libraries(platform PRIVATE pico_stdlib)
