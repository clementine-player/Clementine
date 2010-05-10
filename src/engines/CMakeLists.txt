cmake_minimum_required(VERSION 2.6)

set(ENGINES-SOURCES
  enginebase.cpp
)

set(ENGINES-HEADERS
  enginebase.h
)

# lists of engine source files
set(GST_ENGINE_SRC
  gstengine.cpp
  gstequalizer.cpp
  gstenginepipeline.cpp
)
set(GST_ENGINE_MOC
  gstengine.h
  gstenginepipeline.h
)
set(GST_ENGINE_LIB
  GSTREAMER
  GSTREAMER_BASE
)

set(XINE_ENGINE_SRC
  xine-engine.cpp
  xine-scope.c
)
set(XINE_ENGINE_MOC
  xine-engine.h
)

# try to add engines
include(../../AddEngine.cmake)
add_engine(gst "${GST_ENGINE_LIB}" "${GST_ENGINE_SRC}" "${GST_ENGINE_MOC}" ON)
add_engine(vlc LIBVLC vlcengine.cpp vlcengine.h OFF)
add_engine(xine LIBXINE "${XINE_ENGINE_SRC}" "${XINE_ENGINE_MOC}" OFF)
add_engine(qt-phonon QT_PHONON phononengine.cpp phononengine.h OFF)
print_engines()

# need at least 1 engine
if(NOT ENGINES_ENABLED)
  message(FATAL_ERROR "no engine enabled!")
endif(NOT ENGINES_ENABLED)


qt4_wrap_cpp(ENGINES-MOC ${ENGINES-HEADERS})

add_library(clementine_engines
  ${ENGINES-SOURCES}
  ${ENGINES-MOC}
)

add_translation_source(engines ${ENGINES-SOURCES} ${ENGINES-MOC})
