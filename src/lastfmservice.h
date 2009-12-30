#ifndef LASTFMSERVICE_H
#define LASTFMSERVICE_H

#include "radioservice.h"
#include "song.h"
#include "lastfmstationdialog.h"

#include <lastfm/RadioTuner>

class QMenu;
class QAction;

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
    Type_ArtistRadio,
    Type_TagRadio,
    Type_MyFriends,
    Type_MyNeighbours,
    Type_OtherUser,
    Type_OtherUserRadio,
    Type_OtherUserLoved,
    Type_OtherUserNeighbourhood,
    Type_Artist,
    Type_Tag,
  };

  // RadioService
  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem *item);

  QUrl UrlForItem(const RadioItem* item) const;
  QString TitleForItem(const RadioItem* item) const;

  void ShowContextMenu(RadioItem *item, const QPoint &global_pos);

  void StartLoading(const QUrl& url);
  void LoadNext(const QUrl& url);

  bool IsPauseAllowed() const { return false; }
  bool ShowLastFmControls() const { return true; }

  // Last.fm specific stuff
  bool IsAuthenticated() const;
  bool IsScrobblingEnabled() const { return scrobbling_enabled_; }

  void Authenticate(const QString& username, const QString& password);

 public slots:
  void NowPlaying(const Song& song);
  void Scrobble();
  void Love();
  void Ban();

 signals:
  void AuthenticationComplete(bool success);
  void ScrobblingEnabledChanged(bool value);

 private slots:
  void AuthenticateReplyFinished();
  void ScrobblingEnabledChangedSlot(bool value);
  void RefreshFriendsFinished();
  void RefreshNeighboursFinished();

  void TunerTrackAvailable();
  void TunerError(lastfm::ws::Error error);

  void AddToPlaylist();
  void AddArtistRadio();
  void AddTagRadio();

 private:
  RadioItem* CreateStationItem(ItemType type, const QString& name,
                               const QString& icon, RadioItem* parent);
  QString ErrorString(lastfm::ws::Error error) const;
  bool InitScrobbler();
  lastfm::Track TrackFromSong(const Song& song) const;
  void RefreshFriends();
  void RefreshNeighbours();
  void AddArtistOrTag(const QString& name,
                      LastFMStationDialog::Type dialog_type, ItemType item_type,
                      const QIcon& icon, RadioItem* list);
  void SaveList(const QString& name, RadioItem* list) const;
  void RestoreList(const QString &name, ItemType item_type,
                   const QIcon& icon, RadioItem *list);

 private:
  lastfm::RadioTuner* tuner_;
  lastfm::Audioscrobbler* scrobbler_;
  lastfm::Track last_track_;

  LastFMConfig* config_;
  LastFMStationDialog* station_dialog_;

  QMenu* context_menu_;
  QAction* play_action_;
  QAction* remove_action_;
  QAction* add_artist_action_;
  QAction* add_tag_action_;
  RadioItem* context_item_;

  QUrl last_url_;
  bool initial_tune_;

  bool scrobbling_enabled_;

  RadioItem* artist_list_;
  RadioItem* tag_list_;
  RadioItem* friends_list_;
  RadioItem* neighbours_list_;
};

#endif // LASTFMSERVICE_H
