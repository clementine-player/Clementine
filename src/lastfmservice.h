#ifndef LASTFMSERVICE_H
#define LASTFMSERVICE_H

#include "radioservice.h"

#include <lastfm/RadioTuner>

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

  // RadioService
  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem *item);
  QList<QUrl> UrlsForItem(RadioItem* item);
  void StartLoading(const QUrl& url);

  void Authenticate(const QString& username, const QString& password);

 signals:
  void AuthenticationComplete(bool success);

 private slots:
  void AuthenticateReplyFinished();

  void TunerTrackAvailable();
  void TunerError(lastfm::ws::Error error);

 private:
  RadioItem* CreateStationItem(ItemType type, const QString& name,
                               const QString& icon, RadioItem* parent);
  QString ErrorString(lastfm::ws::Error error) const;

 private:
  LastFMConfig* config_;
  lastfm::RadioTuner* tuner_;
  QUrl last_url_;
};

#endif // LASTFMSERVICE_H
