include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-common)
include_directories(${CMAKE_SOURCE_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_BINARY_DIR}/ext/libclementine-tagreader)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(SOURCES
  main.cpp
  tagreaderworker.cpp
)

qt5_wrap_cpp(MOC ${HEADERS})
qt5_add_resources(QRC data/data.qrc)

add_executable(clementine-tagreader
  ${SOURCES}
  ${MOC}
  ${QRC}
)

target_link_libraries(clementine-tagreader
  ${TAGLIB_LIBRARIES}
  libclementine-common
  libclementine-tagreader
  Qt5::Core
  Qt5::Network
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
