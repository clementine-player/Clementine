/* This file is part of Clementine.
   Copyright 2010-2014, David Sansome <me@davidsansome.com>
   Copyright 2010-2014, John Maguire <john.maguire@gmail.com>
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

#include "songloader.h"

#include <QBuffer>
#include <QDirIterator>
#include <QFileInfo>
#include <QScopeGuard>
#include <QTimer>
#include <QUrl>
#include <QtDebug>
#include <algorithm>
#include <memory>

#include "config.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/signalchecker.h"
#include "core/song.h"
#include "core/tagreaderclient.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "core/waitforsignal.h"
#include "engines/gstengine.h"
#include "internet/core/internetmodel.h"
#include "internet/podcasts/podcastparser.h"
#include "internet/podcasts/podcastservice.h"
#include "internet/podcasts/podcasturlloader.h"
#include "library/librarybackend.h"
#include "library/sqlrow.h"
#include "playlistparsers/cueparser.h"
#include "playlistparsers/parserbase.h"
#include "playlistparsers/playlistparser.h"
#include "utilities.h"

#ifdef HAVE_AUDIOCD
#include <gst/audio/gstaudiocdsrc.h>

#include "devices/cddasongloader.h"
#endif

using std::placeholders::_1;

QSet<QString> SongLoader::sRawUriSchemes;
const int SongLoader::kDefaultTimeout = 5000;
// 50 MiB accommodates large podcast feeds without unbounded remote buffering.
const int SongLoader::kMaxRemotePlaylistSize = 50 * 1024 * 1024;

SongLoader::SongLoader(LibraryBackendInterface* library, const Player* player,
                       QObject* parent)
    : QObject(parent),
      timeout_timer_(new QTimer(this)),
      playlist_parser_(new PlaylistParser(library, this)),
      podcast_parser_(new PodcastParser),
      cue_parser_(new CueParser(library, this)),
      timeout_(kDefaultTimeout),
      state_(WaitingForType),
      success_(false),
      parser_(nullptr),
      is_podcast_(false),
      remote_finished_(false),
      library_(library),
      player_(player) {
  if (sRawUriSchemes.isEmpty()) {
    sRawUriSchemes << "udp"
                   << "mms"
                   << "mmsh"
                   << "mmst"
                   << "mmsu"
                   << "rtsp"
                   << "rtspu"
                   << "rtspt"
                   << "rtsph";
  }

  timeout_timer_->setSingleShot(true);

  connect(timeout_timer_, SIGNAL(timeout()), SLOT(Timeout()));
}

SongLoader::~SongLoader() {
  std::shared_ptr<GstElement> pipeline;
  {
    // Move pipeline_ out and mark Finished under mutex_, then detach the bus
    // handler and stop GStreamer outside mutex_ so callbacks cannot deadlock.
    QMutexLocker l(&mutex_);
    state_ = Finished;
    pipeline = std::move(pipeline_);
  }

  if (pipeline) {
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline.get()));
    if (bus) {
      gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
      gst_object_unref(bus);
    }
    gst_element_set_state(pipeline.get(), GST_STATE_NULL);
  }

  delete podcast_parser_;
}

SongLoader::Result SongLoader::Load(const QUrl& url) {
  url_ = url;

  if (url_.scheme() == "file") {
    return LoadLocal(url_.toLocalFile());
  }

  if (sRawUriSchemes.contains(url_.scheme()) ||
      player_->HandlerForUrl(url) != nullptr) {
    // The URI scheme indicates that it can't possibly be a playlist, or we have
    // a custom handler for the URL, so add it as a raw stream.
    AddAsRawStream();
    return Success;
  }

  // It could be a playlist, we give it a shot.
  if (LoadRemotePlaylist(url_)) {
    return Success;
  }

  url_ = PodcastUrlLoader::FixPodcastUrl(url_);

  preload_func_ = std::bind(&SongLoader::LoadRemote, this);
  return BlockingLoadRequired;
}

SongLoader::Result SongLoader::LoadFilenamesBlocking() {
  if (preload_func_) {
    return preload_func_();
  } else {
    qLog(Error) << "Preload function was not set for blocking operation";
    return Error;
  }
}

SongLoader::Result SongLoader::LoadLocalPartial(const QString& filename) {
  qLog(Debug) << "Fast Loading local file" << filename;
  // First check to see if it's a directory - if so we can load all the songs
  // inside right away.
  if (QFileInfo(filename).isDir()) {
    LoadLocalDirectory(filename);
    return Success;
  }
  Song song;
  song.InitFromFilePartial(filename);
  if (song.is_valid()) songs_ << song;
  return Success;
}

SongLoader::Result SongLoader::LoadAudioCD() {
#ifdef HAVE_AUDIOCD
  // Parent the loader and pass `this` as the lambda receiver so destroying
  // SongLoader early disconnects the signal and avoids a dangling `this`.
  CddaSongLoader* cdda_song_loader = new CddaSongLoader(QUrl(), this);
  connect(cdda_song_loader, &CddaSongLoader::SongsUpdated, this,
          &SongLoader::AudioCDTracksLoadedSlot);
  connect(cdda_song_loader, &CddaSongLoader::Finished, this,
          [this, cdda_song_loader]() {
            cdda_song_loader->deleteLater();
            emit LoadAudioCDFinished(true);
          });
  cdda_song_loader->LoadSongs();
  return Success;
#else  // HAVE_AUDIOCD
  return Error;
#endif
}

#ifdef HAVE_AUDIOCD
void SongLoader::AudioCDTracksLoadedSlot(const SongList& songs) {
  songs_ = songs;
  emit AudioCDTracksLoaded();
}
#endif  // HAVE_AUDIOCD

SongLoader::Result SongLoader::LoadLocal(const QString& filename) {
  qLog(Debug) << "Loading local file" << filename;

  // Search in the database.
  QUrl url = QUrl::fromLocalFile(filename);

  LibraryQuery query;
  query.SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);
  query.AddWhere("filename", url.toEncoded());

  if (library_->ExecQuery(&query) && query.Next()) {
    // we may have many results when the file has many sections
    do {
      Song song;
      song.InitFromQuery(query, true);

      if (song.is_valid()) {
        songs_ << song;
      }
    } while (query.Next());

    return Success;
  }

  // It's not in the database, load it asynchronously.
  preload_func_ = std::bind(&SongLoader::LoadLocalAsync, this, filename);
  return BlockingLoadRequired;
}

SongLoader::Result SongLoader::LoadLocalAsync(const QString& filename) {
  // First check to see if it's a directory - if so we will load all the songs
  // inside right away.
  QFileInfo info(filename);
  if (info.isDir()) {
    LoadLocalDirectory(filename);
    return Success;
  }

  // It's a local file, so check if it looks like a playlist.
  // Read the first few bytes.
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    qLog(Error) << "Could not open file " << filename;
    return Error;
  }
  QByteArray data(file.read(PlaylistParser::kMagicSize));

  ParserBase* parser = playlist_parser_->ParserForMagic(data);
  if (!parser) {
    // Check the file extension as well, maybe the magic failed, or it was a
    // basic M3U file which is just a plain list of filenames.
    parser = playlist_parser_->ParserForExtension(
        QFileInfo(filename).suffix().toLower());
  }

  if (parser) {
    qLog(Debug) << "Parsing using" << parser->name();

    // It's a playlist!
    file.reset();
    songs_ = parser->Load(&file, filename, info.path());
    return Success;
  }

  // Check if it's a cue file
  QString matching_cue = filename.section('.', 0, -2) + ".cue";
  if (QFile::exists(matching_cue)) {
    // it's a cue - create virtual tracks
    QFile cue(matching_cue);
    if (!cue.open(QIODevice::ReadOnly)) {
      // Fall through to load the raw audio file if the CUE cannot be opened.
      qLog(Warning) << "Failed to open CUE sheet" << matching_cue;
    } else {
      // Use info.dir() instead of splitting '/' so Windows paths work.
      SongList song_list = cue_parser_->Load(&cue, matching_cue, info.dir());
      for (const Song& song : song_list) {
        if (song.is_valid()) songs_ << song;
      }
      return Success;
    }
  }

  // Assume it's just a normal file
  Song song;
  song.InitFromFilePartial(filename);
  if (song.is_valid()) {
    songs_ << song;
    return Success;
  } else {
    qLog(Error) << "File" << filename
                << "is not recognized by TagLib as a valid audio file.";
    return Error;
  }
}

void SongLoader::LoadMetadataBlocking() {
  for (int i = 0; i < songs_.size(); i++) {
    EffectiveSongLoad(&songs_[i]);
  }
}

void SongLoader::EffectiveSongLoad(Song* song) {
  if (!song) return;

  if (song->filetype() != Song::Type_Unknown) {
    // Maybe we loaded the metadata already, for example from a cuesheet.
    return;
  }

  // First, try to get the song from the library
  Song library_song = library_->GetSongByUrl(song->url());
  if (library_song.is_valid()) {
    *song = library_song;
  } else {
    // it's a normal media file
    QString filename = song->url().toLocalFile();
    TagReaderClient::Instance()->ReadFileBlocking(filename, song);
  }
}

static bool CompareSongs(const Song& left, const Song& right) {
  // Order by artist, album, disc, track
  if (left.artist() < right.artist()) return true;
  if (left.artist() > right.artist()) return false;
  if (left.album() < right.album()) return true;
  if (left.album() > right.album()) return false;
  if (left.disc() < right.disc()) return true;
  if (left.disc() > right.disc()) return false;
  if (left.track() < right.track()) return true;
  if (left.track() > right.track()) return false;
  return left.url() < right.url();
}

void SongLoader::LoadLocalDirectory(const QString& filename) {
  QDirIterator it(filename, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                  QDirIterator::Subdirectories);

  while (it.hasNext()) {
    LoadLocalPartial(it.next());
  }

  std::stable_sort(songs_.begin(), songs_.end(), CompareSongs);

  // Load the first song: all songs will be loaded async, but we want the first
  // one in our list to be fully loaded, so if the user has the "Start playing
  // when adding to playlist" preference behaviour set, it can enjoy the first
  // song being played (seek it, have moodbar, etc.)
  if (!songs_.isEmpty()) EffectiveSongLoad(&(*songs_.begin()));
}

void SongLoader::AddAsRawStream() {
  Song song;
  song.set_valid(true);
  song.set_filetype(Song::Type_Stream);
  song.set_url(url_);
  song.set_title(url_.toString());
  songs_ << song;
}

void SongLoader::Timeout() {
  qLog(Warning) << "Timed out loading" << url_.toString();
  bool should_stop = false;
  {
    QMutexLocker l(&mutex_);
    should_stop = StopTypefindAsyncLocked(false);
  }
  if (should_stop) {
    StopTypefind();
  }
}

void SongLoader::StopTypefind() {
  std::shared_ptr<GstElement> pipeline;
  bool success = false;
  ParserBase* parser = nullptr;
  bool is_podcast = false;
  QByteArray buffer;
  QUrl url;

  {
    QMutexLocker l(&mutex_);
    pipeline = std::move(pipeline_);
    success = success_;
    parser = parser_;
    is_podcast = is_podcast_;
    buffer = buffer_;
    url = url_;
  }

  // Clear the bus sync handler and stop GStreamer outside mutex_ to prevent
  // lock-order deadlocks with callbacks holding GStreamer stream locks.
  if (pipeline) {
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline.get()));
    if (bus) {
      gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
      gst_object_unref(bus);
    }
    gst_element_set_state(pipeline.get(), GST_STATE_NULL);
  }
  timeout_timer_->stop();

  if (success && parser) {
    qLog(Debug) << "Parsing" << url << "with" << parser->name();

    // Parse the playlist
    QBuffer buf(&buffer);
    buf.open(QIODevice::ReadOnly);
    songs_ = parser->Load(&buf);
  } else if (success && is_podcast) {
    qLog(Debug) << "Parsing" << url << "as a podcast";

    QBuffer buf(&buffer);
    buf.open(QIODevice::ReadOnly);
    QVariant result = podcast_parser_->Load(&buf, url);

    if (result.isNull()) {
      qLog(Warning) << "Failed to parse podcast";
    } else {
      // On the podcast service's thread: the network remote loads songs away
      // from the main thread.
      PodcastService* service = InternetModel::Service<PodcastService>();
      CallOn(service, &PodcastService::SubscribeAndShow, result);
    }

  } else if (success) {
    qLog(Debug) << "Loading" << url << "as raw stream";

    // It wasn't a playlist - just put the URL in as a stream
    AddAsRawStream();
  }

  {
    QMutexLocker l(&mutex_);
    // Set before emitting so LoadRemote() won't enter loop.exec() and miss it.
    remote_finished_ = true;
  }
  emit LoadRemoteFinished();
}

SongLoader::Result SongLoader::LoadRemote() {
  qLog(Debug) << "Loading remote file" << url_;

  // It's not a local file so we have to fetch it to see what it is.  We use
  // gstreamer to do this since it handles funky URLs for us (http://, ssh://,
  // etc) and also has typefinder plugins.
  // First we wait for typefinder to tell us what it is.  If it's not text/plain
  // or text/uri-list assume it's a song and return success.
  // Otherwise wait to get 512 bytes of data and do magic on it - if the magic
  // fails then we don't know what it is so return failure.
  // If the magic succeeds then we know for sure it's a playlist - so read the
  // rest of the file, parse the playlist and return success.

  // gstreamer is initialised on a thread of its own as Clementine starts, and
  // a URL can arrive before that's finished: from the command line, or from a
  // remote. gst_is_initialized() is already true while the plugin registry is
  // still being filled in, so some elements below would be missing and the
  // load would never finish.
  if (player_) {
    if (GstEngine* engine = player_->gst_engine()) {
      engine->EnsureInitialised();
    }
  }

  // Create the pipeline - it gets unreffed if it goes out of scope
  std::shared_ptr<GstElement> pipeline(gst_pipeline_new(nullptr),
                                       std::bind(&gst_object_unref, _1));

  // Create the source element automatically based on the URL
  GstElement* source = gst_element_make_from_uri(
      GST_URI_SRC, Utilities::GetUriForGstreamer(url_).constData(), nullptr,
      nullptr);
  if (!source) {
    qLog(Warning) << "Couldn't create gstreamer source element for"
                  << url_.toString();
    return Error;
  }

  // Create the other elements and link them up
  GstElement* typefind = gst_element_factory_make("typefind", nullptr);
  GstElement* fakesink = gst_element_factory_make("fakesink", nullptr);
  if (!typefind || !fakesink) {
    qLog(Warning) << "Couldn't create gstreamer elements to load"
                  << url_.toString();
    for (GstElement* element : {source, typefind, fakesink}) {
      if (element) gst_object_unref(gst_object_ref_sink(element));
    }
    return Error;
  }

  gst_bin_add_many(GST_BIN(pipeline.get()), source, typefind, fakesink,
                   nullptr);
  gst_element_link_many(source, typefind, fakesink, nullptr);

  // Connect callbacks
  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline.get()));
  CHECKED_GCONNECT(typefind, "have-type", &TypeFound, this);
  // Handle messages only in the sync handler (dropping them there) instead of
  // a GLib bus watch, which could outlive teardown and double-handle errors.
  gst_bus_set_sync_handler(bus, BusCallbackSync, this, nullptr);
  gst_object_unref(bus);

  // Add a probe to the sink so we can capture the data if it's a playlist
  GstPad* pad = gst_element_get_static_pad(fakesink, "sink");
  gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, &DataReady, this, nullptr);
  gst_object_unref(pad);

  QEventLoop loop;
  loop.connect(this, SIGNAL(LoadRemoteFinished()), SLOT(quit()));

  {
    QMutexLocker l(&mutex_);
    state_ = WaitingForType;
    success_ = false;
    parser_ = nullptr;
    is_podcast_ = false;
    mime_type_.clear();
    buffer_.clear();
    remote_finished_ = false;
    pipeline_ = pipeline;
  }

  // This runs on a worker thread, and a timer can only be started from the
  // thread it belongs to. StopTypefind() stops it there too, and is queued
  // after this.
  PostTo(timeout_timer_, qOverload<int>(&QTimer::start), timeout_);

  // Publish pipeline_ under mutex_ before PLAYING so IsPipelinePlaying() and
  // streaming callbacks immediately see it, and fail fast if PLAYING fails.
  if (gst_element_set_state(pipeline.get(), GST_STATE_PLAYING) ==
      GST_STATE_CHANGE_FAILURE) {
    StopTypefindAsync(false);
  }

  // Wait until loading is finished
  {
    QMutexLocker l(&mutex_);
    // Check remote_finished_ before exec() in case StopTypefind() already ran.
    if (!remote_finished_) {
      l.unlock();
      loop.exec();
    }
  }

  QMutexLocker l(&mutex_);
  return success_ ? Success : Error;
}

void SongLoader::TypeFound(GstElement*, uint, GstCaps* caps, void* self) {
  SongLoader* instance = static_cast<SongLoader*>(self);

  bool should_stop = false;
  {
    QMutexLocker l(&instance->mutex_);
    if (instance->state_ != WaitingForType) return;

    // Check the mimetype
    instance->mime_type_ =
        gst_structure_get_name(gst_caps_get_structure(caps, 0));
    qLog(Debug) << "Mime type is" << instance->mime_type_;
    if (instance->mime_type_ == "text/plain" ||
        instance->mime_type_ == "text/uri-list" ||
        instance->podcast_parser_->supported_mime_types().contains(
            instance->mime_type_)) {
      // Yeah it might be a playlist, let's get some data and have a better look
      instance->state_ = WaitingForMagic;
      return;
    }

    // Nope, not a playlist - we're done
    should_stop = instance->StopTypefindAsyncLocked(true);
  }
  if (should_stop) PostTo(instance, &SongLoader::StopTypefind);
}

GstPadProbeReturn SongLoader::DataReady(GstPad*, GstPadProbeInfo* info,
                                        gpointer self) {
  SongLoader* instance = reinterpret_cast<SongLoader*>(self);

  GstBuffer* buffer = gst_pad_probe_info_get_buffer(info);
  if (!buffer) return GST_PAD_PROBE_OK;

  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
    return GST_PAD_PROBE_OK;
  }

  bool check_magic = false;
  bool hit_buffer_limit = false;
  bool should_stop = false;
  State current_state = Finished;
  int current_size = 0;

  {
    QMutexLocker l(&instance->mutex_);
    if (instance->state_ == Finished) {
      // Unlock before gst_buffer_unmap to keep lock nesting minimal.
      l.unlock();
      gst_buffer_unmap(buffer, &map);
      return GST_PAD_PROBE_OK;
    }

    // Cap buffered bytes so non-audio streams can't grow buffer_ unboundedly.
    const int remaining = kMaxRemotePlaylistSize - instance->buffer_.size();
    if (remaining > 0) {
      const int to_copy = qMin(static_cast<int>(map.size), remaining);
      instance->buffer_.append(reinterpret_cast<const char*>(map.data),
                               to_copy);
      qLog(Debug) << "Received total" << instance->buffer_.size() << "bytes";
    }

    current_state = instance->state_;
    current_size = instance->buffer_.size();
    if (current_size >= kMaxRemotePlaylistSize) {
      hit_buffer_limit = true;
      should_stop = instance->StopTypefindAsyncLocked(false);
    } else if (current_state == WaitingForMagic &&
               current_size >= PlaylistParser::kMagicSize) {
      check_magic = true;
    }
  }
  gst_buffer_unmap(buffer, &map);

  if (hit_buffer_limit) {
    qLog(Warning) << "Remote buffer limit exceeded for"
                  << instance->url_.toString();
    if (should_stop) PostTo(instance, &SongLoader::StopTypefind);
    return GST_PAD_PROBE_OK;
  }

  // Query pipeline state and inspect magic outside mutex_ to avoid lock-order
  // inversion with GStreamer locks.
  if (check_magic ||
      (current_state == WaitingForMagic && !instance->IsPipelinePlaying())) {
    // Got enough that we can test the magic
    instance->MagicReady();
  }

  return GST_PAD_PROBE_OK;
}

GstBusSyncReply SongLoader::BusCallbackSync(GstBus*, GstMessage* msg,
                                            gpointer self) {
  SongLoader* instance = reinterpret_cast<SongLoader*>(self);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      instance->EndOfStreamReached();
      break;

    case GST_MESSAGE_ERROR:
      instance->ErrorMessageReceived(msg);
      break;

    default:
      break;
  }
  // Unref and drop every message here because we have no GLib bus watch;
  // GST_BUS_PASS would queue messages onto the bus indefinitely.
  gst_message_unref(msg);
  return GST_BUS_DROP;
}

void SongLoader::ErrorMessageReceived(GstMessage* msg) {
  GError* error = nullptr;
  gchar* debugs = nullptr;

  gst_message_parse_error(msg, &error, &debugs);
  QString message_str;
  if (error) {
    qLog(Error) << QString::fromLocal8Bit(error->message);
    message_str = QString::fromLocal8Bit(error->message);
    g_error_free(error);
  }
  if (debugs) {
    qLog(Error) << QString::fromLocal8Bit(debugs);
    // GStreamer allocates debug/error strings via GLib; free with g_free().
    g_free(debugs);
  }

  bool should_stop = false;
  {
    QMutexLocker l(&mutex_);
    if (state_ == Finished) return;

    if (state_ == WaitingForType) {
      gchar* type_not_found = gst_error_get_message(
          GST_STREAM_ERROR, GST_STREAM_ERROR_TYPE_NOT_FOUND);
      const bool is_type_not_found =
          type_not_found && message_str == type_not_found;
      g_free(type_not_found);
      if (is_type_not_found) {
        // Don't give up - assume it's a playlist and see if one of our parsers
        // can read it.
        state_ = WaitingForMagic;
        return;
      }
    }

    should_stop = StopTypefindAsyncLocked(false);
  }
  if (should_stop) PostTo(this, &SongLoader::StopTypefind);
}

void SongLoader::EndOfStreamReached() {
  bool run_magic = false;
  bool should_stop = false;

  {
    QMutexLocker l(&mutex_);
    qLog(Debug) << Q_FUNC_INFO << state_;
    switch (state_) {
      case Finished:
        break;

      case WaitingForMagic:
        run_magic = true;
        break;

      case WaitingForData:
        // It's a playlist and we've got all the data - finish and parse it
        should_stop = StopTypefindAsyncLocked(true);
        break;

      case WaitingForType:
        should_stop = StopTypefindAsyncLocked(false);
        break;
    }
  }

  if (run_magic) {
    bool still_waiting = false;
    {
      QMutexLocker l(&mutex_);
      if (state_ == Finished) return;
      should_stop = MagicReadyLocked(&still_waiting);
      // EOS already arrived, so if magic recognized a playlist (WaitingForData)
      // parse whatever bytes we buffered instead of waiting for more data.
      if (!should_stop && state_ == WaitingForData) {
        should_stop = StopTypefindAsyncLocked(true);
      }
    }
  }

  if (should_stop) PostTo(this, &SongLoader::StopTypefind);
}

// Runs with mutex_ held; returns true if loading finished and the caller must
// post StopTypefind() once mutex_ is released.
bool SongLoader::MagicReadyLocked(bool* still_waiting_for_data) {
  *still_waiting_for_data = false;
  qLog(Debug) << Q_FUNC_INFO;
  parser_ = playlist_parser_->ParserForMagic(buffer_, mime_type_);
  is_podcast_ = false;

  if (!parser_) {
    // Maybe it's a podcast?
    if (podcast_parser_->TryMagic(buffer_)) {
      is_podcast_ = true;
      qLog(Debug) << "Looks like a podcast";
    } else {
      qLog(Warning) << url_.toString()
                    << "is text, but not a recognised playlist";
      // It doesn't look like a playlist, so just finish
      return StopTypefindAsyncLocked(false);
    }
  }

  // We'll get more data and parse the whole thing in EndOfStreamReached

  if (!is_podcast_) {
    qLog(Debug) << "Magic says" << parser_->name();
    if (parser_->name() == "ASX/INI" && url_.scheme() == "http") {
      // This is actually a weird MS-WMSP stream. Changing the protocol to MMS
      // from
      // HTTP makes it playable.
      parser_ = nullptr;
      url_.setScheme("mms");
      return StopTypefindAsyncLocked(true);
    }
  }

  state_ = WaitingForData;
  *still_waiting_for_data = true;
  return false;
}

void SongLoader::MagicReady() {
  bool still_waiting = false;
  bool should_stop = false;
  {
    QMutexLocker l(&mutex_);
    if (state_ == Finished) return;
    should_stop = MagicReadyLocked(&still_waiting);
  }
  if (should_stop) {
    PostTo(this, &SongLoader::StopTypefind);
    return;
  }

  // Query pipeline state outside mutex_ to avoid lock inversion with GStreamer;
  // if the stream already stopped, parse the buffered data immediately.
  if (still_waiting && !IsPipelinePlaying()) {
    EndOfStreamReached();
  }
}

bool SongLoader::IsPipelinePlaying() {
  std::shared_ptr<GstElement> pipeline;
  {
    QMutexLocker l(&mutex_);
    pipeline = pipeline_;
  }
  if (!pipeline) return false;

  GstState state = GST_STATE_NULL;
  GstState pending_state = GST_STATE_NULL;
  GstStateChangeReturn ret =
      gst_element_get_state(pipeline.get(), &state, &pending_state, GST_SECOND);

  if (ret == GST_STATE_CHANGE_ASYNC && pending_state == GST_STATE_PLAYING) {
    // We're still on the way to playing
    return true;
  }
  return state == GST_STATE_PLAYING;
}

bool SongLoader::StopTypefindAsyncLocked(bool success) {
  // Ensure StopTypefind() is posted at most once across concurrent callbacks.
  if (state_ == Finished) return false;
  state_ = Finished;
  success_ = success;
  return true;
}

void SongLoader::StopTypefindAsync(bool success) {
  bool should_stop = false;
  {
    QMutexLocker l(&mutex_);
    should_stop = StopTypefindAsyncLocked(success);
  }
  if (should_stop) PostTo(this, &SongLoader::StopTypefind);
}

bool SongLoader::LoadRemotePlaylist(const QUrl& url) {
  // This function makes a remote request for the given URL and, if its MIME
  // type corresponds to a known playlist type, it loads it, and returns true.
  // If the URL does not point to a playlist file we could handle,
  // it returns false.

  NetworkAccessManager manager;
  QNetworkRequest req(url);

  // Getting headers:
  QNetworkReply* const headers_reply = manager.head(req);
  // Delete replies on return since QNetworkAccessManager retains ownership.
  auto cleanup_headers =
      qScopeGuard([headers_reply] { headers_reply->deleteLater(); });
  WaitForSignal(headers_reply, SIGNAL(finished()));

  if (headers_reply->error() != QNetworkReply::NoError) {
    qLog(Error) << url.toString() << headers_reply->errorString();
    return false;
  }

  // Now we check if there is a parser that can handle that MIME type.
  QString mime_type =
      headers_reply->header(QNetworkRequest::ContentTypeHeader).toString();

  ParserBase* const parser = playlist_parser_->ParserForMimeType(mime_type);
  if (parser == nullptr) {
    qLog(Debug) << url.toString() << "seems to not be a playlist";
    return false;
  }

  // We know it is a playlist!
  // Getting its contents:
  QNetworkReply* const data_reply = manager.get(req);
  auto cleanup_data = qScopeGuard([data_reply] { data_reply->deleteLater(); });
  WaitForSignal(data_reply, SIGNAL(finished()));

  if (data_reply->error() != QNetworkReply::NoError) {
    qLog(Error) << url.toString() << data_reply->errorString();
    return false;
  }

  qLog(Debug) << "Loading" << url.toString() << "with MIME" << mime_type;

  songs_ = parser->Load(data_reply);
  return true;
}
