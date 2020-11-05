add_executable(tone_file_generator tone_file_generator.cpp)
target_link_libraries(tone_file_generator ${LINK_LIBS})
target_include_directories(tone_file_generator PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)