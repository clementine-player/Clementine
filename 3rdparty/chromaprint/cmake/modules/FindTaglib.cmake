# - Try to find the Taglib library
# Once done this will define
#
#  TAGLIB_FOUND - system has the taglib library
#  TAGLIB_CFLAGS - the taglib cflags
#  TAGLIB_LIBRARIES - The libraries needed to use taglib

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT TAGLIB_MIN_VERSION)
  set(TAGLIB_MIN_VERSION "1.6")
endif(NOT TAGLIB_MIN_VERSION)

if(NOT WIN32)
    find_program(TAGLIBCONFIG_EXECUTABLE NAMES taglib-config PATHS
       ${BIN_INSTALL_DIR}
    )
endif(NOT WIN32)

#reset vars
set(TAGLIB_LIBRARIES)
set(TAGLIB_CFLAGS)

# if taglib-config has been found
if(TAGLIBCONFIG_EXECUTABLE)

  exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_VERSION)

  if(TAGLIB_VERSION STRLESS "${TAGLIB_MIN_VERSION}")
     message(STATUS "TagLib version too old: version searched :${TAGLIB_MIN_VERSION}, found ${TAGLIB_VERSION}")
     set(TAGLIB_FOUND FALSE)
  else(TAGLIB_VERSION STRLESS "${TAGLIB_MIN_VERSION}")

     exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_LIBRARIES)

     exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_CFLAGS)

     if(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
        set(TAGLIB_FOUND TRUE)
     endif(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
     string(REGEX REPLACE " *-I" ";" TAGLIB_INCLUDES "${TAGLIB_CFLAGS}")
  endif(TAGLIB_VERSION STRLESS "${TAGLIB_MIN_VERSION}") 
  mark_as_advanced(TAGLIB_CFLAGS TAGLIB_LIBRARIES TAGLIB_INCLUDES)

else(TAGLIBCONFIG_EXECUTABLE)

  find_path(TAGLIB_INCLUDES
    NAMES
    tag.h
    PATH_SUFFIXES taglib
    PATHS
    ${KDE4_INCLUDE_DIR}
    ${INCLUDE_INSTALL_DIR}
  )

    IF(NOT WIN32)
      # on non-win32 we don't need to take care about WIN32_DEBUG_POSTFIX

      FIND_LIBRARY(TAGLIB_LIBRARIES tag PATHS ${KDE4_LIB_DIR} ${LIB_INSTALL_DIR})

    ELSE(NOT WIN32)

      # 1. get all possible libnames
      SET(args PATHS ${KDE4_LIB_DIR} ${LIB_INSTALL_DIR})             
      SET(newargs "")               
      SET(libnames_release "")      
      SET(libnames_debug "")        

      LIST(LENGTH args listCount)

        # just one name
        LIST(APPEND libnames_release "tag")
        LIST(APPEND libnames_debug   "tagd")

        SET(newargs ${args})

      # search the release lib
      FIND_LIBRARY(TAGLIB_LIBRARIES_RELEASE
                   NAMES ${libnames_release}
                   ${newargs}
      )

      # search the debug lib
      FIND_LIBRARY(TAGLIB_LIBRARIES_DEBUG
                   NAMES ${libnames_debug}
                   ${newargs}
      )

      IF(TAGLIB_LIBRARIES_RELEASE AND TAGLIB_LIBRARIES_DEBUG)

        # both libs found
        SET(TAGLIB_LIBRARIES optimized ${TAGLIB_LIBRARIES_RELEASE}
                        debug     ${TAGLIB_LIBRARIES_DEBUG})

      ELSE(TAGLIB_LIBRARIES_RELEASE AND TAGLIB_LIBRARIES_DEBUG)

        IF(TAGLIB_LIBRARIES_RELEASE)

          # only release found
          SET(TAGLIB_LIBRARIES ${TAGLIB_LIBRARIES_RELEASE})

        ELSE(TAGLIB_LIBRARIES_RELEASE)

          # only debug (or nothing) found
          SET(TAGLIB_LIBRARIES ${TAGLIB_LIBRARIES_DEBUG})

        ENDIF(TAGLIB_LIBRARIES_RELEASE)

      ENDIF(TAGLIB_LIBRARIES_RELEASE AND TAGLIB_LIBRARIES_DEBUG)

      MARK_AS_ADVANCED(TAGLIB_LIBRARIES_RELEASE)
      MARK_AS_ADVANCED(TAGLIB_LIBRARIES_DEBUG)

    ENDIF(NOT WIN32)
  
  INCLUDE(FindPackageMessage)
  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Taglib DEFAULT_MSG TAGLIB_INCLUDES TAGLIB_LIBRARIES)

endif(TAGLIBCONFIG_EXECUTABLE)


if(TAGLIB_FOUND)
  if(NOT Taglib_FIND_QUIETLY AND TAGLIBCONFIG_EXECUTABLE)
    message(STATUS "Taglib found: ${TAGLIB_LIBRARIES}")
  endif(NOT Taglib_FIND_QUIETLY AND TAGLIBCONFIG_EXECUTABLE)
else(TAGLIB_FOUND)
  if(Taglib_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Taglib")
  endif(Taglib_FIND_REQUIRED)
endif(TAGLIB_FOUND)

