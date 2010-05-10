cmake_minimum_required(VERSION 2.6)

set(SOURCES
  analyzerbase.cpp
  analyzercontainer.cpp
  baranalyzer.cpp
  blockanalyzer.cpp
  boomanalyzer.cpp
  sonogram.cpp
  turbine.cpp
)

set(HEADERS
  analyzerbase.h
  analyzercontainer.h
  baranalyzer.h
  blockanalyzer.h
  boomanalyzer.h
  sonogram.h
  turbine.h
)

qt4_wrap_cpp(MOC ${HEADERS})

add_library(clementine_analyzers
  ${SOURCES}
  ${MOC}
)

add_translation_source(analyzers ${SOURCES} ${MOC})
