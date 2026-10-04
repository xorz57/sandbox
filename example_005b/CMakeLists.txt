add_executable(example_005b)
target_sources(example_005b PRIVATE main.cc mesh.cc shader.cc program.cc
                                    camera.cc stb_image_impl.cc)

target_link_libraries(example_005b PRIVATE glad::glad)
target_link_libraries(example_005b PRIVATE glm::glm)
target_link_libraries(example_005b PRIVATE SDL3::SDL3)
target_link_libraries(example_005b PRIVATE spdlog::spdlog)
target_link_libraries(example_005b PRIVATE imgui::imgui)
target_link_libraries(example_005b PRIVATE stb::stb)

target_copy_assets(example_005b "${CMAKE_CURRENT_SOURCE_DIR}/assets")
