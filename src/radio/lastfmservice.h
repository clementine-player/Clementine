/* This file is part of Clementine.

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

namespace lastfm {
class RadioStation;
class Track;
}

#include <QtGlobal>
uint qHash(const lastfm::Track& track);

#include "fixlastfm.h"
#include <lastfm/Track>
#include <lastfm/ws.h>

#include "radioservice.h"
#include "lastfmconfigdialog.h"
#include "lastfmstationdialog.h"
#include "core/song.h"

#include <QMap>
#include <QMenu>
#include <QQueue>

#include <boost/scoped_ptr.hpp>

class QAction;

class QNetworkAccessManager;

class LastFMService : public RadioService {
  Q_OBJECT

 public:
  LastFMService(RadioModel* parent);
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

  void ShowContextMenu(RadioItem *item, const QModelIndex& index,
                       const QPoint &global_pos);

  void StartLoading(const QUrl& url);
  void LoadNext(const QUrl& url);

  bool IsPauseAllowed() const { return false; }
  bool ShowLastFmControls() const { return true; }

  void ReloadSettings();

  virtual QString Icon() { return ":last.fm/lastfm.png"; }

  // Last.fm specific stuff
  bool IsAuthenticated() const;
  bool IsScrobblingEnabled() const { return scrobbling_enabled_; }
  bool AreButtonsVisible() const { return buttons_visible_; }

  void Authenticate(const QString& username, const QString& password);
  void SignOut();

  void FetchMoreTracks();

 public slots:
  void NowPlaying(const Song& song);
  void Scrobble();
  void Love();
  void Ban();

 signals:
  void AuthenticationComplete(bool success);
  void ScrobblingEnabledChanged(bool value);
  void ButtonVisibilityChanged(bool value);

 private slots:
  void AuthenticateReplyFinished();
  void RefreshFriendsFinished();
  void RefreshNeighboursFinished();
  void ShowConfig();

  void TunerTrackAvailable();
  void TunerError(lastfm::ws::Error error);

  void AddToPlaylist();
  void AddArtistRadio();
  void AddTagRadio();
  void Remove();

  // Radio tuner.
  void FetchMoreTracksFinished();
  void TuneFinished();

  void StreamMetadataReady();

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

  void Tune(const lastfm::RadioStation& station);
  void FetchImage(const lastfm::Track& track, const QString& image_url);
  QImage GetImageForTrack(const lastfm::Track& track);

 private:
  lastfm::Audioscrobbler* scrobbler_;
  lastfm::Track last_track_;
  lastfm::Track next_metadata_;
  QQueue<lastfm::Track> playlist_;

  boost::scoped_ptr<LastFMConfigDialog> config_;
  boost::scoped_ptr<LastFMStationDialog> station_dialog_;

  boost::scoped_ptr<QMenu> context_menu_;
  QAction* play_action_;
  QAction* remove_action_;
  QAction* add_artist_action_;
  QAction* add_tag_action_;
  RadioItem* context_item_;

  QUrl last_url_;
  bool initial_tune_;

  bool scrobbling_enabled_;
  bool buttons_visible_;

  RadioItem* artist_list_;
  RadioItem* tag_list_;
  RadioItem* friends_list_;
  RadioItem* neighbours_list_;

  QNetworkAccessManager* network_;
  QHash<lastfm::Track, QNetworkReply*> image_requests_;
};

#endif // LASTFMSERVICE_H
