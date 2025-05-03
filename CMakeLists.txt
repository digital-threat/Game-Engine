cmake_minimum_required(VERSION 3.29)
project(root)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static")
# Address Sanitizer
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_subdirectory(engine)
add_subdirectory(ray_tracer)
add_subdirectory(forward_renderer)