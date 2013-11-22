cmake_minimum_required(VERSION 2.6)

add_definitions(-DQT_STATICPLUGIN)

# Source files
set(SQLITE-SOURCES
    qsql_sqlite.cpp
    smain.cpp
    clementinesqlcachedresult.cpp
)

# Header files that have Q_OBJECT in
set(SQLITE-MOC-HEADERS
    qsql_sqlite.h
)

set(SQLITE-WIN32-RESOURCES qsqlite_resource.rc)

qt4_wrap_cpp(SQLITE-SOURCES-MOC ${SQLITE-MOC-HEADERS})

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DQT_PLUGIN -DQT_NO_DEBUG)

find_path(SQLITE_INCLUDE_DIRS sqlite3.h)
find_library(SQLITE_LIBRARIES sqlite3)

if (SQLITE_INCLUDE_DIRS AND SQLITE_LIBRARIES)
  set(SQLITE_FOUND true)
endif()

if (NOT SQLITE_FOUND)
  message(SEND_ERROR "Could not find sqlite3")
endif()

include_directories(${SQLITE_INCLUDE_DIRS})

add_library(qsqlite STATIC
   ${SQLITE-SOURCES}
   ${SQLITE-SOURCES-MOC}
   ${SQLITE-WIN32-RESOURCES}
)
target_link_libraries(qsqlite
    ${SQLITE_LIBRARIES}
    ${QT_LIBRARIES}
)
