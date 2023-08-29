cmake_minimum_required(VERSION 3.10)

project(compiler)

add_library(compiler src/main.c)

target_include_directories(compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

enable_testing()

add_subdirectory(tests)
