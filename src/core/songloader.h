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

#ifndef SONGLOADER_H
#define SONGLOADER_H

#include <memory>

#include <gst/gst.h>

#include <QObject>
#include <QThreadPool>
#include <QUrl>

#include "song.h"
#include "core/tagreaderclient.h"
#include "musicbrainz/musicbrainzclient.h"

class CueParser;
class LibraryBackendInterface;
class ParserBase;
class PlaylistParser;
class PodcastParser;

class SongLoader : public QObject {
  Q_OBJECT
public:
  SongLoader(LibraryBackendInterface* library, QObject* parent = 0);
  ~SongLoader();

  enum Result {
    Success,
    Error,
    WillLoadAsync,
  };

  static const int kDefaultTimeout;

  const QUrl& url() const { return url_; }
  const SongList& songs() const { return songs_; }

  int timeout() const { return timeout_; }
  void set_timeout(int msec) { timeout_ = msec; }

  Result Load(const QUrl& url);
  // To effectively load the songs:
  // when we call Load() on a directory, it will return WillLoadAsync, load the
  // files with only filenames and emit LoadFinished(). When LoadFinished() is
  // received by songloaderinserter, it will insert songs (incompletely loaded)
  // in playlist, and call EffectiveSongsLoad() in a background thread to
  // perform the real load of the songs. Next, UpdateItems() will be called on
  // playlist and replace the partially-loaded items by the new ones, fully
  // loaded.
  void EffectiveSongsLoad();
  void EffectiveSongLoad(Song* song);
  Result LoadAudioCD();

signals:
  void LoadFinished(bool success);

private slots:
  void Timeout();
  void StopTypefind();
  void AudioCDTagsLoaded(const QString& artist, const QString& album,
                         const MusicBrainzClient::ResultList& results);
  void LocalFileLoaded(TagReaderReply* reply);

private:
  enum State {
    WaitingForType,
    WaitingForMagic,
    WaitingForData,
    Finished,
  };

  Result LoadLocal(const QString& filename);
  Result LoadLocalPartial(const QString& filename);
  void LoadLocalDirectory(const QString& filename);
  void LoadPlaylist(ParserBase* parser, const QString& filename);
  void LoadLocalDirectoryAndEmit(const QString& filename);
  void LoadPlaylistAndEmit(ParserBase* parser, const QString& filename);

  void AddAsRawStream();

  Result LoadRemote();

  // GStreamer callbacks
  static void TypeFound(GstElement* typefind, uint probability, GstCaps* caps, void* self);
  static gboolean DataReady(GstPad*, GstBuffer* buf, void* self);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage*, gpointer);
  static gboolean BusCallback(GstBus*, GstMessage*, gpointer);

  void StopTypefindAsync(bool success);
  void ErrorMessageReceived(GstMessage* msg);
  void EndOfStreamReached();
  void MagicReady();
  bool IsPipelinePlaying();

private:
  static QSet<QString> sRawUriSchemes;

  QUrl url_;
  SongList songs_;

  QTimer* timeout_timer_;
  PlaylistParser* playlist_parser_;
  PodcastParser* podcast_parser_;
  CueParser* cue_parser_;

  // For async loads
  int timeout_;
  State state_;
  bool success_;
  ParserBase* parser_;
  QString mime_type_;
  bool is_podcast_;
  QByteArray buffer_;
  LibraryBackendInterface* library_;

  std::shared_ptr<GstElement> pipeline_;

  QThreadPool thread_pool_;
};

#endif // SONGLOADER_H
