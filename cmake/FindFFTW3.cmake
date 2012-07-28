# 
# Try to find FFTW3  library  
# (see www.fftw.org)
# Once run this will define: 
# 
# FFTW3_FOUND
# FFTW3_INCLUDE_DIR 
# FFTW3_LIBRARIES
# FFTW3_LINK_DIRECTORIES
#
# You may set one of these options before including this file:
#  FFTW3_USE_SSE2
#
#  TODO: _F_ versions.
#
# Jan Woetzel 05/2004
# www.mip.informatik.uni-kiel.de
# --------------------------------

 FIND_PATH(FFTW3_INCLUDE_DIR fftw3.h
   ${FFTW3_DIR}/include
   ${FFTW3_HOME}/include
   ${FFTW3_DIR}
   ${FFTW3_HOME}
   $ENV{FFTW3_DIR}/include
   $ENV{FFTW3_HOME}/include
   $ENV{FFTW3_DIR}
   $ENV{FFTW3_HOME}
   /usr/include
   /usr/local/include
   $ENV{SOURCE_DIR}/fftw3
   $ENV{SOURCE_DIR}/fftw3/include
   $ENV{SOURCE_DIR}/fftw
   $ENV{SOURCE_DIR}/fftw/include
 )
#MESSAGE("DBG FFTW3_INCLUDE_DIR=${FFTW3_INCLUDE_DIR}")  


SET(FFTW3_POSSIBLE_LIBRARY_PATH
  ${FFTW3_DIR}/lib
  ${FFTW3_HOME}/lib
  ${FFTW3_DIR}
  ${FFTW3_HOME}  
  $ENV{FFTW3_DIR}/lib
  $ENV{FFTW3_HOME}/lib
  $ENV{FFTW3_DIR}
  $ENV{FFTW3_HOME}  
  /usr/lib
  /usr/local/lib
  $ENV{SOURCE_DIR}/fftw3
  $ENV{SOURCE_DIR}/fftw3/lib
  $ENV{SOURCE_DIR}/fftw
  $ENV{SOURCE_DIR}/fftw/lib
)

  
# the lib prefix is containe din filename onf W32, unfortuantely. JW
# teh "general" lib: 
FIND_LIBRARY(FFTW3_FFTW_LIBRARY
  NAMES fftw3 libfftw libfftw3 libfftw3-3
  PATHS 
  ${FFTW3_POSSIBLE_LIBRARY_PATH}
  )
#MESSAGE("DBG FFTW3_FFTW_LIBRARY=${FFTW3_FFTW_LIBRARY}")

FIND_LIBRARY(FFTW3_FFTWF_LIBRARY
  NAMES fftwf3 fftw3f fftwf libfftwf libfftwf3 libfftw3f-3
  PATHS 
  ${FFTW3_POSSIBLE_LIBRARY_PATH}
  )
#MESSAGE("DBG FFTW3_FFTWF_LIBRARY=${FFTW3_FFTWF_LIBRARY}")

FIND_LIBRARY(FFTW3_FFTWL_LIBRARY
  NAMES fftwl3 fftw3l fftwl libfftwl libfftwl3 libfftw3l-3
  PATHS 
  ${FFTW3_POSSIBLE_LIBRARY_PATH}
  )
#MESSAGE("DBG FFTW3_FFTWF_LIBRARY=${FFTW3_FFTWL_LIBRARY}")


FIND_LIBRARY(FFTW3_FFTW_SSE2_LIBRARY
  NAMES fftw_sse2 fftw3_sse2 libfftw_sse2 libfftw3_sse2
  PATHS 
  ${FFTW3_POSSIBLE_LIBRARY_PATH}
  )
#MESSAGE("DBG FFTW3_FFTW_SSE2_LIBRARY=${FFTW3_FFTW_SSE2_LIBRARY}")

FIND_LIBRARY(FFTW3_FFTWF_SSE_LIBRARY
  NAMES fftwf_sse fftwf3_sse libfftwf_sse libfftwf3_sse
  PATHS 
  ${FFTW3_POSSIBLE_LIBRARY_PATH}
  )
#MESSAGE("DBG FFTW3_FFTWF_SSE_LIBRARY=${FFTW3_FFTWF_SSE_LIBRARY}")


# --------------------------------
# select one of the above
# default: 
IF (FFTW3_FFTW_LIBRARY)
  SET(FFTW3_LIBRARIES ${FFTW3_FFTW_LIBRARY})
ENDIF (FFTW3_FFTW_LIBRARY)
# specialized: 
IF (FFTW3_USE_SSE2 AND FFTW3_FFTW_SSE2_LIBRARY)
  SET(FFTW3_LIBRARIES ${FFTW3_FFTW_SSE2_LIBRARY})
ENDIF (FFTW3_USE_SSE2 AND FFTW3_FFTW_SSE2_LIBRARY)

# --------------------------------

IF(FFTW3_LIBRARIES)
  IF (FFTW3_INCLUDE_DIR)

    # OK, found all we need
    SET(FFTW3_FOUND TRUE)
    GET_FILENAME_COMPONENT(FFTW3_LINK_DIRECTORIES ${FFTW3_LIBRARIES} PATH)
    
  ELSE (FFTW3_INCLUDE_DIR)
    MESSAGE("FFTW3 include dir not found. Set FFTW3_DIR to find it.")
  ENDIF(FFTW3_INCLUDE_DIR)
ELSE(FFTW3_LIBRARIES)
  MESSAGE("FFTW3 lib not found. Set FFTW3_DIR to find it.")
ENDIF(FFTW3_LIBRARIES)


MARK_AS_ADVANCED(
  FFTW3_INCLUDE_DIR
  FFTW3_LIBRARIES
  FFTW3_FFTW_LIBRARY
  FFTW3_FFTW_SSE2_LIBRARY
  FFTW3_FFTWF_LIBRARY
  FFTW3_FFTWF_SSE_LIBRARY
  FFTW3_FFTWL_LIBRARY
  FFTW3_LINK_DIRECTORIES
)
