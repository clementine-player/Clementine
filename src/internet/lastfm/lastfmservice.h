/* This file is part of Clementine.
   Copyright 2009-2013, David Sansome <me@davidsansome.com>
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
   Copyright 2012, Kacper "mattrick" Banasik <mattrick@jabster.pl>
   Copyright 2012, Harald Sitter <sitter@kde.org>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef INTERNET_LASTFM_LASTFMSERVICE_H_
#define INTERNET_LASTFM_LASTFMSERVICE_H_

#include <memory>

namespace lastfm {
class Track;
}

#include <QtGlobal>
uint qHash(const lastfm::Track& track);

#include "lastfmcompat.h"

#include "internet/core/scrobbler.h"

class Application;
class LastFMUrlHandler;
class QAction;
class QNetworkAccessManager;
class Song;

class LastFMService : public Scrobbler {
  Q_OBJECT

 public:
  explicit LastFMService(Application* app, QObject* parent = nullptr);
  ~LastFMService();

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kAudioscrobblerClientId;
  static const char* kApiKey;
  static const char* kSecret;
  static const char* kAuthLoginUrl;

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

  void GetToken();
  void Authenticate(const QString& token);
  void SignOut();
  void UpdateSubscriberStatus();

 public slots:
  void NowPlaying(const Song& song);
  void Scrobble();
  void Love();
  void Ban();
  void ShowConfig();
  void ToggleScrobbling();

signals:
  void TokenReceived(bool success, const QString& token);
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
  void GetTokenReplyFinished(QNetworkReply* reply);
  void AuthenticateReplyFinished(QNetworkReply* reply);
  void UpdateSubscriberStatusFinished(QNetworkReply* reply);

  void ScrobblerStatus(int value);

 private:
  QString ErrorString(lastfm::ws::Error error) const;
  bool InitScrobbler();
  lastfm::Track TrackFromSong(const Song& song) const;

  static QUrl FixupUrl(const QUrl& url);

 private:
  std::unique_ptr<lastfm::Audioscrobbler> scrobbler_;
  lastfm::Track last_track_;
  lastfm::Track next_metadata_;
  bool already_scrobbled_;

  QUrl last_url_;

  bool scrobbling_enabled_;
  bool buttons_visible_;
  bool scrobble_button_visible_;
  bool prefer_albumartist_;

  QHash<lastfm::Track, QString> art_urls_;

  // Useful to inform the user that we can't scrobble right now
  bool connection_problems_;

  Application* app_;
};

#endif  // INTERNET_LASTFM_LASTFMSERVICE_H_
