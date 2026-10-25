function(target_copy_assets target assets_dir)
  file(GLOB_RECURSE assets CONFIGURE_DEPENDS "${assets_dir}/*")

  set(stamp "${CMAKE_CURRENT_BINARY_DIR}/${target}_assets.stamp")

  add_custom_command(
    OUTPUT "${stamp}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${assets_dir}"
            "$<TARGET_FILE_DIR:${target}>/assets"
    COMMAND ${CMAKE_COMMAND} -E touch "${stamp}"
    DEPENDS ${assets})

  add_custom_target(${target}_assets DEPENDS "${stamp}")
  add_dependencies(${target} ${target}_assets)
endfunction()
