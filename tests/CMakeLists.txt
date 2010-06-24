cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/gtest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/gtest)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)
include_directories(${QT_QTTEST_INCLUDE_DIR})

set(GTEST-SOURCES
    ../3rdparty/gmock/gtest/src/gtest.cc
    ../3rdparty/gmock/gtest/src/gtest-death-test.cc
    ../3rdparty/gmock/gtest/src/gtest-filepath.cc
    ../3rdparty/gmock/gtest/src/gtest-port.cc
    ../3rdparty/gmock/gtest/src/gtest-test-part.cc
    ../3rdparty/gmock/gtest/src/gtest-typed-test.cc
)

add_library(gtest STATIC EXCLUDE_FROM_ALL ${GTEST-SOURCES})

set(GMOCK-SOURCES
    ../3rdparty/gmock/src/gmock-cardinalities.cc
    ../3rdparty/gmock/src/gmock.cc
    ../3rdparty/gmock/src/gmock-internal-utils.cc
    ../3rdparty/gmock/src/gmock-matchers.cc
    ../3rdparty/gmock/src/gmock-printers.cc
    ../3rdparty/gmock/src/gmock-spec-builders.cc)

add_library(gmock STATIC EXCLUDE_FROM_ALL ${GMOCK-SOURCES})
target_link_libraries(gmock gtest)

set(TESTUTILS-SOURCES
    test_utils.cpp
    mock_networkaccessmanager.cpp
    mock_taglib.cpp
    mock_playlistitem.cpp
)

set(TESTUTILS-MOC-HEADERS
    mock_networkaccessmanager.h
)

qt4_wrap_cpp(TESTUTILS-SOURCES-MOC ${TESTUTILS-MOC-HEADERS})

add_library(test_utils STATIC EXCLUDE_FROM_ALL ${TESTUTILS-SOURCES} ${TESTUTILS-SOURCES-MOC})
target_link_libraries(test_utils gmock ${QT_LIBRARIES} ${QT_QTTEST_LIBRARY})

add_custom_target(test
    echo "Running tests"
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
)
add_custom_target(build_tests
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
)
add_dependencies(test build_tests)

qt4_add_resources(TEST-RESOURCE-SOURCES data/testdata.qrc)

add_library(test_gui_main STATIC EXCLUDE_FROM_ALL ${TEST-RESOURCE-SOURCES} main.cpp)
target_link_libraries(test_gui_main clementine_lib)
set_target_properties(test_gui_main PROPERTIES COMPILE_DEFINITIONS GUI)

add_library(test_main STATIC EXCLUDE_FROM_ALL ${TEST-RESOURCE-SOURCES} main.cpp)
target_link_libraries(test_main clementine_lib)

# Given a file foo_test.cpp, creates a target foo_test and adds it to the test target.
macro(add_test_file test_source gui_required)
    get_filename_component(TEST_NAME ${test_source} NAME_WE)
    add_executable(${TEST_NAME}
      EXCLUDE_FROM_ALL
      ${test_source}
    )
    target_link_libraries(${TEST_NAME} gmock clementine_lib test_utils)
    set(GUI_REQUIRED ${gui_required})
    if (GUI_REQUIRED)
      target_link_libraries(${TEST_NAME} test_gui_main)
    else (GUI_REQUIRED)
      target_link_libraries(${TEST_NAME} test_main)
    endif (GUI_REQUIRED)

    if (WIN32)
        add_custom_command(TARGET test POST_BUILD
            COMMAND ./${TEST_NAME}.exe)
    else (WIN32)
        add_custom_command(TARGET test POST_BUILD
            COMMAND ./${TEST_NAME})
    endif (WIN32)
    add_dependencies(build_tests ${TEST_NAME})
endmacro (add_test_file)


#add_test_file(albumcoverfetcher_test.cpp false)
add_test_file(albumcovermanager_test.cpp true)
add_test_file(asxparser_test.cpp false)
add_test_file(database_test.cpp false)
add_test_file(fileformats_test.cpp false)
add_test_file(librarybackend_test.cpp false)
add_test_file(librarymodel_test.cpp true)
add_test_file(m3uparser_test.cpp false)
add_test_file(mergedproxymodel_test.cpp false)
add_test_file(organiseformat_test.cpp false)
add_test_file(playlist_test.cpp true)
add_test_file(plsparser_test.cpp false)
add_test_file(scopedtransaction_test.cpp false)
add_test_file(songloader_test.cpp false)
add_test_file(songplaylistitem_test.cpp false)
add_test_file(song_test.cpp false)
add_test_file(translations_test.cpp false)
add_test_file(xspfparser_test.cpp false)

