cmake_minimum_required(VERSION 2.8)

execute_process(COMMAND date "+%a, %-d %b %Y %H:%M:%S %z"
                OUTPUT_VARIABLE DEB_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

set(DEB_DIST "unstable" CACHE STRING "Distribution to set in the .deb changelog")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/changelog.in
               ${CMAKE_CURRENT_SOURCE_DIR}/changelog)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rules.in
               ${CMAKE_CURRENT_BINARY_DIR}/rules @ONLY)
file(COPY        ${CMAKE_CURRENT_BINARY_DIR}/rules
     DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/
     FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                      GROUP_EXECUTE GROUP_READ)
