execute_process(COMMAND env LC_ALL="en_US.utf8" date "+%a %b %d %Y" OUTPUT_VARIABLE RPM_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/clementine.spec.in
               ${CMAKE_CURRENT_SOURCE_DIR}/clementine.spec @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
               ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/maketarball.sh.in
               ${CMAKE_CURRENT_SOURCE_DIR}/maketarball.sh @ONLY)

# Create two installers, one for the normal version, one for the portable one
# Create normal installer
set(PORTABLE ";")
set(NORMAL "")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/windows/clementine.nsi.in
               ${CMAKE_CURRENT_SOURCE_DIR}/windows/clementine.nsi @ONLY)

# Create portable installer
set(PORTABLE "")
set(NORMAL ";")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/windows/clementine.nsi.in
               ${CMAKE_CURRENT_SOURCE_DIR}/windows/clementine-portable.nsi @ONLY)


# windows/windres.rc is done by src/CMakeLists.txt

if(EXISTS /etc/lsb-release)
  file(READ "/etc/lsb-release" LSB_RELEASE_CONTENTS)
  string(REGEX MATCH "DISTRIB_ID=Ubuntu" IS_UBUNTU ${LSB_RELEASE_CONTENTS})
  if(IS_UBUNTU)
    set(INSTALL_UBUNTU_ICONS_DEFAULT ON)
  else(IS_UBUNTU)
    set(INSTALL_UBUNTU_ICONS_DEFAULT OFF)
  endif(IS_UBUNTU)
else(EXISTS /etc/lsb-release)
  set(INSTALL_UBUNTU_ICONS_DEFAULT OFF)
endif(EXISTS /etc/lsb-release)

option(INSTALL_UBUNTU_ICONS "Install the Ubuntu themed monochrome panel icons" ${INSTALL_UBUNTU_ICONS_DEFAULT})

if (NOT APPLE)
  install(FILES clementine_64.png
    DESTINATION share/icons/hicolor/64x64/apps/
    RENAME clementine.png
  )

  install(FILES clementine_128.png
    DESTINATION share/icons/hicolor/128x128/apps/
    RENAME clementine.png
  )

  install(FILES ../data/icon.svg
    DESTINATION share/icons/hicolor/scalable/apps/
    RENAME clementine.svg
  )

  install(FILES clementine.desktop
    DESTINATION share/applications
  )

  install(FILES clementine-itms.protocol
                clementine-itpc.protocol
                clementine-feed.protocol
                clementine-zune.protocol
    DESTINATION share/kde4/services
  )

  install(FILES clementine.appdata.xml
    DESTINATION share/metainfo
  )

  if(INSTALL_UBUNTU_ICONS)
    foreach(icon clementine-panel.png clementine-panel-grey.png)
      foreach(theme ubuntu-mono-dark ubuntu-mono-light)
        install(FILES icons/${theme}/${icon}
          DESTINATION share/icons/${theme}/apps/24/
        )
      endforeach(theme)
    endforeach(icon)
  endif(INSTALL_UBUNTU_ICONS)
endif (NOT APPLE)
