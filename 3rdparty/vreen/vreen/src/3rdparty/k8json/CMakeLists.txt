project(k8json)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_LIBK8JSON_VERSION_MAJOR 1 CACHE INT "Major k8json version number" FORCE)
set(CMAKE_LIBK8JSON_VERSION_MINOR 0 CACHE INT "Minor k8json version number" FORCE)
set(CMAKE_LIBK8JSON_VERSION_PATCH 0 CACHE INT "Release k8json version number" FORCE)
set(CMAKE_LIBK8JSON_VERSION_STRING "${CMAKE_LIBK8JSON_VERSION_MAJOR}.${CMAKE_LIBK8JSON_VERSION_MINOR}.${CMAKE_LIBK8JSON_VERSION_PATCH}" CACHE STRING "k8json version string" FORCE)

add_definitions(-DK8JSON_LIB_MAKEDLL -DK8JSON_INCLUDE_GENERATOR -DK8JSON_INCLUDE_COMPLEX_GENERATOR)

set(CMAKE_INSTALL_NAME_DIR ${LIB_INSTALL_DIR})
set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )
set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" CACHE STRING "Library directory name" FORCE)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(Qt4 REQUIRED)
include_directories(${QT_INCLUDES})

# mingw can't handle exported explicit template instantiations in a DLL
if (MINGW)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols ${CMAKE_SHARED_LINKER_FLAGS}")
endif (MINGW)

set(k8json_SRCS
    k8json.cpp
)

add_library(k8json SHARED ${k8json_SRCS})

set_target_properties(k8json PROPERTIES
    VERSION ${CMAKE_LIBK8JSON_VERSION_STRING}
    SOVERSION ${CMAKE_LIBK8JSON_VERSION_MAJOR}
    LINK_INTERFACE_LIBRARIES ""
    DEFINE_SYMBOL K8JSON_LIB_MAKEDLL
)

target_link_libraries(k8json ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})


install(TARGETS k8json ARCHIVE DESTINATION ${LIB_DESTINATION}
                               LIBRARY DESTINATION ${LIB_DESTINATION}
                               RUNTIME DESTINATION bin)

install(FILES
    k8json.h
    DESTINATION include/k8json COMPONENT Devel
)

# Install package config file
if(NOT WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/k8json.pc.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/k8json.pc
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/k8json.pc
        DESTINATION ${LIB_DESTINATION}/pkgconfig
    )
endif(NOT WIN32)

