function(target_copy_assets target assets_dir)
  # Every file inside the assets directory.
  file(GLOB_RECURSE assets CONFIGURE_DEPENDS "${assets_dir}/*")

  # An empty file whose timestamp records when we last copied.
  set(stamp "${CMAKE_CURRENT_BINARY_DIR}/${target}_assets.stamp")

  # Runs only when an asset is newer than the stamp.
  add_custom_command(
    OUTPUT "${stamp}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${assets_dir}"
            "$<TARGET_FILE_DIR:${target}>/assets"
    COMMAND ${CMAKE_COMMAND} -E touch "${stamp}"
    DEPENDS ${assets})

  # Make building the target also run the command above.
  add_custom_target(${target}_assets DEPENDS "${stamp}")
  add_dependencies(${target} ${target}_assets)
endfunction()
