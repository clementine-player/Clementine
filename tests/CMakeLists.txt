cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/gtest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/gtest)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

set(GTEST-SOURCES
    ../3rdparty/gmock/gtest/src/gtest.cc
    ../3rdparty/gmock/gtest/src/gtest-death-test.cc
    ../3rdparty/gmock/gtest/src/gtest-filepath.cc
    ../3rdparty/gmock/gtest/src/gtest-port.cc
    ../3rdparty/gmock/gtest/src/gtest-test-part.cc
    ../3rdparty/gmock/gtest/src/gtest-typed-test.cc
    test_utils.cpp
)

add_library(gtest ${GTEST-SOURCES})

set(GMOCK-SOURCES
    ../3rdparty/gmock/src/gmock-cardinalities.cc
    ../3rdparty/gmock/src/gmock.cc
    ../3rdparty/gmock/src/gmock-internal-utils.cc
    ../3rdparty/gmock/src/gmock-matchers.cc
    ../3rdparty/gmock/src/gmock-printers.cc
    ../3rdparty/gmock/src/gmock-spec-builders.cc)

add_library(gmock ${GMOCK-SOURCES})
target_link_libraries(gmock gtest)

set(MOCK-SOURCES
    mock_networkaccessmanager.cpp)

set(MOCK-MOC-HEADERS
    mock_networkaccessmanager.h)

qt4_wrap_cpp(MOCK-SOURCES-MOC ${MOCK-MOC-HEADERS})

add_library(mocks ${MOCK-SOURCES} ${MOCK-SOURCES-MOC})
target_link_libraries(mocks gmock)

add_custom_target(test
    echo "Running tests"
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
)

# Given a file foo_test.cpp, creates a target foo_test and adds it to the test target.
macro(add_test_file test_source)
    get_filename_component(TEST_NAME ${test_source} NAME_WE)
    add_executable(${TEST_NAME}
       ${test_source}
       main.cpp
    )
    target_link_libraries(${TEST_NAME} gmock clementine_lib mocks)
    add_custom_command(TARGET test POST_BUILD
        COMMAND ./${TEST_NAME})
    add_dependencies(test ${TEST_NAME})
endmacro (add_test_file)


add_test_file(m3uparser_test.cpp)
add_test_file(song_test.cpp)
add_test_file(librarybackend_test.cpp)
add_test_file(albumcoverfetcher_test.cpp)
