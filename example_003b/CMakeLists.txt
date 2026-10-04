add_executable(example_003b)
target_sources(example_003b PRIVATE main.cc shader.cc program.cc camera.cc
                                    stb_image_impl.cc)

target_link_libraries(example_003b PRIVATE glad::glad)
target_link_libraries(example_003b PRIVATE glm::glm)
target_link_libraries(example_003b PRIVATE SDL3::SDL3)
target_link_libraries(example_003b PRIVATE spdlog::spdlog)
target_link_libraries(example_003b PRIVATE imgui::imgui)
target_link_libraries(example_003b PRIVATE stb::stb)

target_copy_assets(example_003b "${CMAKE_CURRENT_SOURCE_DIR}/assets")
