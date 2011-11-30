
include_directories( ${QJSON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} )

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
        Generator.cpp
        Catalog.cpp
        CatalogUpdateEntry.cpp
        CatalogSong.cpp
        CatalogArtist.cpp
        CatalogItem.cpp
        TypeInformation.cpp
)

set( LIBECHONEST_H
        echonest_export.h
        Track.h
        Song.h
        Artist.h
        Playlist.h
        Config.h
        AudioSummary.h
        ArtistTypes.h
        Util.h
        Catalog.h
        CatalogUpdateEntry.h
        CatalogSong.h
        CatalogArtist.h
        CatalogItem.h
        TypeInformation.h
)

QT4_WRAP_CPP( ${LIBECHONEST_H} )

if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions( -Werror )
endif (CMAKE_COMPILER_IS_GNUCXX)

SET( OS_SPECIFIC_LINK_LIBRARIES "" )
IF( APPLE OR MINGW )
  SET( OS_SPECIFIC_LINK_LIBRARIES ${QJSON_LIBRARIES} )
ELSE( APPLE OR MINGW )
  SET( OS_SPECIFIC_LINK_LIBRARIES ${QJSON_LDFLAGS} )
ENDIF( APPLE OR MINGW )


add_library( echonest STATIC ${LIBECHONEST_SRC} )
target_link_libraries( echonest ${OS_SPECIFIC_LINK_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY}  )

foreach(file ${LIBECHONEST_H} echonest_export.h Util.h)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_CURRENT_BINARY_DIR}/echonest/${file}
    COPYONLY)
endforeach(file ${LIBECHONEST_H})
