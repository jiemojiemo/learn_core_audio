add_executable(meta_data meta_data.cpp)
target_link_libraries(meta_data PUBLIC ${LINK_LIBS})
target_include_directories(meta_data PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)