add_executable(stream_format_tester stream_format_tester.cpp)
target_link_libraries(stream_format_tester ${LINK_LIBS})
target_include_directories(stream_format_tester PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)