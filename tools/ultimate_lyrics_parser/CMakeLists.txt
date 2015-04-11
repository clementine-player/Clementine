cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Woverloaded-virtual -Wall -Wno-sign-compare ${CMAKE_CXX_FLAGS}")

add_executable(ultimate_lyrics_parser EXCLUDE_FROM_ALL main.cpp)

target_link_libraries(ultimate_lyrics_parser Qt5::Core)
