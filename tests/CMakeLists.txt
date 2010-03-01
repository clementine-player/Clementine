cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gtest)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gtest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

set(GTEST-SOURCES
    ../gtest/src/gtest.cc
    ../gtest/src/gtest-death-test.cc
    ../gtest/src/gtest-filepath.cc
    ../gtest/src/gtest-port.cc
    ../gtest/src/gtest-test-part.cc
    ../gtest/src/gtest-typed-test.cc
)

add_library(gtest ${GTEST-SOURCES})


set(SONGTEST-SOURCES
    ../gtest/src/gtest_main.cc
    ../src/song.cpp
    song_test.cpp)

add_executable(song_test ${SONGTEST-SOURCES})
target_link_libraries(song_test clementine_lib gtest)

add_custom_target(test
    ./song_test
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
    DEPENDS song_test)
