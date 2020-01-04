cmake_minimum_required( VERSION 2.8.11 FATAL_ERROR )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII" )

set(MYGPO_QT_VERSION_SUFFIX 5)

macro(qt_wrap_cpp)
    qt5_wrap_cpp(${ARGN})
endmacro()

# pkg-config names of QtCore and QtNetwork are Qt5Core and Qt5Network for
# Qt5
set(MYGPO_QT_MAJOR_VERSION "5")
set(MYGPO_QT4_QJSON_DEP "")

# Don't use absolute path in Mygpo-qtTargets-$buildType.cmake
# (This will have no effect with CMake < 2.8)
# set(QT_USE_IMPORTED_TARGETS TRUE)

set( MYGPO_QT_VERSION_MAJOR "1" )
set( MYGPO_QT_VERSION_MINOR "1" )
set( MYGPO_QT_VERSION_PATCH "0" )
set( MYGPO_QT_VERSION "${MYGPO_QT_VERSION_MAJOR}.${MYGPO_QT_VERSION_MINOR}.${MYGPO_QT_VERSION_PATCH}" )

#Configure Version.h.in with the actual version number
configure_file( src/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/Version.h )

include(CheckCXXCompilerFlag)

if (CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS( -Wcast-align -Wchar-subscripts -Wpointer-arith
       -Wwrite-strings -Wpacked -Wformat-security -Wmissing-format-attribute
       -Wold-style-cast -Woverloaded-virtual -Wnon-virtual-dtor  -Wall -Wextra
       -Wformat=2 -Wundef -Wstack-protector -Wmissing-include-dirs
       -Winit-self -Wunsafe-loop-optimizations  -ggdb3 -fno-inline -DQT_STRICT_ITERATORS )
    if ( NOT WIN32 )
        add_definitions( -fvisibility=hidden )
    endif()

    check_cxx_compiler_flag( -Wlogical-op GNUCXX_HAS_WLOGICAL_OP )
    if ( GNUCXX_HAS_WLOGICAL_OP )
        add_definitions( -Wlogical-op )
    endif()
endif(CMAKE_COMPILER_IS_GNUCXX)

set(MYGPO_QT_TARGET_NAME mygpo-qt${MYGPO_QT_VERSION_SUFFIX} CACHE INTERNAL "" FORCE )

add_subdirectory( src )
