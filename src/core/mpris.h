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

#ifndef MPRIS_H
#define MPRIS_H

#include "core/player.h"

#include <QDateTime>
#include <QDBusArgument>
#include <QObject>

class Player;


struct DBusStatus {    // From Amarok.
  int play;            // Playing = 0, Paused = 1, Stopped = 2
  int random;          // Linearly = 0, Randomly = 1
  int repeat;          // Go_To_Next = 0, Repeat_Current = 1
  int repeat_playlist; // Stop_When_Finished = 0, Never_Give_Up_Playing = 1, Never_Let_You_Down = 42

  enum MprisPlayState {
    Mpris_Playing = 0,
    Mpris_Paused = 1,
    Mpris_Stopped = 2,
  };
};
Q_DECLARE_METATYPE(DBusStatus);

struct Version {
  quint16 minor;
  quint16 major;
};
Q_DECLARE_METATYPE(Version)

enum DBusCaps {
  NONE                 = 0,
  CAN_GO_NEXT          = 1 << 0,
  CAN_GO_PREV          = 1 << 1,
  CAN_PAUSE            = 1 << 2,
  CAN_PLAY             = 1 << 3,
  CAN_SEEK             = 1 << 4,
  CAN_PROVIDE_METADATA = 1 << 5,
  CAN_HAS_TRACKLIST    = 1 << 6,
};


#ifdef Q_WS_X11
# include <QDBusArgument>
  QDBusArgument& operator<< (QDBusArgument& arg, const DBusStatus& status);
  const QDBusArgument& operator>> (const QDBusArgument& arg, DBusStatus& status);
#endif


namespace metadata {

inline void AddMetadata(const QString& key, const QString& metadata, QVariantMap* map) {
  if (!metadata.isEmpty()) {
    (*map)[key] = metadata;
  }
}

inline void AddMetadata(const QString& key, int metadata, QVariantMap* map) {
  if (metadata > 0) {
    (*map)[key] = metadata;
  }
}

inline void AddMetadata(const QString& key, const QDateTime& metadata, QVariantMap* map) {
  if (metadata.isValid()) {
    (*map)[key] = metadata;
  }
}

inline void AddMetadata(const QString &key, const QStringList& metadata, QVariantMap *map) {
    if (!metadata.isEmpty()) {
      (*map)[key] = metadata;
    }
}

} // namespace metadata

QDBusArgument& operator<< (QDBusArgument& arg, const Version& version);
const QDBusArgument& operator>> (const QDBusArgument& arg, Version& version);

class MPRIS : public QObject {
  Q_OBJECT

public:
  // Root interface
  MPRIS(Player* player, QObject* parent);

  QString Identity();
  void Quit();
  Version MprisVersion();

  // Player Interface
  void Pause();
  void Stop();
  void Prev();
  void Play();
  void Next();
  void Repeat(bool);
  DBusStatus GetStatus() const;
  void VolumeSet(int volume);
  int VolumeGet() const;
  void PositionSet(int pos);
  int PositionGet() const;
  QVariantMap GetMetadata() const;
  QVariantMap GetMetadata(PlaylistItemPtr item) const;
  int GetCaps() const;

  // Amarok Extensions for Player
  void VolumeUp(int vol);
  void VolumeDown(int vol);
  void Mute();
  void ShowOSD();

  // Tracklist Interface
  int AddTrack(const QString&, bool);
  void DelTrack(int index);
  int GetCurrentTrack() const;
  int GetLength() const;
  QVariantMap GetMetadata(int) const;
  void SetLoop(bool enable);
  void SetRandom(bool enable);

  // Amarok extension.
  void PlayTrack(int index);

  // Signals Emitters
  void EmitCapsChange(int param);
  void EmitTrackChange(QVariantMap param);
  void EmitStatusChange(DBusStatus);
  void EmitTrackListChange(int i);

signals:
  void CapsChange(int);
  void TrackChange(QVariantMap);
  void StatusChange(DBusStatus);
  // TrackList
  void TrackListChange(int i);

private:
  Player* player_;
};

#endif // MPRIS_H



