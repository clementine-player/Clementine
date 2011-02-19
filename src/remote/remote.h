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

#ifndef REMOTE_H
#define REMOTE_H

#include "core/song.h"

#include <QObject>

#include <xrme/connection.h>
#include <xrme/mediaplayerinterface.h>

class ArtLoader;
class Player;

class Remote : public QObject,
               protected xrme::MediaPlayerInterface {
  Q_OBJECT

public:
  Remote(Player* player, QObject* parent = 0);

  static const int kMaxRetries = 3;

public slots:
  void ReloadSettings();
  void ArtLoaded(const Song&, const QString&, const QImage& image);

signals:
  void Error(const QString& message);

protected:
  // xrme::MediaPlayerInterface
  void PlayPause();
  void Stop();
  void Next();
  void Previous();
  xrme::State state() const;
  QImage album_art() const;

private slots:
  void Connected();
  void Disconnected(const QString& error);

  void SetStateChanged();

private:
  bool is_configured() const;

private:
  Player* player_;
  xrme::Connection* connection_;

  QImage last_image_;
  int retry_count_;
};

#endif // REMOTE_H
