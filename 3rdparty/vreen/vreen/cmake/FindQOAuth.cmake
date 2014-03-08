# Find QOAuth
#
#  QOAuth_FOUND - system has QOAuth
#  QOAuth_INCLUDE_DIR - the QOAuth include directory
#  QOAuth_LIBRARIES - the libraries needed to use QOAuth
#  QOAuth_DEFINITIONS - Compiler switches required for using QOAuth
#
# Copyright © 2010 Harald Sitter <apachelogger@ubuntu.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindLibraryWithDebug)

if (QOAuth_INCLUDE_DIR AND QOAuth_LIBRARIES)
  set(QOAuth_FOUND TRUE)
else (QOAuth_INCLUDE_DIR AND QOAuth_LIBRARIES)
  if (NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_QOAuth QUIET qoauth)
    set(QOAuth_DEFINITIONS ${PC_QOAuth_CFLAGS_OTHER})
  endif (NOT WIN32)

  find_library_with_debug(QOAuth_LIBRARIES
                  WIN32_DEBUG_POSTFIX d
                  NAMES qoauth
                  HINTS ${PC_QOAuth_LIBDIR} ${PC_QOAuth_LIBRARY_DIRS}
                  )

  find_path(QOAuth_INCLUDE_DIR QtOAuth
            HINTS ${PC_QOAuth_INCLUDEDIR} ${PC_QOAuth_INCLUDE_DIRS}
            PATH_SUFFIXES QtOAuth)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(QOAuth
    DEFAULT_MSG
    QOAuth_LIBRARIES
    QOAuth_INCLUDE_DIR
  )

  mark_as_advanced(QOAuth_INCLUDE_DIR QOAuth_LIBRARIES)
endif (QOAuth_INCLUDE_DIR AND QOAuth_LIBRARIES)
