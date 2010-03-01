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
    test_utils.cpp
)

add_library(gtest ${GTEST-SOURCES})

add_custom_target(test
    echo "Running tests"
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
    DEPENDS song_test m3uparser_test)

# Given a file foo_test.cpp, creates a target foo_test and adds it to the test target.
macro(add_test_file test_source)
    get_filename_component(TEST_NAME ${ARGV0} NAME_WE)
    add_executable(${TEST_NAME}
       ${ARGV0}
       ../3rdparty/gtest/src/gtest_main.cc)
    target_link_libraries(${TEST_NAME} clementine_lib gtest)
    add_custom_command(TARGET test POST_BUILD
        COMMAND ./${TEST_NAME})
endmacro (add_test_file)


add_test_file(m3uparser_test.cpp)
add_test_file(song_test.cpp)
