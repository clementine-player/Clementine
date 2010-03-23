#ifndef FIXLASTFM_H
#define FIXLASTFM_H

// Include this before <lastfm/Track> to fix a compile error in release mode

#ifdef QT_NO_DEBUG_OUTPUT
# include <QtDebug>
# include <QUrl>
  // Stub this out so lastfm/Track still compiles
  QDebug& operator<<(QDebug&, const QUrl&);
#endif

#endif
