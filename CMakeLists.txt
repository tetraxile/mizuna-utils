cmake_minimum_required(VERSION 3.15)
project(mizuna-utils)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Werror=return-type -Wno-unqualified-std-cast-call -Wno-deprecated-declarations -Wno-main -Wno-invalid-offsetof)

add_subdirectory(src)
add_subdirectory(lib/mizuna)
add_subdirectory(lib/argspp)
