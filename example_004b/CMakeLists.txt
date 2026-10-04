add_executable(example_004b)
target_sources(example_004b PRIVATE main.cc mesh.cc shader.cc program.cc
                                    camera.cc stb_image_impl.cc)

target_link_libraries(example_004b PRIVATE glad::glad)
target_link_libraries(example_004b PRIVATE glm::glm)
target_link_libraries(example_004b PRIVATE SDL3::SDL3)
target_link_libraries(example_004b PRIVATE spdlog::spdlog)
target_link_libraries(example_004b PRIVATE imgui::imgui)
target_link_libraries(example_004b PRIVATE stb::stb)

target_copy_assets(example_004b "${CMAKE_CURRENT_SOURCE_DIR}/assets")
