include(lastfm_add_test.cmake)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

lastfm_add_test(UrlBuilder)
lastfm_add_test(Track)
