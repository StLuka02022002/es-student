add_library(logging STATIC src/log.c)

target_include_directories(logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)