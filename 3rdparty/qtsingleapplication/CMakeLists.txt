cmake_minimum_required(VERSION 3.16.0)

set(SINGLEAPP-SOURCES
    qtlocalpeer.cpp
    qtsingleapplication.cpp
    qtsinglecoreapplication.cpp
)

set(SINGLEAPP-MOC-HEADERS
    qtlocalpeer.h
    qtsingleapplication.h
    qtsinglecoreapplication.h
)

if(WIN32)
  set(SINGLEAPP-SOURCES ${SINGLEAPP-SOURCES} qtlockedfile_win.cpp)
elseif(WIN32)
  set(SINGLEAPP-SOURCES ${SINGLEAPP-SOURCES} qtlockedfile_unix.cpp)
endif(WIN32)

QT6_WRAP_CPP(SINGLEAPP-SOURCES-MOC ${SINGLEAPP-MOC-HEADERS})

ADD_LIBRARY(qtsingleapplication STATIC
    ${SINGLEAPP-SOURCES}
    ${SINGLEAPP-SOURCES-MOC}
)

target_link_libraries(qtsingleapplication Qt6::Core Qt6::Widgets Qt6::Network)
