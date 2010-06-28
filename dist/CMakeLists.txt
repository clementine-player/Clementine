execute_process(COMMAND date "+%a %b %d %Y"
                OUTPUT_VARIABLE RPM_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/clementine.spec.in
               ${CMAKE_CURRENT_SOURCE_DIR}/clementine.spec)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
               ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/maketarball.sh.in
               ${CMAKE_CURRENT_SOURCE_DIR}/maketarball.sh @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/windows/clementine.nsi.in
               ${CMAKE_CURRENT_SOURCE_DIR}/windows/clementine.nsi @ONLY)
# windows/windres.rc is done by src/CMakeLists.txt
