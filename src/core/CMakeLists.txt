cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  albumcoverfetcher.cpp
  albumcoverloader.cpp
  backgroundthread.cpp
  commandlineoptions.cpp
  database.cpp
  fht.cpp
  globalshortcutbackend.cpp
  globalshortcuts.cpp
  gnomeglobalshortcutbackend.cpp
  mergedproxymodel.cpp
  networkaccessmanager.cpp
  player.cpp
  qxtglobalshortcutbackend.cpp
  scopedtransaction.cpp
  settingsprovider.cpp
  song.cpp
  stylesheetloader.cpp
  utilities.cpp
)

set(HEADERS
  albumcoverfetcher.h
  albumcoverloader.h
  backgroundthread.h
  database.h
  globalshortcuts.h
  gnomeglobalshortcutbackend.h
  mergedproxymodel.h
  networkaccessmanager.h
  player.h
)

if(NOT APPLE AND NOT WIN32)
  # MPRIS
  qt4_add_dbus_adaptor(MPRIS-PLAYER-SOURCES
      ../../data/org.freedesktop.MediaPlayer.player.xml
      core/player.h Player mpris_player MprisPlayer)
  qt4_add_dbus_adaptor(MPRIS-ROOT-SOURCES
      ../../data/org.freedesktop.MediaPlayer.root.xml
      core/mpris.h MPRIS mpris_root MprisRoot)
  qt4_add_dbus_adaptor(MPRIS-TRACKLIST-SOURCES
      ../../data/org.freedesktop.MediaPlayer.tracklist.xml
      core/player.h Player mpris_tracklist MprisTrackList)

  # org.freedesktop.Notifications
  qt4_add_dbus_interface(NOTIFICATION-SOURCES
      ../../data/org.freedesktop.Notifications.xml
      notification)
  list(APPEND SOURCES
    ${MPRIS-PLAYER-SOURCES}
    ${MPRIS-ROOT-SOURCES}
    ${MPRIS-TRACKLIST-SOURCES}
    ${NOTIFICATION-SOURCES}
    mpris.cpp
  )
  list(APPEND HEADERS mpris.h)
endif(NOT APPLE AND NOT WIN32)

if(APPLE)
  list(APPEND SOURCES mac_startup.mm)
  include_directories(${SPARKLE})
endif(APPLE)

qt4_wrap_cpp(MOC ${HEADERS})

add_library(clementine_core
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine_core
  clementine_engines
)

add_translation_source(core ${SOURCES} ${MOC})
