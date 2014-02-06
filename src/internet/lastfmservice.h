/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LASTFMSERVICE_H
#define LASTFMSERVICE_H

#include <memory>

namespace lastfm {
class RadioStation;
class Track;
}

#include <QtGlobal>
uint qHash(const lastfm::Track& track);

#include "lastfmcompat.h"

#include "internetmodel.h"
#include "internetservice.h"
#include "lastfmstationdialog.h"
#include "core/cachedlist.h"
#include "core/song.h"
#include "playlist/playlistitem.h"

#include <QDateTime>
#include <QMap>
#include <QMenu>
#include <QQueue>

class LastFMUrlHandler;

class QAction;
class QNetworkAccessManager;

class LastFMService : public InternetService {
  Q_OBJECT
  friend class LastFMUrlHandler;

 public:
  LastFMService(Application* app, InternetModel* parent);
  ~LastFMService();

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kAudioscrobblerClientId;
  static const char* kApiKey;
  static const char* kSecret;

  static const char* kUrlArtist;
  static const char* kUrlTag;
  static const char* kUrlCustom;

  static const char* kTitleArtist;
  static const char* kTitleTag;
  static const char* kTitleCustom;

  static const int kFriendsCacheDurationSecs;

  enum ItemType {
    Type_Root = InternetModel::TypeCount,
    Type_Artists,
    Type_Tags,
    Type_Custom,
    Type_Friends,
    Type_Neighbours,
    Type_OtherUser,
  };

  // InternetService
  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);

  void ShowContextMenu(const QPoint &global_pos);

  PlaylistItem::Options playlistitem_options() const;

  void ReloadSettings();

  virtual QString Icon() { return ":last.fm/lastfm.png"; }

  // Last.fm specific stuff
  bool IsAuthenticated() const;
  bool IsSubscriber() const;
  bool IsScrobblingEnabled() const { return scrobbling_enabled_; }
  bool AreButtonsVisible() const { return buttons_visible_; }
  bool IsScrobbleButtonVisible() const { return scrobble_button_visible_; }
  bool PreferAlbumArtist() const { return prefer_albumartist_; }
  bool HasConnectionProblems() const { return connection_problems_; }

  void Authenticate(const QString& username, const QString& password);
  void SignOut();
  void UpdateSubscriberStatus();

  void FetchMoreTracks();
  QUrl DeququeNextMediaUrl();

  PlaylistItemPtr PlaylistItemForUrl(const QUrl& url);

  bool IsFriendsListStale() const { return friend_names_.IsStale(); }

  // Thread safe
  QStringList FriendNames();
  QStringList SavedArtistRadioNames() const;
  QStringList SavedTagRadioNames() const;

 public slots:
  void NowPlaying(const Song& song);
  void Scrobble();
  void Love();
  void Ban();
  void ShowConfig();
  void ToggleScrobbling();

 signals:
  void AuthenticationComplete(bool success, const QString& error_message);
  void ScrobblingEnabledChanged(bool value);
  void ButtonVisibilityChanged(bool value);
  void ScrobbleButtonVisibilityChanged(bool value);
  void PreferAlbumArtistChanged(bool value);
  void ScrobbleSubmitted();
  void ScrobbleError(int value);
  void UpdatedSubscriberStatus(bool is_subscriber);
  void ScrobbledRadioStream();

  void SavedItemsChanged();

 private slots:
  void AuthenticateReplyFinished(QNetworkReply* reply);
  void UpdateSubscriberStatusFinished(QNetworkReply* reply);
  void RefreshFriendsFinished(QNetworkReply* reply);
  void RefreshNeighboursFinished(QNetworkReply* reply);

  void TunerTrackAvailable();
  void TunerError(lastfm::ws::Error error);
  void ScrobblerStatus(int value);

  void AddArtistRadio();
  void AddTagRadio();
  void AddCustomRadio();
  void ForceRefreshFriends();
  void RefreshFriends();
  void Remove();

  // Radio tuner.
  void FetchMoreTracksFinished(QNetworkReply* reply);
  void TuneFinished(QNetworkReply* reply);

  void StreamMetadataReady();

 private:
  QStandardItem* CreateStationItem(QStandardItem* parent,
      const QString& name, const QString& icon, const QUrl& url,
      const QString& title);
  QString ErrorString(lastfm::ws::Error error) const;
  bool InitScrobbler();
  lastfm::Track TrackFromSong(const Song& song) const;
  void RefreshFriends(bool force);
  void RefreshNeighbours();
  void AddArtistOrTag(const QString& name,
                      LastFMStationDialog::Type dialog_type,
                      const QString& url_pattern,
                      const QString& title_pattern,
                      const QString& icon, QStandardItem* list);
  void SaveList(const QString& name, QStandardItem* list) const;
  void RestoreList(const QString& name,
                   const QString& url_pattern,
                   const QString& title_pattern,
                   const QIcon& icon, QStandardItem* parent);

  static QUrl FixupUrl(const QUrl& url);
  void Tune(const QUrl& station);

  void PopulateFriendsList();

  void AddSelectedToPlaylist(bool clear_first);

 private:
  LastFMUrlHandler* url_handler_;

  lastfm::Audioscrobbler* scrobbler_;
  lastfm::Track last_track_;
  lastfm::Track next_metadata_;
  QQueue<lastfm::Track> playlist_;
  bool already_scrobbled_;

  std::unique_ptr<LastFMStationDialog> station_dialog_;

  std::unique_ptr<QMenu> context_menu_;
  QAction* remove_action_;
  QAction* add_artist_action_;
  QAction* add_tag_action_;
  QAction* add_custom_action_;
  QAction* refresh_friends_action_;

  QUrl last_url_;
  bool initial_tune_;
  int tune_task_id_;

  bool scrobbling_enabled_;
  bool buttons_visible_;
  bool scrobble_button_visible_;
  bool prefer_albumartist_;

  QStandardItem* root_item_;
  QStandardItem* artist_list_;
  QStandardItem* tag_list_;
  QStandardItem* custom_list_;
  QStandardItem* friends_list_;
  QStandardItem* neighbours_list_;

  QHash<lastfm::Track, QString> art_urls_;

  CachedList<QString> friend_names_;

  // Useful to inform the user that we can't scrobble right now
  bool connection_problems_;
};

#endif // LASTFMSERVICE_H
