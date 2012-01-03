cmake_minimum_required(VERSION 2.8)

execute_process(COMMAND env LC_ALL=C date "+%a, %-d %b %Y %H:%M:%S %z"
                OUTPUT_VARIABLE DEB_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

set(DEB_REWRITE_RULES ON CACHE BOOL "Rewrite the debian rules file from rules.in")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/changelog.in
               ${CMAKE_CURRENT_SOURCE_DIR}/changelog)

if(DEB_REWRITE_RULES)
  configure_file(  ${CMAKE_CURRENT_SOURCE_DIR}/rules.in
                   ${CMAKE_CURRENT_BINARY_DIR}/rules.tmp/rules @ONLY)
  file(REMOVE      ${CMAKE_CURRENT_SOURCE_DIR}/rules)
  file(COPY        ${CMAKE_CURRENT_BINARY_DIR}/rules.tmp/rules
       DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/
       FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                        GROUP_EXECUTE GROUP_READ)
endif(DEB_REWRITE_RULES)
