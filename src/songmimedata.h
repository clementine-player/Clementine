#ifndef SONGMIMEDATA_H
#define SONGMIMEDATA_H

#include <QMimeData>

#include "song.h"

class SongMimeData : public QMimeData {
  Q_OBJECT

 public:
  SongList songs;
};

#endif // SONGMIMEDATA_H
