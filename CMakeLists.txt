cmake_minimum_required(VERSION 3.28)
project(sandbox VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(vendor SYSTEM)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(ClangTidy)
include(CopyAssets)

add_executable(sandbox)
target_include_directories(sandbox PUBLIC src)
target_sources(
  sandbox
  PRIVATE src/main.cpp
          src/camera_controller.cpp
          src/camera.cpp
          src/program.cpp
          src/stb_image_impl.cpp)

# target_link_libraries(sandbox PRIVATE assimp::assimp)
target_link_libraries(sandbox PRIVATE glad::glad)
target_link_libraries(sandbox PRIVATE glm::glm)
target_link_libraries(sandbox PRIVATE SDL3::SDL3)
# target_link_libraries(sandbox PRIVATE simdjson::simdjson)
target_link_libraries(sandbox PRIVATE spdlog::spdlog)
target_link_libraries(sandbox PRIVATE imgui::imgui)
target_link_libraries(sandbox PRIVATE stb::stb)

target_copy_assets(sandbox "${CMAKE_CURRENT_SOURCE_DIR}/assets")
