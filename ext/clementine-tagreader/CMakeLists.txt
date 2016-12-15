include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -U__STRICT_ANSI__")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(SOURCES
  main.cpp
  tagreaderworker.cpp
)

qt4_wrap_cpp(MOC ${HEADERS})
qt4_add_resources(QRC data/data.qrc)

add_executable(clementine-tagreader
  ${SOURCES}
  ${MOC}
  ${QRC}
)

target_link_libraries(clementine-tagreader
  ${TAGLIB_LIBRARIES}
  libclementine-common
  libclementine-tagreader
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  z
)

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  target_link_libraries(clementine-tagreader
    execinfo
  )
endif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

if(APPLE)
  target_link_libraries(clementine-tagreader
    "-framework Foundation"
  )
endif(APPLE)

if(NOT APPLE)
  # macdeploy.py takes care of this on mac
  install(TARGETS clementine-tagreader
          RUNTIME DESTINATION bin
  )
endif(NOT APPLE)
