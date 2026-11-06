cmake_minimum_required(VERSION 3.16)
project(cellsim)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_library(compiler-options INTERFACE)
target_compile_features(compiler-options INTERFACE cxx_std_23)
target_compile_options(compiler-options INTERFACE "-Og")

add_executable(main "src/main.cpp")
set_target_properties(main PROPERTIES OUTPUT_NAME "cellsim")

add_subdirectory(lib/SDL)

target_link_libraries(main PRIVATE compiler-options SDL3::SDL3)
