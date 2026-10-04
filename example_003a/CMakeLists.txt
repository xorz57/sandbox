add_executable(example_003a)
target_sources(example_003a PRIVATE main.cc shader.cc program.cc camera.cc
                                    stb_image_impl.cc)

target_link_libraries(example_003a PRIVATE glad::glad)
target_link_libraries(example_003a PRIVATE glm::glm)
target_link_libraries(example_003a PRIVATE SDL3::SDL3)
target_link_libraries(example_003a PRIVATE spdlog::spdlog)
target_link_libraries(example_003a PRIVATE imgui::imgui)
target_link_libraries(example_003a PRIVATE stb::stb)

target_copy_assets(example_003a "${CMAKE_CURRENT_SOURCE_DIR}/assets")
