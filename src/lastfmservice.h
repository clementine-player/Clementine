#ifndef LASTFMSERVICE_H
#define LASTFMSERVICE_H

#include "radioservice.h"

class LastFMConfig;

class LastFMService : public RadioService {
  Q_OBJECT

 public:
  LastFMService(QObject* parent = 0);
  ~LastFMService();

  static const char* kSettingsGroup;

  enum ItemType {
    Type_MyRecommendations = 1000,
    Type_MyRadio,
    Type_MyLoved,
    Type_MyNeighbourhood,
  };

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem *item);

  void Authenticate(const QString& username, const QString& password);

 signals:
  void AuthenticationComplete(bool success);

 private slots:
  void AuthenticateReplyFinished();

 private:
  RadioItem* CreateStationItem(ItemType type, const QString& name,
                               const QString& icon, RadioItem* parent);

 private:
  LastFMConfig* config_;
};

#endif // LASTFMSERVICE_H
