add_library(glad src/gl.c)
add_library(glad::glad ALIAS glad)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})
