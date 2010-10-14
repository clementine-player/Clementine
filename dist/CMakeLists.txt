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


if (NOT APPLE)
  install(FILES clementine_64.png
    DESTINATION share/icons/hicolor/64x64/apps/
    RENAME application-x-clementine.png
  )

  install(FILES ../data/icon.svg
    DESTINATION share/icons/hicolor/scalable/apps/
    RENAME application-x-clementine.svg
  )

  install(FILES clementine.desktop
    DESTINATION share/applications
  )

  foreach(icon clementine-panel.png clementine-panel-grey.png)
    foreach(theme ubuntu-mono-dark ubuntu-mono-light)
      install(FILES icons/${theme}/${icon}
        DESTINATION share/icons/${theme}/apps/24/
      )
    endforeach(theme)
  endforeach(icon)
endif (NOT APPLE)
