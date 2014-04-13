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

#include "songloader.h"

#include <memory>

#include <QBuffer>
#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>
#include <QtDebug>

#ifdef HAVE_AUDIOCD
#include <gst/cdda/gstcddabasesrc.h>
#endif

#include "config.h"
#include "core/concurrentrun.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/signalchecker.h"
#include "core/song.h"
#include "core/tagreaderclient.h"
#include "core/timeconstants.h"
#include "internet/fixlastfm.h"
#include "internet/internetmodel.h"
#include "library/librarybackend.h"
#include "library/sqlrow.h"
#include "playlistparsers/cueparser.h"
#include "playlistparsers/parserbase.h"
#include "playlistparsers/playlistparser.h"
#include "podcasts/podcastparser.h"
#include "podcasts/podcastservice.h"
#include "podcasts/podcasturlloader.h"

using std::placeholders::_1;

QSet<QString> SongLoader::sRawUriSchemes;
const int SongLoader::kDefaultTimeout = 5000;

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
                   << "rtsph"
                   << "spotify";
  }

  timeout_timer_->setSingleShot(true);

  connect(timeout_timer_, SIGNAL(timeout()), SLOT(Timeout()));
}

SongLoader::~SongLoader() {
  if (pipeline_) {
    state_ = Finished;
    gst_element_set_state(pipeline_.get(), GST_STATE_NULL);
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

  url_ = PodcastUrlLoader::FixPodcastUrl(url_);

  preload_func_ = std::bind(&SongLoader::LoadRemote, this);
  return BlockingLoadRequired;
}

void SongLoader::LoadFilenamesBlocking() {
  if (preload_func_) {
    preload_func_();
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
  // Create gstreamer cdda element
  GstElement* cdda = gst_element_make_from_uri(GST_URI_SRC, "cdda://", nullptr);
  if (cdda == nullptr) {
    qLog(Error) << "Error while creating CDDA GstElement";
    return Error;
  }

  // Change the element's state to ready and paused, to be able to query it
  if (gst_element_set_state(cdda, GST_STATE_READY) ==
          GST_STATE_CHANGE_FAILURE ||
      gst_element_set_state(cdda, GST_STATE_PAUSED) ==
          GST_STATE_CHANGE_FAILURE) {
    qLog(Error) << "Error while changing CDDA GstElement's state";
    gst_element_set_state(cdda, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(cdda));
    return Error;
  }

  // Get number of tracks
  GstFormat fmt = gst_format_get_by_nick("track");
  GstFormat out_fmt = fmt;
  gint64 num_tracks = 0;
  if (!gst_element_query_duration(cdda, &out_fmt, &num_tracks) ||
      out_fmt != fmt) {
    qLog(Error) << "Error while querying cdda GstElement";
    gst_object_unref(GST_OBJECT(cdda));
    return Error;
  }

  for (int track_number = 1; track_number <= num_tracks; track_number++) {
    // Init song
    Song song;
    guint64 duration = 0;
    // quint64 == ulonglong and guint64 == ulong, therefore we must cast
    if (gst_tag_list_get_uint64(
            GST_CDDA_BASE_SRC(cdda)->tracks[track_number - 1].tags,
            GST_TAG_DURATION, &duration)) {
      song.set_length_nanosec((quint64)duration);
    }
    song.set_valid(true);
    song.set_filetype(Song::Type_Cdda);
    song.set_url(QUrl(QString("cdda://%1").arg(track_number)));
    song.set_title(QString("Track %1").arg(track_number));
    song.set_track(track_number);
    songs_ << song;
  }

  // Generate MusicBrainz DiscId
  gst_tag_register_musicbrainz_tags();
  GstElement* pipe = gst_pipeline_new("pipeline");
  gst_bin_add(GST_BIN(pipe), cdda);
  gst_element_set_state(pipe, GST_STATE_READY);
  gst_element_set_state(pipe, GST_STATE_PAUSED);
  GstMessage* msg = gst_bus_timed_pop_filtered(
      GST_ELEMENT_BUS(pipe), GST_CLOCK_TIME_NONE, GST_MESSAGE_TAG);
  GstTagList* tags = nullptr;
  gst_message_parse_tag(msg, &tags);
  char* string_mb = nullptr;
  if (gst_tag_list_get_string(tags, GST_TAG_CDDA_MUSICBRAINZ_DISCID,
                              &string_mb)) {
    QString musicbrainz_discid(string_mb);
    qLog(Info) << "MusicBrainz discid: " << musicbrainz_discid;

    MusicBrainzClient* musicbrainz_client = new MusicBrainzClient(this);
    connect(musicbrainz_client, SIGNAL(Finished(const QString&, const QString&,
                                                MusicBrainzClient::ResultList)),
            SLOT(AudioCDTagsLoaded(const QString&, const QString&,
                                   MusicBrainzClient::ResultList)));
    musicbrainz_client->StartDiscIdRequest(musicbrainz_discid);
    g_free(string_mb);
  }

  // Clean all the Gstreamer objects we have used: we don't need them anymore
  gst_object_unref(GST_OBJECT(cdda));
  gst_element_set_state(pipe, GST_STATE_NULL);
  gst_object_unref(GST_OBJECT(pipe));
  gst_object_unref(GST_OBJECT(msg));
  gst_object_unref(GST_OBJECT(tags));

  return Success;
#else  // HAVE_AUDIOCD
  return Error;
#endif
}

void SongLoader::AudioCDTagsLoaded(
    const QString& artist, const QString& album,
    const MusicBrainzClient::ResultList& results) {
  // Remove previously added songs metadata, because there are not needed
  // and that we are going to fill it with new (more complete) ones
  songs_.clear();
  int track_number = 1;
  for (const MusicBrainzClient::Result& ret : results) {
    Song song;
    song.set_artist(artist);
    song.set_album(album);
    song.set_title(ret.title_);
    song.set_length_nanosec(ret.duration_msec_ * kNsecPerMsec);
    song.set_track(track_number);
    song.set_year(ret.year_);
    // We need to set url: that's how playlist will find the correct item to
    // update
    song.set_url(QUrl(QString("cdda://%1").arg(track_number++)));
    songs_ << song;
  }
  emit LoadAudioCDFinished(true);
}

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
  preload_func_ =
      std::bind(&SongLoader::LoadLocalAsync, this, filename);
  return BlockingLoadRequired;
}

void SongLoader::LoadLocalAsync(const QString& filename) {

  // First check to see if it's a directory - if so we will load all the songs
  // inside right away.
  if (QFileInfo(filename).isDir()) {
    LoadLocalDirectory(filename);
    return;
  }

  // It's a local file, so check if it looks like a playlist.
  // Read the first few bytes.
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) return;
  QByteArray data(file.read(PlaylistParser::kMagicSize));

  ParserBase* parser = playlist_parser_->ParserForMagic(data);
  if (!parser) {
    // Check the file extension as well, maybe the magic failed, or it was a
    // basic M3U file which is just a plain list of filenames.
    parser = playlist_parser_->ParserForExtension(QFileInfo(filename).suffix().toLower());
  }

  if (parser) {
    qLog(Debug) << "Parsing using" << parser->name();

    // It's a playlist!
    LoadPlaylist(parser, filename);
    return;
  }

  // Check if it's a cue file
  QString matching_cue = filename.section('.', 0, -2) + ".cue";
  if (QFile::exists(matching_cue)) {
    // it's a cue - create virtual tracks
    QFile cue(matching_cue);
    cue.open(QIODevice::ReadOnly);

    SongList song_list = cue_parser_->Load(&cue, matching_cue,
                                           QDir(filename.section('/', 0, -2)));
    for (Song song: song_list){
      if (song.is_valid()) songs_ << song;
    }
    return;
  }

  // Assume it's just a normal file
  Song song;
  song.InitFromFilePartial(filename);
  if (song.is_valid()) songs_ << song;
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

void SongLoader::LoadPlaylist(ParserBase* parser, const QString& filename) {
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  songs_ = parser->Load(&file, filename, QFileInfo(filename).path());
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

  qStableSort(songs_.begin(), songs_.end(), CompareSongs);

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
  state_ = Finished;
  success_ = false;
  StopTypefind();
}

void SongLoader::StopTypefind() {
  // Destroy the pipeline
  if (pipeline_) {
    gst_element_set_state(pipeline_.get(), GST_STATE_NULL);
    pipeline_.reset();
  }
  timeout_timer_->stop();

  if (success_ && parser_) {
    qLog(Debug) << "Parsing" << url_ << "with" << parser_->name();

    // Parse the playlist
    QBuffer buf(&buffer_);
    buf.open(QIODevice::ReadOnly);
    songs_ = parser_->Load(&buf);
  } else if (success_ && is_podcast_) {
    qLog(Debug) << "Parsing" << url_ << "as a podcast";

    QBuffer buf(&buffer_);
    buf.open(QIODevice::ReadOnly);
    QVariant result = podcast_parser_->Load(&buf, url_);

    if (result.isNull()) {
      qLog(Warning) << "Failed to parse podcast";
    } else {
      InternetModel::Service<PodcastService>()->SubscribeAndShow(result);
    }

  } else if (success_) {
    qLog(Debug) << "Loading" << url_ << "as raw stream";

    // It wasn't a playlist - just put the URL in as a stream
    AddAsRawStream();
  }

  emit LoadRemoteFinished();
}

void SongLoader::LoadRemote() {
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

  timeout_timer_->start(timeout_);

  // Create the pipeline - it gets unreffed if it goes out of scope
  std::shared_ptr<GstElement> pipeline(gst_pipeline_new(nullptr),
                                       std::bind(&gst_object_unref, _1));

  // Create the source element automatically based on the URL
  GstElement* source = gst_element_make_from_uri(
      GST_URI_SRC, url_.toEncoded().constData(), nullptr);
  if (!source) {
    qLog(Warning) << "Couldn't create gstreamer source element for"
                  << url_.toString();
    return;
  }

  // Create the other elements and link them up
  GstElement* typefind = gst_element_factory_make("typefind", nullptr);
  GstElement* fakesink = gst_element_factory_make("fakesink", nullptr);

  gst_bin_add_many(GST_BIN(pipeline.get()), source, typefind, fakesink,
                   nullptr);
  gst_element_link_many(source, typefind, fakesink, nullptr);

  // Connect callbacks
  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline.get()));
  CHECKED_GCONNECT(typefind, "have-type", &TypeFound, this);
  gst_bus_set_sync_handler(bus, BusCallbackSync, this);
  gst_bus_add_watch(bus, BusCallback, this);

  // Add a probe to the sink so we can capture the data if it's a playlist
  GstPad* pad = gst_element_get_static_pad(fakesink, "sink");
  gst_pad_add_buffer_probe(pad, G_CALLBACK(DataReady), this);
  gst_object_unref(pad);

  QEventLoop loop;
  loop.connect(this, SIGNAL(LoadRemoteFinished()), SLOT(quit()));

  // Start "playing"
  gst_element_set_state(pipeline.get(), GST_STATE_PLAYING);
  pipeline_ = pipeline;

  // Wait until loading is finished
  loop.exec();
}

void SongLoader::TypeFound(GstElement*, uint, GstCaps* caps, void* self) {
  SongLoader* instance = static_cast<SongLoader*>(self);

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
  instance->StopTypefindAsync(true);
}

gboolean SongLoader::DataReady(GstPad*, GstBuffer* buf, void* self) {
  SongLoader* instance = static_cast<SongLoader*>(self);

  if (instance->state_ == Finished) return true;

  // Append the data to the buffer
  instance->buffer_.append(reinterpret_cast<const char*>(GST_BUFFER_DATA(buf)),
                           GST_BUFFER_SIZE(buf));
  qLog(Debug) << "Received total" << instance->buffer_.size() << "bytes";

  if (instance->state_ == WaitingForMagic &&
      (instance->buffer_.size() >= PlaylistParser::kMagicSize ||
       !instance->IsPipelinePlaying())) {
    // Got enough that we can test the magic
    instance->MagicReady();
  }

  return true;
}

gboolean SongLoader::BusCallback(GstBus*, GstMessage* msg, gpointer self) {
  SongLoader* instance = reinterpret_cast<SongLoader*>(self);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
      instance->ErrorMessageReceived(msg);
      break;

    default:
      break;
  }

  return TRUE;
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
  return GST_BUS_PASS;
}

void SongLoader::ErrorMessageReceived(GstMessage* msg) {
  if (state_ == Finished) return;

  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  qLog(Error) << error->message;
  qLog(Error) << debugs;

  QString message_str = error->message;

  g_error_free(error);
  free(debugs);

  if (state_ == WaitingForType &&
      message_str == gst_error_get_message(GST_STREAM_ERROR,
                                           GST_STREAM_ERROR_TYPE_NOT_FOUND)) {
    // Don't give up - assume it's a playlist and see if one of our parsers can
    // read it.
    state_ = WaitingForMagic;
    return;
  }

  StopTypefindAsync(false);
}

void SongLoader::EndOfStreamReached() {
  qLog(Debug) << Q_FUNC_INFO << state_;
  switch (state_) {
    case Finished:
      break;

    case WaitingForMagic:
      // Do the magic on the data we have already
      MagicReady();
      if (state_ == Finished) break;
    // It looks like a playlist, so parse it

    // fallthrough
    case WaitingForData:
      // It's a playlist and we've got all the data - finish and parse it
      StopTypefindAsync(true);
      break;

    case WaitingForType:
      StopTypefindAsync(false);
      break;
  }
}

void SongLoader::MagicReady() {
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
      StopTypefindAsync(false);
      return;
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
      StopTypefindAsync(true);
    }
  }

  state_ = WaitingForData;

  if (!IsPipelinePlaying()) {
    EndOfStreamReached();
  }
}

bool SongLoader::IsPipelinePlaying() {
  GstState state = GST_STATE_NULL;
  GstState pending_state = GST_STATE_NULL;
  GstStateChangeReturn ret = gst_element_get_state(pipeline_.get(), &state,
                                                   &pending_state, GST_SECOND);

  if (ret == GST_STATE_CHANGE_ASYNC && pending_state == GST_STATE_PLAYING) {
    // We're still on the way to playing
    return true;
  }
  return state == GST_STATE_PLAYING;
}

void SongLoader::StopTypefindAsync(bool success) {
  state_ = Finished;
  success_ = success;

  metaObject()->invokeMethod(this, "StopTypefind", Qt::QueuedConnection);
}
