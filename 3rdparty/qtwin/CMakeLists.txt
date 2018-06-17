cmake_minimum_required(VERSION 2.8.11)

set(QTWIN-SOURCES
    qtwin.cpp
)

ADD_LIBRARY(qtwin STATIC
    ${QTWIN-SOURCES}
)

target_link_libraries(qtwin
    Qt5::Widgets
)
