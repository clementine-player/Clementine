#ifndef RADIOMIMEDATA_H
#define RADIOMIMEDATA_H

#include <QMimeData>

class RadioService;

class RadioMimeData : public QMimeData {
  Q_OBJECT

 public:
  QList<RadioService*> services;
  QList<QString> titles;
};

#endif // RADIOMIMEDATA_H
