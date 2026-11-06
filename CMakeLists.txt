cmake_minimum_required(VERSION 3.16)
project(cellsim)

add_library(compiler-options INTERFACE)
target_compile_features(compiler-options INTERFACE cxx_std_23)
target_compile_options(compiler-options INTERFACE "-Og")

add_executable(main "src/main.cpp")
set_target_properties(main PROPERTIES OUTPUT_NAME "cellsim")

target_link_libraries(main compiler-options)
