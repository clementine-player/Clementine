cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fpermissive -Wno-c++11-narrowing -U__STRICT_ANSI__")

if(USE_SYSTEM_GMOCK)
  include_directories(${GMOCK_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
else(USE_SYSTEM_GMOCK)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/include)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/gtest/include)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gmock/gtest)
endif(USE_SYSTEM_GMOCK)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/ext/clementine-tagreader)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-tagreader)

include_directories(${QT_QTTEST_INCLUDE_DIR})

if(HAVE_LIBGPOD)
  include_directories(${LIBGPOD_INCLUDE_DIRS})
endif(HAVE_LIBGPOD)

if(HAVE_LIBLASTFM)
  include_directories(${LASTFM_INCLUDE_DIRS})
endif(HAVE_LIBLASTFM)

if(NOT USE_SYSTEM_GMOCK)
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
    ../3rdparty/gmock/src/gmock-spec-builders.cc
  )

  add_library(gmock STATIC EXCLUDE_FROM_ALL ${GMOCK-SOURCES})
  set(GMOCK_LIBRARIES gmock gtest)
endif(NOT USE_SYSTEM_GMOCK)

add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=1)

set(TESTUTILS-SOURCES
  mock_networkaccessmanager.cpp
  mock_playlistitem.cpp
  test_utils.cpp
  testobjectdecorators.cpp

  ${CMAKE_SOURCE_DIR}/ext/libclementine-tagreader/fmpsparser.cpp
)

set(TESTUTILS-MOC-HEADERS
  mock_networkaccessmanager.h
  test_utils.h
  testobjectdecorators.h
)

qt4_wrap_cpp(TESTUTILS-SOURCES-MOC ${TESTUTILS-MOC-HEADERS})

add_library(test_utils STATIC EXCLUDE_FROM_ALL ${TESTUTILS-SOURCES} ${TESTUTILS-SOURCES-MOC})
target_link_libraries(test_utils ${GMOCK_LIBRARIES} ${QT_LIBRARIES} ${QT_QTTEST_LIBRARY})

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
    target_link_libraries(${TEST_NAME} ${GMOCK_LIBRARIES} clementine_lib test_utils)
    set(GUI_REQUIRED ${gui_required})
    if (GUI_REQUIRED)
      target_link_libraries(${TEST_NAME} test_gui_main)
    else (GUI_REQUIRED)
      target_link_libraries(${TEST_NAME} test_main)
    endif (GUI_REQUIRED)

    check_cxx_compiler_flag("-Wno-bool-conversions" SUPPORTS_NOBOOL)
    if (SUPPORTS_NOBOOL)
      set_target_properties(${TEST_NAME} PROPERTIES COMPILE_FLAGS "-Wno-bool-conversions")
    endif (SUPPORTS_NOBOOL)

    add_custom_command(TARGET test POST_BUILD
        COMMAND ./${TEST_NAME}${CMAKE_EXECUTABLE_SUFFIX})
    add_dependencies(build_tests ${TEST_NAME})
endmacro (add_test_file)


#add_test_file(albumcoverfetcher_test.cpp false)

#add_test_file(albumcovermanager_test.cpp true)
add_test_file(asxparser_test.cpp false)
add_test_file(asxiniparser_test.cpp false)
#add_test_file(cueparser_test.cpp false)
#add_test_file(database_test.cpp false)
#add_test_file(fileformats_test.cpp false)
add_test_file(fmpsparser_test.cpp false)
#add_test_file(librarybackend_test.cpp false)
#add_test_file(librarymodel_test.cpp true)
#add_test_file(m3uparser_test.cpp false)
add_test_file(mergedproxymodel_test.cpp false)
add_test_file(organiseformat_test.cpp false)
#add_test_file(playlist_test.cpp true)
#add_test_file(plsparser_test.cpp false)
add_test_file(scopedtransaction_test.cpp false)
#add_test_file(songloader_test.cpp false)
add_test_file(songplaylistitem_test.cpp false)
add_test_file(song_test.cpp false)
add_test_file(translations_test.cpp false)
add_test_file(utilities_test.cpp false)
#add_test_file(xspfparser_test.cpp false)
add_test_file(closure_test.cpp false)
add_test_file(concurrentrun_test.cpp false)
add_test_file(zeroconf_test.cpp false)
add_test_file(sqlite_test.cpp false)

#if(LINUX AND HAVE_DBUS)
#  add_test_file(mpris1_test.cpp true)
#endif(LINUX AND HAVE_DBUS)
