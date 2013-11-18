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

#ifndef MPRIS2_H
#define MPRIS2_H

#include "playlist/playlistitem.h"

#include <QMetaObject>
#include <QObject>
#include <QtDBus>

#include <boost/scoped_ptr.hpp>

class Application;
class MainWindow;
class Playlist;

typedef QList<QVariantMap> TrackMetadata;
typedef QList<QDBusObjectPath> TrackIds;
Q_DECLARE_METATYPE(TrackMetadata)

struct MprisPlaylist {
  QDBusObjectPath id;
  QString name;
  QString icon;  // Uri
};
typedef QList<MprisPlaylist> MprisPlaylistList;
Q_DECLARE_METATYPE(MprisPlaylist);
Q_DECLARE_METATYPE(MprisPlaylistList);

struct MaybePlaylist {
  bool valid;
  MprisPlaylist playlist;
};
Q_DECLARE_METATYPE(MaybePlaylist);

QDBusArgument& operator<< (QDBusArgument& arg, const MprisPlaylist& playlist);
const QDBusArgument& operator>> (
    const QDBusArgument& arg, MprisPlaylist& playlist);

QDBusArgument& operator<< (QDBusArgument& arg, const MaybePlaylist& playlist);
const QDBusArgument& operator>> (
    const QDBusArgument& arg, MaybePlaylist& playlist);


namespace mpris {

class Mpris1;

class Mpris2 : public QObject {
  Q_OBJECT

 public:
  //org.mpris.MediaPlayer2 MPRIS 2.0 Root interface
  Q_PROPERTY( bool CanQuit READ CanQuit )
  Q_PROPERTY( bool CanRaise READ CanRaise )
  Q_PROPERTY( bool HasTrackList READ HasTrackList )
  Q_PROPERTY( QString Identity READ Identity )
  Q_PROPERTY( QString DesktopEntry READ DesktopEntry )
  Q_PROPERTY( QStringList SupportedUriSchemes READ SupportedUriSchemes )
  Q_PROPERTY( QStringList SupportedMimeTypes READ SupportedMimeTypes )

  //org.mpris.MediaPlayer2 MPRIS 2.2 Root interface
  Q_PROPERTY( bool CanSetFullscreen READ CanSetFullscreen )
  Q_PROPERTY( bool Fullscreen READ Fullscreen WRITE SetFullscreen )

  //org.mpris.MediaPlayer2.Player MPRIS 2.0 Player interface
  Q_PROPERTY( QString PlaybackStatus READ PlaybackStatus )
  Q_PROPERTY( QString LoopStatus READ LoopStatus WRITE SetLoopStatus )
  Q_PROPERTY( double Rate READ Rate WRITE SetRate )
  Q_PROPERTY( bool Shuffle READ Shuffle WRITE SetShuffle )
  Q_PROPERTY( QVariantMap Metadata READ Metadata )
  Q_PROPERTY( double Volume READ Volume WRITE SetVolume )
  Q_PROPERTY( qlonglong Position READ Position )
  Q_PROPERTY( double MinimumRate READ MinimumRate )
  Q_PROPERTY( double MaximumRate READ MaximumRate )
  Q_PROPERTY( bool CanGoNext READ CanGoNext )
  Q_PROPERTY( bool CanGoPrevious READ CanGoPrevious )
  Q_PROPERTY( bool CanPlay READ CanPlay )
  Q_PROPERTY( bool CanPause READ CanPause )
  Q_PROPERTY( bool CanSeek READ CanSeek )
  Q_PROPERTY( bool CanControl READ CanControl )

  //org.mpris.MediaPlayer2.TrackList MPRIS 2.0 Player interface
  Q_PROPERTY( TrackIds Tracks READ Tracks )
  Q_PROPERTY( bool CanEditTracks READ CanEditTracks )

  //org.mpris.MediaPlayer2.Playlists MPRIS 2.1 Playlists interface
  Q_PROPERTY( quint32 PlaylistCount READ PlaylistCount )
  Q_PROPERTY( QStringList Orderings READ Orderings )
  Q_PROPERTY( MaybePlaylist ActivePlaylist READ ActivePlaylist )

  Mpris2(Application* app, Mpris1* mpris1, QObject* parent = 0);

  // Root Properties
  bool CanQuit() const;
  bool CanRaise() const;
  bool HasTrackList() const;
  QString Identity() const;
  QString DesktopEntry() const;
  QStringList SupportedUriSchemes() const;
  QStringList SupportedMimeTypes() const;

  // Root Properties added in MPRIS 2.2
  bool CanSetFullscreen() const { return false; }
  bool Fullscreen() const { return false; }
  void SetFullscreen(bool) {}

  // Methods
  void Raise();
  void Quit();

  // Player Properties
  QString PlaybackStatus() const;
  QString LoopStatus() const;
  void SetLoopStatus(const QString& value);
  double Rate() const;
  void SetRate(double value);
  bool Shuffle() const;
  void SetShuffle(bool value);
  QVariantMap Metadata() const;
  double Volume() const;
  void SetVolume(double value);
  qlonglong Position() const;
  double MaximumRate() const;
  double MinimumRate() const;
  bool CanGoNext() const;
  bool CanGoPrevious() const;
  bool CanPlay() const;
  bool CanPause() const;
  bool CanSeek() const;
  bool CanControl() const;

  // Methods
  void Next();
  void Previous();
  void Pause();
  void PlayPause();
  void Stop();
  void Play();
  void Seek(qlonglong offset);
  void SetPosition(const QDBusObjectPath& trackId, qlonglong offset);
  void OpenUri(const QString& uri);

  // TrackList Properties
  TrackIds Tracks() const;
  bool CanEditTracks() const;

  // Methods
  TrackMetadata GetTracksMetadata(const TrackIds& tracks) const;
  void AddTrack(const QString& uri, const QDBusObjectPath& afterTrack, bool setAsCurrent);
  void RemoveTrack(const QDBusObjectPath& trackId);
  void GoTo(const QDBusObjectPath& trackId);

  // Playlist Properties
  quint32 PlaylistCount() const;
  QStringList Orderings() const;
  MaybePlaylist ActivePlaylist() const;

  // Methods
  void ActivatePlaylist(const QDBusObjectPath& playlist_id);
  QList<MprisPlaylist> GetPlaylists(
      quint32 index, quint32 max_count, const QString& order, bool reverse_order);

signals:
  // Player
  void Seeked(qlonglong position);

  // TrackList
  void TrackListReplaced(const TrackIds& Tracks, QDBusObjectPath CurrentTrack);
  void TrackAdded(const TrackMetadata& Metadata, QDBusObjectPath AfterTrack);
  void TrackRemoved(const QDBusObjectPath& trackId);
  void TrackMetadataChanged(const QDBusObjectPath& trackId, const TrackMetadata& metadata);

  void RaiseMainWindow();

  // Playlist
  void PlaylistChanged(const MprisPlaylist& playlist);

private slots:
  void ArtLoaded(const Song& song, const QString& art_uri);
  void EngineStateChanged(Engine::State newState);
  void VolumeChanged();

  void PlaylistManagerInitialized();
  void CurrentSongChanged(const Song& song);
  void ShuffleModeChanged();
  void RepeatModeChanged();
  void PlaylistChanged(Playlist* playlist);
  void PlaylistCollectionChanged(Playlist* playlist);

private:
  void EmitNotification(const QString& name);
  void EmitNotification(const QString& name, const QVariant& val);
  void EmitNotification(const QString& name, const QVariant& val, const QString& mprisEntity);

  QString PlaybackStatus(Engine::State state) const;

  QString current_track_id() const;

  QString DesktopEntryAbsolutePath() const;

private:
  static const char* kMprisObjectPath;
  static const char* kServiceName;
  static const char* kFreedesktopPath;

  QVariantMap last_metadata_;

  Application* app_;
  Mpris1* mpris1_;
};

} // namespace mpris

#endif
