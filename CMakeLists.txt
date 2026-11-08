cmake_minimum_required(VERSION 3.16)
project(cellsim)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_library(compiler-options INTERFACE)
target_compile_features(compiler-options INTERFACE cxx_std_23)
target_compile_options(compiler-options INTERFACE "-Og")

target_link_options(compiler-options INTERFACE -static -static-libgcc -static-libstdc++)

add_executable(main "src/main.cpp")
set_target_properties(main PROPERTIES OUTPUT_NAME "cellsim")

target_sources(main PRIVATE "src/life.cpp")
target_sources(main PRIVATE "src/display.cpp")

target_include_directories(main PRIVATE "src")

add_subdirectory(lib/SDL)

target_link_libraries(main PRIVATE compiler-options SDL3::SDL3)
