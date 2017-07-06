
include_directories( ${QJSON_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set ( LIBMYGPO_QT_SRC
        Config.cpp
        DeviceSyncResult.cpp
        Settings.cpp
        AddRemoveResult.cpp
        Tag.cpp 
        JsonCreator.cpp 
        Podcast.cpp
        Episode.cpp 
        EpisodeAction.cpp
        EpisodeActionList.cpp
        ApiRequest.cpp
        RequestHandler.cpp
        UrlBuilder.cpp
        TagList.cpp
        EpisodeList.cpp
        PodcastList.cpp
        DeviceUpdates.cpp
        DeviceList.cpp
        Device.cpp
        qjsonwrapper/Json.cpp
)

set ( LIBMYGPO_QT_MOC_H
        Podcast.h
        Podcast_p.h
        PodcastList.h
        PodcastList_p.h
        Episode.h
        Episode_p.h
        EpisodeList.h
        EpisodeList_p.h
        Tag.h
        Tag_p.h
        TagList.h
        TagList_p.h
        Device.h
        Device_p.h
        DeviceList.h
        DeviceList_p.h
        DeviceSyncResult.h
        DeviceSyncResult_p.h
        DeviceUpdates.h
        DeviceUpdates_p.h
        EpisodeAction.h
        EpisodeAction_p.h
        EpisodeActionList.h
        EpisodeActionList_p.h
        Settings.h
        Settings_p.h
        AddRemoveResult.h
        AddRemoveResult_p.h
)

set ( LIBMYGPO_QT_INSTALL_H
        ApiRequest.h
        mygpo_export.h
        Config.h
        Podcast.h
        PodcastList.h
        Episode.h
        EpisodeList.h
        Tag.h
        TagList.h
        Device.h
        DeviceList.h
        DeviceSyncResult.h
        DeviceUpdates.h
        EpisodeAction.h
        EpisodeActionList.h
        Settings.h
        AddRemoveResult.h
)

QT_WRAP_CPP(LIBMYGPO_QT_MOC_SRC ${LIBMYGPO_QT_MOC_H} )

add_library( ${MYGPO_QT_TARGET_NAME} STATIC ${LIBMYGPO_QT_SRC} ${LIBMYGPO_QT_MOC_SRC} )

target_link_libraries( ${MYGPO_QT_TARGET_NAME} ${QJSON_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY}  )
qt5_use_modules( ${MYGPO_QT_TARGET_NAME} Core Network )
