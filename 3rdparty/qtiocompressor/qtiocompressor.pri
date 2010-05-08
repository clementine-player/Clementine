include(../common.pri)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# We need to find zlib.h. If it's not in the system includes, this will
# find the one includes with Qt, assuming a developer-style "in-place" install.
# Otherwise, edit this to point at the directory where zlib.h resides.
INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib

qtiocompressor-uselib:!qtiocompressor-buildlib {
    LIBS += -L$$QTIOCOMPRESSOR_LIBDIR -l$$QTIOCOMPRESSOR_LIBNAME
} else {
    SOURCES += $$PWD/qtiocompressor.cpp
    HEADERS += $$PWD/qtiocompressor.h
}

win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += QT_QTIOCOMPRESSOR_EXPORT
    else:qtiocompressor-uselib:DEFINES += QT_QTIOCOMPRESSOR_IMPORT
}
