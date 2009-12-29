#ifndef LASTFMSERVICE_H
#define LASTFMSERVICE_H

#include "radioservice.h"
#include "song.h"

#include <lastfm/RadioTuner>

class QMenu;

class LastFMConfig;

class LastFMService : public RadioService {
  Q_OBJECT

 public:
  LastFMService(QObject* parent = 0);
  ~LastFMService();

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kAudioscrobblerClientId;
  static const char* kApiKey;
  static const char* kSecret;

  enum ItemType {
    Type_MyRecommendations = 1000,
    Type_MyRadio,
    Type_MyLoved,
    Type_MyNeighbourhood,
  };

  // RadioService
  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem *item);
  QList<RadioItem::PlaylistData> DataForItem(RadioItem* item);
  void ShowContextMenu(RadioItem *item, const QPoint &global_pos);
  void StartLoading(const QUrl& url);
  void LoadNext(const QUrl& url);
  bool IsPauseAllowed() const { return false; }
  bool ShowLastFmControls() const { return true; }

  bool IsAuthenticated() const;
  bool IsScrobblingEnabled() const { return scrobbling_enabled_; }
  void Authenticate(const QString& username, const QString& password);

  void NowPlaying(const Song& song);

 public slots:
  void Scrobble();
  void Love();
  void Ban();

 signals:
  void AuthenticationComplete(bool success);
  void ScrobblingEnabledChanged(bool value);

 private slots:
  void AuthenticateReplyFinished();
  void ScrobblingEnabledChangedSlot(bool value);

  void TunerTrackAvailable();
  void TunerError(lastfm::ws::Error error);

 private:
  RadioItem* CreateStationItem(ItemType type, const QString& name,
                               const QString& icon, RadioItem* parent);
  QString ErrorString(lastfm::ws::Error error) const;
  bool InitScrobbler();
  lastfm::Track TrackFromSong(const Song& song) const;

 private:
  lastfm::RadioTuner* tuner_;
  lastfm::Audioscrobbler* scrobbler_;
  lastfm::Track last_track_;

  LastFMConfig* config_;
  QMenu* context_menu_;

  QUrl last_url_;
  bool initial_tune_;

  bool scrobbling_enabled_;
};

#endif // LASTFMSERVICE_H
