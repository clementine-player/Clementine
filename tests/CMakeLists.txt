cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -Wno-c++11-narrowing")

set(GMOCK_LIBRARIES GTest::gmock GTest::gtest)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/ext/clementine-tagreader)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-remote)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-remote)

include_directories(${QT_QTTEST_INCLUDE_DIR})

# Test executables link against clementine_lib, which requires these at
# link time. src/CMakeLists.txt sets up the same link_directories() for
# clementine_lib itself, but that call is scoped to that directory and
# doesn't reach targets defined here.
link_directories(
  ${CHROMAPRINT_LIBRARY_DIRS}
  ${GSTREAMER_APP_LIBRARY_DIRS}
  ${GLEW_LIBRARY_DIRS}
  ${LIBGPOD_LIBRARY_DIRS}
  ${LIBMTP_LIBRARY_DIRS}
)

if(HAVE_LIBGPOD)
  include_directories(${LIBGPOD_INCLUDE_DIRS})
endif(HAVE_LIBGPOD)


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

qt6_wrap_cpp(TESTUTILS-SOURCES-MOC ${TESTUTILS-MOC-HEADERS})

add_library(test_utils STATIC EXCLUDE_FROM_ALL ${TESTUTILS-SOURCES} ${TESTUTILS-SOURCES-MOC})
target_link_libraries(test_utils ${GMOCK_LIBRARIES} ${QT_LIBRARIES} ${QT_QTTEST_LIBRARY})

add_custom_target(clementine_test
    echo "Running Clementine tests"
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
)
add_custom_target(build_tests
    WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
)
add_dependencies(clementine_test build_tests)

qt6_add_resources(TEST-RESOURCE-SOURCES data/testdata.qrc)

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
    target_link_libraries(${TEST_NAME} ${GMOCK_LIBRARIES} clementine_lib test_utils Qt6::Test)
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

    add_custom_command(TARGET clementine_test POST_BUILD
        COMMAND ./${TEST_NAME}${CMAKE_EXECUTABLE_SUFFIX})
    add_dependencies(build_tests ${TEST_NAME})
endmacro (add_test_file)

# As add_test_file, for a test that reads tags with real tag reader workers
# (see tagreader_env.h).
macro(add_tagreader_test_file test_source gui_required)
    add_test_file(${test_source} ${gui_required})
    get_filename_component(TAGREADER_TEST_NAME ${test_source} NAME_WE)
    add_dependencies(${TAGREADER_TEST_NAME} clementine-tagreader)
    target_compile_definitions(${TAGREADER_TEST_NAME} PRIVATE
      TAGREADER_DIR="$<TARGET_FILE_DIR:clementine-tagreader>")
endmacro (add_tagreader_test_file)

add_test_file(appearance_test.cpp true)
add_test_file(asxparser_test.cpp false)
add_test_file(asxiniparser_test.cpp false)
add_tagreader_test_file(cueparser_test.cpp false)
add_test_file(database_test.cpp false)
add_tagreader_test_file(fileformats_test.cpp false)
add_test_file(fancytabwidget_test.cpp true)
add_test_file(fmpsparser_test.cpp false)
add_test_file(gpodderclient_test.cpp false)
add_test_file(librarytagprefetcher_test.cpp false)
add_test_file(lrcparser_test.cpp false)
add_test_file(messagereply_test.cpp false)
add_test_file(lyricsfetcher_test.cpp false)
add_test_file(syncedlyricsview_test.cpp true)
add_test_file(outputbutton_test.cpp true)
add_test_file(trackslider_test.cpp true)
add_tagreader_test_file(librarywatcher_test.cpp false)
target_compile_definitions(librarywatcher_test PRIVATE
  TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
if(HAVE_GOOGLE_DRIVE)
  add_test_file(googledriveclient_test.cpp false)
endif(HAVE_GOOGLE_DRIVE)
add_test_file(librarybackend_test.cpp false)
add_test_file(librarymodel_test.cpp true)
add_tagreader_test_file(m3uparser_test.cpp false)
add_test_file(mergedproxymodel_test.cpp false)
add_test_file(enginerouter_test.cpp false)
add_test_file(internetbrowser_test.cpp false)
add_test_file(networkremote_test.cpp false)
add_test_file(remotesmartplaylists_test.cpp false)
add_test_file(remotestreaming_test.cpp false)
add_test_file(mediasync_test.cpp false)
add_test_file(musicbrainzclient_test.cpp false)
add_test_file(organiseformat_test.cpp false)
add_test_file(organisedialog_test.cpp false)
add_test_file(plexparser_test.cpp false)
add_test_file(playlist_test.cpp true)
add_tagreader_test_file(plsparser_test.cpp false)
add_test_file(scopedtransaction_test.cpp false)
add_test_file(sleepinhibitor_test.cpp false)
add_tagreader_test_file(songloader_test.cpp false)
add_test_file(songinfobase_test.cpp true)
add_test_file(songplaylistitem_test.cpp false)
add_test_file(song_test.cpp false)
add_test_file(translations_test.cpp false)
add_test_file(utilities_test.cpp false)
add_test_file(xspfparser_test.cpp false)
add_test_file(airplay_test.cpp false)
target_sources(airplay_test PRIVATE fakeairplayreceiver.cpp)
add_test_file(castchannel_test.cpp false)
add_test_file(castdiscovery_test.cpp false)
add_test_file(castmediaplayer_test.cpp false)
add_test_file(upnp_test.cpp false)
add_test_file(closure_test.cpp false)
add_test_file(concurrentrun_test.cpp false)
add_test_file(zeroconf_test.cpp false)
add_test_file(sqlite_test.cpp false)
