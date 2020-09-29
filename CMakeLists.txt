cmake_minimum_required(VERSION 3.10)

project(learn_core_audio)
set(CMAKE_CXX_STANDARD 14)

set(LINK_LIBS "-framework CoreFoundation -framework AudioToolbox")

include_directories(include)

add_subdirectory(ch01)
add_subdirectory(ch02)
add_subdirectory(ch03)