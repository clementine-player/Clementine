execute_process(COMMAND date "+%a, %-d %b %Y %H:%M:%S %z"
                OUTPUT_VARIABLE DEB_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

set(DEB_DIST "unstable" CACHE STRING "Distribution to set in the .deb changelog")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/changelog.in
               ${CMAKE_CURRENT_SOURCE_DIR}/changelog)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rules.in
               ${CMAKE_CURRENT_SOURCE_DIR}/rules @ONLY)
