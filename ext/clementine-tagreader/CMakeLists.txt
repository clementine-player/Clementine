include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(SOURCES
  fmpsparser.cpp
  main.cpp
  tagreaderworker.cpp
)

set(HEADERS
)

optional_source(HAVE_GOOGLE_DRIVE
  INCLUDE_DIRECTORIES ${SPARSEHASH_INCLUDE_DIRS}
  SOURCES
    googledrivestream.cpp
)

qt4_wrap_cpp(MOC ${HEADERS})

add_executable(clementine-tagreader
  ${SOURCES}
  ${MOC}
)

target_link_libraries(clementine-tagreader
  ${TAGLIB_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  libclementine-common
  libclementine-tagreader
)

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  target_link_libraries(clementine-tagreader
    execinfo
  )
endif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

if(APPLE)
  target_link_libraries(clementine-tagreader
    /System/Library/Frameworks/Foundation.framework
  )
endif(APPLE)

if(NOT APPLE)
  # macdeploy.py takes care of this on mac
  install(TARGETS clementine-tagreader
          RUNTIME DESTINATION bin
  )
endif(NOT APPLE)
