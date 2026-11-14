add_library(platform STATIC platform.c)

target_include_directories(platform PUBLIC ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR})

target_link_libraries(platform PRIVATE pico_stdlib)