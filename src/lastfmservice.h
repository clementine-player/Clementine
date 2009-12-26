#ifndef LASTFMSERVICE_H
#define LASTFMSERVICE_H

#include "radioservice.h"

class LastFMService : public RadioService {
 public:
  LastFMService(QObject* parent = 0);

  enum ItemType {
    Type_MyRecommendations = 1000,
    Type_MyRadio,
    Type_MyLoved,
    Type_MyNeighbourhood,
  };

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem *item);

 private:
  RadioItem* CreateStationItem(ItemType type, const QString& name,
                               const QString& icon, RadioItem* parent);
};

#endif // LASTFMSERVICE_H
