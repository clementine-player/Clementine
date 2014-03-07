project(vreen)
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if(COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
endif(COMMAND cmake_policy)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CommonUtils)

option(VREEN_WITH_OAUTH "Enable webkit based oauth authorization" ON)
option(VREEN_WITH_DIRECTAUTH "Enable direct authorization" OFF)
option(VREEN_WITH_QMLAPI "Enable QtQuick bindings for vreen" ON)
option(VREEN_WITH_EXAMPLES "Enable vreen tests" ON)
option(VREEN_WITH_TESTS "Enable vreen examples" ON)
option(VREEN_INSTALL_HEADERS "Install devel headers for vreen" ON)
option(VREEN_DEVELOPER_BUILD "Install devel headers for vreen" OFF)
option(USE_QT5 "Build with Qt 5" OFF)

#TODO check if vars is defined
set(RLIBDIR bin)
set(LIBDIR lib${LIB_SUFFIX})
set(LIB_DESTINATION ${CMAKE_INSTALL_PREFIX}/${LIBDIR})
if(APPLE)
    set(CMAKE_INSTALL_NAME_DIR "${LIB_DESTINATION}") #hack for mac
endif()

set(CMAKE_VREEN_VERSION_MAJOR 0 CACHE INT "Major vk version number" FORCE)
set(CMAKE_VREEN_VERSION_MINOR 9 CACHE INT "Minor vk version number" FORCE)
set(CMAKE_VREEN_VERSION_PATCH 5 CACHE INT "Release vk version number" FORCE)
set(CMAKE_VREEN_VERSION_STRING "${CMAKE_VREEN_VERSION_MAJOR}.${CMAKE_VREEN_VERSION_MINOR}.${CMAKE_VREEN_VERSION_PATCH}" CACHE STRING "vreen version string" FORCE)

set(VREEN_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include)
set(VREEN_PRIVATE_INCLUDE_DIR ${VREEN_INCLUDE_DIR}/vreen/${CMAKE_VREEN_VERSION_STRING})

add_subdirectory(src)
if(VREEN_WITH_EXAMPLES)
    add_subdirectory(examples)
endif()
if(VREEN_WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
