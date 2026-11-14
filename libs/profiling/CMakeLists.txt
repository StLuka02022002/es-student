add_library(profiling STATIC src/profiling.c)

target_include_directories(profiling PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(profiling PRIVATE pico_stdlib)