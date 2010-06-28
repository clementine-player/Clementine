execute_process(COMMAND date "+%a, %-d %b %Y %H:%M:%S %z"
                OUTPUT_VARIABLE DEB_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/changelog.in
               ${CMAKE_CURRENT_SOURCE_DIR}/changelog)
