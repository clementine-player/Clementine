include(KDE4Defaults)
include(MacroLibrary)

add_definitions(${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${KDE4_INCLUDES})

set(SOURCES
  clementinerunner.cpp
  ../src/globalsearch/common.cpp
)

set(RESOURCES
  ../data/clementineplasmarunner.qrc
)

list(APPEND QT_DBUSXML2CPP_EXECUTABLE -i src/globalsearch/common.h)

qt4_add_dbus_interface(SOURCES
  ${CMAKE_SOURCE_DIR}/src/dbus/org.clementineplayer.GlobalSearch.xml
  globalsearchinterface)
qt4_add_resources(QRC ${RESOURCES})
kde4_add_plugin(plasma_runner_clementine ${SOURCES} ${QRC})

target_link_libraries(plasma_runner_clementine ${KDE4_PLASMA_LIBS})

# Install the library and .desktop file
install(TARGETS plasma_runner_clementine DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-clementine.desktop DESTINATION ${SERVICES_INSTALL_DIR})

