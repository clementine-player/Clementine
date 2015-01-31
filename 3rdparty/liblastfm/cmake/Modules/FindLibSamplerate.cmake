# This file is copyrighted under the BSD-license for buildsystem files of KDE
# copyright 2010, Patrick von Reth <patrick.vonreth@gmail.com>
#
#
# - Try to find the libsamplerate library
# Once done this will define
#
#  LIBSAMPLERATE_FOUND          Set to TRUE if libsamplerate librarys and include directory is found
#  LIBSAMPLERATE_LIBRARY        The libsamplerate librarys
#  LIBSAMPLERATE_INCLUDE_DIR    The libsamplerate include directory

find_package(PkgConfig)
pkg_check_modules(PC_LIBSAMPLERATE QUIET libsamplerate)
set(LIBSAMPLERATE_DEFINITIONS ${PC_LIBSAMPLERATE_CFLAGS_OTHER})

find_library(LIBSAMPLERATE_LIBRARY NAMES samplerate libsamplerate-0 samplerate-0
				HINTS ${PC_LIBSAMPLERATE_LIBDIR} ${PC_LIBSAMPLERATE_LIBRARY_DIRS})

find_path(LIBSAMPLERATE_INCLUDE_DIR samplerate.h
			HINTS ${PC_LIBSAMPLERATE_INCLUDEDIR} ${PC_LIBSAMPLERATE_INCLUDE_DIRS})

if(LIBSAMPLERATE_LIBRARY AND LIBSAMPLERATE_INCLUDE_DIR)
    set(LIBSAMPLERATE_FOUND TRUE)
    message(STATUS "Found libsamplerate ${LIBSAMPLERATE_LIBRARY}")
else(LIBSAMPLERATE_LIBRARY AND LIBSAMPLERATE_PLUGIN_PATH)
    message(STATUS "Could not find libsamplerate, get it http://www.mega-nerd.com/SRC/")
endif(LIBSAMPLERATE_LIBRARY AND LIBSAMPLERATE_INCLUDE_DIR)
