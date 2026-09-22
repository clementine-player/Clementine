# dpkg-buildpackage builds for the host architecture, so default to that.
execute_process(COMMAND dpkg --print-architecture
  OUTPUT_VARIABLE DEB_HOST_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT DEB_HOST_ARCH)
  set(DEB_HOST_ARCH amd64)
endif()

set(DEB_ARCH ${DEB_HOST_ARCH} CACHE STRING "Architecture of the deb file")
set(DEB_DIST "unstable" CACHE STRING "Distribution to set in the .deb changelog")

add_custom_target(deb
  COMMAND dpkg-buildpackage -b -d -uc -us
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/../clementine_${CLEMENTINE_VERSION_DEB}~${DEB_DIST}_${DEB_ARCH}.deb
                                   ${CMAKE_BINARY_DIR}/clementine_${CLEMENTINE_VERSION_DEB}~${DEB_DIST}_${DEB_ARCH}.deb
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
