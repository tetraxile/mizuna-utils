cmake_minimum_required(VERSION 3.15)
project(mizuna-utils)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)
add_subdirectory(lib/mizuna)
add_subdirectory(lib/argspp)
