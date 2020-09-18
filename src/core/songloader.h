/* This file is part of Clementine.
   Copyright 2010, 2012-2014, David Sansome <me@davidsansome.com>
   Copyright 2010-2011, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011-2012, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2014, Alexander Bikadorov <abiku@cs.tu-berlin.de>
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

#ifndef CORE_SONGLOADER_H_
#define CORE_SONGLOADER_H_

#include <gst/gst.h>

#include <QObject>
#include <QThreadPool>
#include <QUrl>
#include <functional>
#include <memory>

#include "core/tagreaderclient.h"
#include "musicbrainz/musicbrainzclient.h"
#include "song.h"

class CueParser;
class LibraryBackendInterface;
class ParserBase;
class Player;
class PlaylistParser;
class PodcastParser;
class CddaSongLoader;

class SongLoader : public QObject {
  Q_OBJECT

 public:
  SongLoader(LibraryBackendInterface* library, const Player* player,
             QObject* parent = nullptr);
  ~SongLoader();

  enum Result {
    Success,
    Error,
    BlockingLoadRequired,
  };

  static const int kDefaultTimeout;

  const QUrl& url() const { return url_; }
  const SongList& songs() const { return songs_; }

  int timeout() const { return timeout_; }
  void set_timeout(int msec) { timeout_ = msec; }

  // If Success is returned the songs are fully loaded. If BlockingLoadRequired
  // is returned LoadFilenamesBlocking() needs to be called next.
  Result Load(const QUrl& url);
  // Loads the files with only filenames. When finished, songs() contains a
  // complete list of all Song objects, but without metadata. This method is
  // blocking, do not call it from the UI thread.
  Result LoadFilenamesBlocking();
  // Completely load songs previously loaded with LoadFilenamesBlocking(). When
  // finished, the Song objects in songs() contain metadata now. This method is
  // blocking, do not call it from the UI thread.
  void LoadMetadataBlocking();
  Result LoadAudioCD();

 signals:
  void AudioCDTracksLoaded();
  void LoadAudioCDFinished(bool success);
  void LoadRemoteFinished();

 private slots:
  void Timeout();
  void StopTypefind();
#ifdef HAVE_AUDIOCD
  void AudioCDTracksLoadedSlot(const SongList& songs);
  void AudioCDTracksTagsLoaded(const SongList& songs);
#endif  // HAVE_AUDIOCD

 private:
  enum State {
    WaitingForType,
    WaitingForMagic,
    WaitingForData,
    Finished,
  };

  Result LoadLocal(const QString& filename);
  Result LoadLocalAsync(const QString& filename);
  void EffectiveSongLoad(Song* song);
  Result LoadLocalPartial(const QString& filename);
  void LoadLocalDirectory(const QString& filename);
  void LoadPlaylist(ParserBase* parser, const QString& filename);

  void AddAsRawStream();

  Result LoadRemote();
  bool LoadRemotePlaylist(const QUrl& url);

  // GStreamer callbacks
  static void TypeFound(GstElement* typefind, uint probability, GstCaps* caps,
                        void* self);
  static GstPadProbeReturn DataReady(GstPad*, GstPadProbeInfo* buf,
                                     gpointer self);
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
  std::function<Result()> preload_func_;
  int timeout_;
  State state_;
  bool success_;
  ParserBase* parser_;
  QString mime_type_;
  bool is_podcast_;
  QByteArray buffer_;
  LibraryBackendInterface* library_;
  const Player* player_;

  std::shared_ptr<GstElement> pipeline_;

  QThreadPool thread_pool_;
};

#endif  // CORE_SONGLOADER_H_
