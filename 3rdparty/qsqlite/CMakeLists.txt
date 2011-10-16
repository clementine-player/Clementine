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
add_definitions(-DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS3_PARENTHESIS)

add_library(sqlite STATIC sqlite3.c)
set_target_properties(sqlite PROPERTIES COMPILE_FLAGS
  "-Wno-pointer-to-int-cast -Wno-int-to-pointer-cast")

add_library(qsqlite STATIC
   ${SQLITE-SOURCES}
   ${SQLITE-SOURCES-MOC}
   ${SQLITE-WIN32-RESOURCES}
)
target_link_libraries(qsqlite
    sqlite
    ${QT_LIBRARIES}
)
