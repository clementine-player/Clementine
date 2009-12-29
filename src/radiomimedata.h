#ifndef RADIOMIMEDATA_H
#define RADIOMIMEDATA_H

#include <QMimeData>

class RadioItem;

class RadioMimeData : public QMimeData {
  Q_OBJECT

 public:
  QList<RadioItem*> items;
};

#endif // RADIOMIMEDATA_H
