cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gtest)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gtest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

set(GTEST-SOURCES
    ../3rdparty/gtest/src/gtest.cc
    ../3rdparty/gtest/src/gtest-death-test.cc
    ../3rdparty/gtest/src/gtest-filepath.cc
    ../3rdparty/gtest/src/gtest-port.cc
    ../3rdparty/gtest/src/gtest-test-part.cc
    ../3rdparty/gtest/src/gtest-typed-test.cc
)

add_library(gtest ${GTEST-SOURCES})


set(SONGTEST-SOURCES
    ../3rdparty/gtest/src/gtest_main.cc
    ../src/song.cpp
    song_test.cpp)

add_executable(song_test ${SONGTEST-SOURCES})
target_link_libraries(song_test clementine_lib gtest)

add_custom_target(test
    ./song_test
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
    DEPENDS song_test)
