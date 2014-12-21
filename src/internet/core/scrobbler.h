/* This file is part of Clementine.
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_CORE_SCROBBLER_H_
#define INTERNET_CORE_SCROBBLER_H_

#include <QObject>

class Song;

class Scrobbler : public QObject {
  Q_OBJECT

 public:
  explicit Scrobbler(QObject* parent = nullptr) {}

  virtual bool IsAuthenticated() const = 0;
  virtual bool IsScrobblingEnabled() const = 0;
  virtual bool AreButtonsVisible() const = 0;
  virtual bool IsScrobbleButtonVisible() const = 0;
  virtual bool PreferAlbumArtist() const = 0;

 public slots:
  virtual void NowPlaying(const Song& song) = 0;
  virtual void Scrobble() = 0;
  virtual void Love() = 0;
  virtual void ToggleScrobbling() = 0;
  virtual void ShowConfig() = 0;

 signals:
  void AuthenticationComplete(bool success, const QString& error_message);
  void ScrobblingEnabledChanged(bool value);
  void ButtonVisibilityChanged(bool value);
  void ScrobbleButtonVisibilityChanged(bool value);
  void ScrobbleSubmitted();
  void ScrobbleError(int value);
};

#endif  // INTERNET_CORE_SCROBBLER_H_
