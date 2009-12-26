#include "lastfmservice.h"
#include "radioitem.h"

LastFMService::LastFMService(QObject* parent)
  : RadioService("Last.fm", parent)
{
}

RadioItem* LastFMService::CreateRootItem(RadioItem* parent) {
  RadioItem* item = new RadioItem(this, RadioItem::Type_Service, "Last.fm", parent);
  item->icon = QIcon(":last.fm/as.png");
  return item;
}

void LastFMService::LazyPopulate(RadioItem *item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      CreateStationItem(Type_MyRecommendations, "My Recommendations",
                        ":last.fm/recommended_radio.png", item);
      CreateStationItem(Type_MyRadio, "My Radio Station",
                        ":last.fm/personal_radio.png", item);
      CreateStationItem(Type_MyLoved, "My Loved Tracks",
                        ":last.fm/loved_radio.png", item);
      CreateStationItem(Type_MyNeighbourhood, "My Neighbourhood",
                        ":last.fm/neighbour_radio.png", item);
  }
}

RadioItem* LastFMService::CreateStationItem(ItemType type, const QString& name,
                                            const QString& icon, RadioItem* parent) {
  RadioItem* ret = new RadioItem(this, type, name, parent);
  ret->lazy_loaded = true;
  ret->icon = QIcon(icon);

  return ret;
}
