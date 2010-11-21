set(DEB_ARCH amd64 CACHE STRING "Architecture of the deb file")

add_custom_target(deb
  COMMAND dpkg-buildpackage -b -d -uc -us
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/../clementine_${CLEMENTINE_DEB_VERSION}_${DEB_ARCH}.deb
                                   ${CMAKE_BINARY_DIR}/clementine.deb
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
