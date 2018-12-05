cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

set(SINGLEAPP-SOURCES
    qtlocalpeer.cpp
    qtlockedfile.cpp
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

QT4_WRAP_CPP(SINGLEAPP-SOURCES-MOC ${SINGLEAPP-MOC-HEADERS})

ADD_LIBRARY(qtsingleapplication STATIC
    ${SINGLEAPP-SOURCES}
    ${SINGLEAPP-SOURCES-MOC}
)
