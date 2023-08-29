cmake_minimum_required(VERSION 3.10)

include(CTest)

project(ast_tests)
add_executable(ast_tests ast.cpp)
target_link_libraries(ast_tests compiler)
target_include_directories(ast_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME ast_tests COMMAND ast_tests)
