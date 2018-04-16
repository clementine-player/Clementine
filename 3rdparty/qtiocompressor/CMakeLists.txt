cmake_minimum_required(VERSION 2.8.11)

set(IOCOMPRESSOR-SOURCES
    qtiocompressor.cpp
)

set(IOCOMPRESSOR-MOC-HEADERS
    qtiocompressor.h
)

QT5_WRAP_CPP(IOCOMPRESSOR-SOURCES-MOC ${IOCOMPRESSOR-MOC-HEADERS})

ADD_LIBRARY(qtiocompressor STATIC
    ${IOCOMPRESSOR-SOURCES}
    ${IOCOMPRESSOR-SOURCES-MOC}
)

target_link_libraries(qtiocompressor Qt5::Core)
