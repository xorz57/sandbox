cmake_minimum_required(VERSION 3.30)

project(sandbox)

enable_testing()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(vendor SYSTEM)

set(CMAKE_CXX_CLANG_TIDY clang-tidy)

add_subdirectory(src)
