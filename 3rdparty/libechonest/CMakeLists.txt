
add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)

set(ECHONEST_LIB_MAJOR_VERSION "0")
set(ECHONEST_LIB_MINOR_VERSION "1")
set(ECHONEST_LIB_PATCH_VERSION "0")
set(ECHONEST_LIB_VERSION "${ECHONEST_LIB_MAJOR_VERSION}.${ECHONEST_LIB_MINOR_VERSION}.${ECHONEST_LIB_PATCH_VERSION}" )


set( ECHONEST_LIB_VERSION_STRING "${ECHONEST_LIB_MAJOR_VERSION}.${ECHONEST_LIB_MINOR_VERSION}.${ECHONEST_LIB_PATCH_VERSION}")

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions( -Wall -Wundef -Wcast-align -Wchar-subscripts -Wpointer-arith
       -Wwrite-strings -Wpacked -Wformat-security -Wmissing-format-attribute
       -Wold-style-cast -Woverloaded-virtual -Wnon-virtual-dtor -Werror )
    if ( NOT WIN32 )
        add_definitions( -fvisibility=hidden )
    endif()
# to be added:
#       -Wshadow
# FIXME we might want this one back in, but Qt 4.4.3 spits gazillions of warnings with it on Linux-64:
#       -Wconversion
endif(CMAKE_COMPILER_IS_GNUCXX)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set( LIBECHONEST_SRC
        Track.cpp
        Song.cpp
        Artist.cpp
        Playlist.cpp
        Config.cpp
        Parsing.cpp
        AudioSummary.cpp
        Util.cpp
        ArtistTypes.cpp
)

set( LIBECHONEST_H
        Track.h
        Song.h
        Artist.h
        Playlist.h
        Config.h
        AudioSummary.h
        ArtistTypes.h
)

QT4_WRAP_CPP( ${LIBECHONEST_H} )

add_library( echonest STATIC ${LIBECHONEST_SRC} )
target_link_libraries( echonest ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY}  )
set_target_properties( echonest PROPERTIES VERSION ${ECHONEST_LIB_VERSION} SOVERSION ${ECHONEST_LIB_VERSION} )

# Clementine: "install" the headers into the binary directory
foreach(file ${LIBECHONEST_H} echonest_export.h Util.h)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file}
                 ${CMAKE_CURRENT_BINARY_DIR}/echonest/${file}
                 COPYONLY)
endforeach(file ${LIBECHONEST_H})
