cmake_minimum_required(VERSION 2.6)

set(SINGLEAPP-SOURCES
  singleapplication.cpp
  singlecoreapplication.cpp
)

set(SINGLEAPP-MOC-HEADERS
  singleapplication.h
  singlecoreapplication.h
)

QT5_WRAP_CPP(SINGLEAPP-SOURCES-MOC ${SINGLEAPP-MOC-HEADERS})

ADD_LIBRARY(singleapplication STATIC
  ${SINGLEAPP-SOURCES}
  ${SINGLEAPP-SOURCES-MOC}
)

QT5_USE_MODULES(singleapplication Core Network Widgets)
