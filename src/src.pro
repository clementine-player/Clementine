# Change this line to install Clementine somewhere else
install_prefix = /usr
VERSION = 0.1
QT += sql \
    network \
    xml \
    opengl
TARGET = clementine
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    player.cpp \
    library.cpp \
    librarybackend.cpp \
    playlist.cpp \
    playlistitem.cpp \
    enginebase.cpp \
    analyzers/baranalyzer.cpp \
    analyzers/analyzerbase.cpp \
    fht.cpp \
    analyzers/blockanalyzer.cpp \
    xine-engine.cpp \
    xine-scope.c \
    sliderwidget.cpp \
    playlistview.cpp \
    backgroundthread.cpp \
    librarywatcher.cpp \
    song.cpp \
    songplaylistitem.cpp \
    libraryview.cpp \
    libraryconfig.cpp \
    systemtrayicon.cpp \
    libraryquery.cpp \
    fileview.cpp \
    fileviewlist.cpp \
    playlistheader.cpp \
    radioitem.cpp \
    radioservice.cpp \
    lastfmservice.cpp \
    radiomodel.cpp \
    lastfmconfig.cpp \
    busyindicator.cpp \
    radioplaylistitem.cpp \
    radioview.cpp \
    lastfmstationdialog.cpp \
    osd.cpp \
    trackslider.cpp \
    edittagdialog.cpp \
    lineedit.cpp \
    multiloadingindicator.cpp \
    somafmservice.cpp \
    settingsdialog.cpp \
    librarydirectorymodel.cpp \
    libraryconfigdialog.cpp \
    lastfmconfigdialog.cpp \
    about.cpp
HEADERS += mainwindow.h \
    player.h \
    library.h \
    librarybackend.h \
    playlist.h \
    playlistitem.h \
    enginebase.h \
    engine_fwd.h \
    analyzers/baranalyzer.h \
    analyzers/analyzerbase.h \
    fht.h \
    analyzers/blockanalyzer.h \
    xine-engine.h \
    xine-scope.h \
    sliderwidget.h \
    playlistview.h \
    backgroundthread.h \
    librarywatcher.h \
    directory.h \
    song.h \
    songmimedata.h \
    songplaylistitem.h \
    libraryview.h \
    libraryitem.h \
    libraryconfig.h \
    systemtrayicon.h \
    libraryquery.h \
    fileview.h \
    fileviewlist.h \
    playlistheader.h \
    simpletreeitem.h \
    radioitem.h \
    radioservice.h \
    lastfmservice.h \
    simpletreemodel.h \
    radiomodel.h \
    lastfmconfig.h \
    busyindicator.h \
    radiomimedata.h \
    radioplaylistitem.h \
    radioview.h \
    lastfmstationdialog.h \
    ../3rdparty/qxt/keymapper_x11.h \
    osd.h \
    trackslider.h \
    edittagdialog.h \
    lineedit.h \
    multiloadingindicator.h \
    somafmservice.h \
    settingsdialog.h \
    librarydirectorymodel.h \
    libraryconfigdialog.h \
    lastfmconfigdialog.h \
    about.h
FORMS += mainwindow.ui \
    libraryconfig.ui \
    fileview.ui \
    lastfmconfig.ui \
    lastfmstationdialog.ui \
    trackslider.ui \
    edittagdialog.ui \
    multiloadingindicator.ui \
    settingsdialog.ui \
    libraryconfigdialog.ui \
    lastfmconfigdialog.ui \
    about.ui
RESOURCES += ../data/data.qrc
OTHER_FILES += ../data/schema.sql \
    ../data/mainwindow.css
LIBS += -llastfm
!win32:!fedora-win32-cross {
    mac { 
        QMAKE_CXXFLAGS += -I/usr/local/include
        LIBS += -L/usr/local/lib \
            -ltag \
            -lxine \
            -framework \
            Carbon
    }
    !mac { 
        QMAKE_CXXFLAGS += $$system(taglib-config --cflags)
        LIBS += $$system(taglib-config --libs)
        QMAKE_CXXFLAGS += $$system(xine-config --cflags)
        LIBS += $$system(xine-config --libs)
        QMAKE_CXXFLAGS += $$system(pkg-config --cflags libnotify)
        LIBS += $$system(pkg-config --libs libnotify)
    }
}
win32|fedora-win32-cross:LIBS += -ltag \
    -lxine \
    -lpthreadGC2
unix:!macx:!fedora-win32-cross:SOURCES += osd_x11.cpp
macx:SOURCES += osd_mac.cpp
win32|fedora-win32-cross:SOURCES += osd_win.cpp

# QXT
INCLUDEPATH += ../3rdparty/qxt
unix:!macx:!fedora-win32-cross: {
    HEADERS += ../3rdparty/qxt/qxtglobalshortcut.h
    HEADERS += ../3rdparty/qxt/qxtglobalshortcut_p.h
    HEADERS += ../3rdparty/qxt/qxtglobal.h
    SOURCES += ../3rdparty/qxt/qxtglobalshortcut.cpp
    SOURCES += ../3rdparty/qxt/qxtglobal.cpp
    unix:!macx:SOURCES += ../3rdparty/qxt/qxtglobalshortcut_x11.cpp
    macx:SOURCES += ../3rdparty/qxt/qxtglobalshortcut_mac.cpp
    win32:SOURCES += ../3rdparty/qxt/qxtglobalshortcut_win.cpp
}

# QtSingleApplication
INCLUDEPATH += ../3rdparty/qtsingleapplication
HEADERS += ../3rdparty/qtsingleapplication/qtlocalpeer.h
HEADERS += ../3rdparty/qtsingleapplication/qtsingleapplication.h
HEADERS += ../3rdparty/qtsingleapplication/qtsinglecoreapplication.h
SOURCES += ../3rdparty/qtsingleapplication/qtsingleapplication.cpp
SOURCES += ../3rdparty/qtsingleapplication/qtsinglecoreapplication.cpp
SOURCES += ../3rdparty/qtsingleapplication/qtlocalpeer.cpp
SOURCES += ../3rdparty/qtsingleapplication/qtlockedfile.cpp
unix:!fedora-win32-cross:SOURCES += ../3rdparty/qtsingleapplication/qtlockedfile_unix.cpp
win32|fedora-win32-cross:SOURCES += ../3rdparty/qtsingleapplication/qtlockedfile_win.cpp

# Hide the console on windows
win32|fedora-win32-cross: {
  CONFIG -= console
  CONFIG += windows
  LIBS += -Wl,-subsystem,windows
}

# Installs
target.path = $${install_prefix}/bin/
desktop.path = dummy
desktop.extra = xdg-icon-resource \
    install \
    --size \
    64 \
    ../dist/clementine_64.png \
    application-x-clementine \
    ; \
    xdg-desktop-menu \
    install \
    --novendor \
    ../dist/clementine.desktop
INSTALLS += target \
    desktop
