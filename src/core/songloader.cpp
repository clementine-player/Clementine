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
#include "core/song.h"
#include "library/librarybackend.h"
#include "library/sqlrow.h"
#include "playlistparsers/parserbase.h"
#include "playlistparsers/cueparser.h"
#include "playlistparsers/playlistparser.h"
#include "radio/fixlastfm.h"

#include <QBuffer>
#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>
#include <QtConcurrentRun>
#include <QtDebug>

#include <boost/bind.hpp>

QSet<QString> SongLoader::sRawUriSchemes;
const int SongLoader::kDefaultTimeout = 5000;

SongLoader::SongLoader(LibraryBackendInterface* library, QObject *parent)
  : QObject(parent),
    timeout_timer_(new QTimer(this)),
    playlist_parser_(new PlaylistParser(library, this)),
    cue_parser_(new CueParser(library, this)),
    timeout_(kDefaultTimeout),
    state_(WaitingForType),
    success_(false),
    parser_(NULL),
    library_(library)
{
  if (sRawUriSchemes.isEmpty()) {
    sRawUriSchemes << "udp" << "mms" << "mmsh" << "mmst" << "mmsu" << "rtsp"
                   << "rtspu" << "rtspt" << "rtsph";
  }

  timeout_timer_->setSingleShot(true);

  connect(timeout_timer_, SIGNAL(timeout()), SLOT(Timeout()));
}

SongLoader::~SongLoader() {
  if (pipeline_) {
    state_ = Finished;
    gst_element_set_state(pipeline_.get(), GST_STATE_NULL);
  }
}

SongLoader::Result SongLoader::Load(const QUrl& url) {
  url_ = url;

  if (url_.scheme() == "file") {
    return LoadLocal(url_.toLocalFile());
  }

  if (sRawUriSchemes.contains(url_.scheme())) {
    // The URI scheme indicates that it can't possibly be a playlist, so add
    // it as a raw stream.
    AddAsRawStream();
    return Success;
  }

  timeout_timer_->start(timeout_);
  return LoadRemote();
}

SongLoader::Result SongLoader::LoadLocal(const QString& filename, bool block,
                                         bool ignore_playlists) {
  qDebug() << "Loading local file" << filename;

  // First check to see if it's a directory - if so we can load all the songs
  // inside right away.
  if (QFileInfo(filename).isDir()) {
    if (!block) {
      QtConcurrent::run(this, &SongLoader::LoadLocalDirectoryAndEmit, filename);
      return WillLoadAsync;
    } else {
      LoadLocalDirectory(filename);
      return Success;
    }
  }

  // It's a local file, so check if it looks like a playlist.
  // Read the first few bytes.
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    return Error;
  QByteArray data(file.read(PlaylistParser::kMagicSize));

  ParserBase* parser = playlist_parser_->MaybeGetParserForMagic(data);
  if (!parser) {
    // Check the file extension as well, maybe the magic failed, or it was a
    // basic M3U file which is just a plain list of filenames.
    parser = playlist_parser_->ParserForExtension(QFileInfo(filename).suffix());
  }

  if (parser) {
    if (ignore_playlists) {
      qDebug() << "Skipping" << parser->name() << "playlist while loading directory";
      return Success;
    }

    qDebug() << "Parsing using" << parser->name();

    // It's a playlist!
    if (!block) {
      QtConcurrent::run(this, &SongLoader::LoadPlaylistAndEmit, parser, filename);
      return WillLoadAsync;
    } else {
      LoadPlaylist(parser, filename);
      return Success;
    }
  }

  // Not a playlist, so just assume it's a song
  QFileInfo info(filename);

  LibraryQuery query;
  query.SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);
  query.AddWhere("filename", info.canonicalFilePath());

  SongList song_list;

  if (library_->ExecQuery(&query) && query.Next()) {
    // we may have many results when the file has many sections
    do {
      Song song;
      song.InitFromQuery(query);

      song_list << song;
    } while(query.Next());
  } else {
    QString matching_cue = filename.section('.', 0, -2) + ".cue";

    // it's a cue - create virtual tracks
    if(QFile::exists(matching_cue)) {
      QFile cue(matching_cue);
      cue.open(QIODevice::ReadOnly);

      song_list = cue_parser_->Load(&cue, matching_cue, QDir(filename.section('/', 0, -2)));

    // it's a normal media file
    } else {
      Song song;
      song.InitFromFile(filename, -1);

      song_list << song;

    }
  }

  foreach(const Song& song, song_list) {
    if (song.is_valid())
      songs_ << song;
  }

  return Success;
}

void SongLoader::LoadPlaylistAndEmit(ParserBase* parser, const QString& filename) {
  LoadPlaylist(parser, filename);
  emit LoadFinished(true);
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
  return left.filename() < right.filename();
}

void SongLoader::LoadLocalDirectoryAndEmit(const QString& filename) {
  LoadLocalDirectory(filename);
  emit LoadFinished(true);
}

void SongLoader::LoadLocalDirectory(const QString& filename) {
  QDirIterator it(filename, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                  QDirIterator::Subdirectories);

  while (it.hasNext()) {
    // This is in another thread so we can do blocking calls.
    LoadLocal(it.next(), true, true);
  }

  qStableSort(songs_.begin(), songs_.end(), CompareSongs);
}

void SongLoader::AddAsRawStream() {
  Song song;
  song.set_valid(true);
  song.set_filetype(Song::Type_Stream);
  song.set_filename(url_.toString());
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
    qDebug() << "Parsing" << url_ << "with" << parser_->name();

    // Parse the playlist
    QBuffer buf(&buffer_);
    buf.open(QIODevice::ReadOnly);
    songs_ = parser_->Load(&buf);
  } else if (success_) {
    qDebug() << "Loading" << url_ << "as raw stream";

    // It wasn't a playlist - just put the URL in as a stream
    AddAsRawStream();
  }

  emit LoadFinished(success_);
}

SongLoader::Result SongLoader::LoadRemote() {
  qDebug() << "Loading remote file" << url_;

  // It's not a local file so we have to fetch it to see what it is.  We use
  // gstreamer to do this since it handles funky URLs for us (http://, ssh://,
  // etc) and also has typefinder plugins.
  // First we wait for typefinder to tell us what it is.  If it's not text/plain
  // or text/uri-list assume it's a song and return success.
  // Otherwise wait to get 512 bytes of data and do magic on it - if the magic
  // fails then we don't know what it is so return failure.
  // If the magic succeeds then we know for sure it's a playlist - so read the
  // rest of the file, parse the playlist and return success.

  // Create the pipeline - it gets unreffed if it goes out of scope
  boost::shared_ptr<GstElement> pipeline(
      gst_pipeline_new(NULL), boost::bind(&gst_object_unref, _1));

  // Create the source element automatically based on the URL
  GstElement* source = gst_element_make_from_uri(
      GST_URI_SRC, url_.toEncoded().constData(), NULL);
  if (!source) {
    qWarning() << "Couldn't create gstreamer source element for" << url_.toString();
    return Error;
  }

  // Create the other elements and link them up
  GstElement* typefind = gst_element_factory_make("typefind", NULL);
  GstElement* fakesink = gst_element_factory_make("fakesink", NULL);

  gst_bin_add_many(GST_BIN(pipeline.get()), source, typefind, fakesink, NULL);
  gst_element_link_many(source, typefind, fakesink, NULL);

  // Connect callbacks
  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline.get()));
  g_signal_connect(typefind, "have-type", G_CALLBACK(TypeFound), this);
  gst_bus_set_sync_handler(bus, BusCallbackSync, this);
  gst_bus_add_watch(bus, BusCallback, this);

  // Add a probe to the sink so we can capture the data if it's a playlist
  GstPad* pad = gst_element_get_pad(fakesink, "sink");
  gst_pad_add_buffer_probe(pad, G_CALLBACK(DataReady), this);
  gst_object_unref(pad);

  // Start "playing"
  gst_element_set_state(pipeline.get(), GST_STATE_PLAYING);
  pipeline_ = pipeline;
  return WillLoadAsync;
}

void SongLoader::TypeFound(GstElement*, uint, GstCaps* caps, void* self) {
  SongLoader* instance = static_cast<SongLoader*>(self);

  if (instance->state_ != WaitingForType)
    return;

  // Check the mimetype
  instance->mime_type_ = gst_structure_get_name(gst_caps_get_structure(caps, 0));
  qDebug() << "Mime type is" << instance->mime_type_;
  if (instance->mime_type_ == "text/plain" ||
      instance->mime_type_ == "text/uri-list" ||
      instance->mime_type_ == "application/xml") {
    // Yeah it might be a playlist, let's get some data and have a better look
    instance->state_ = WaitingForMagic;
    return;
  }

  // Nope, not a playlist - we're done
  instance->StopTypefindAsync(true);
}

void SongLoader::DataReady(GstPad *, GstBuffer *buf, void *self) {
  SongLoader* instance = static_cast<SongLoader*>(self);

  if (instance->state_ == Finished)
    return;

  // Append the data to the buffer
  instance->buffer_.append(reinterpret_cast<const char*>(GST_BUFFER_DATA(buf)),
                           GST_BUFFER_SIZE(buf));
  qDebug() << "Received total" << instance->buffer_.size() << "bytes";

  if (instance->state_ == WaitingForMagic &&
      instance->buffer_.size() >= PlaylistParser::kMagicSize) {
    // Got enough that we can test the magic
    instance->MagicReady();
  }
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

  return FALSE;
}

GstBusSyncReply SongLoader::BusCallbackSync(GstBus*, GstMessage* msg, gpointer self) {
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
  if (state_ == Finished)
    return;

  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  qDebug() << error->message;
  qDebug() << debugs;

  QString message_str = error->message;

  g_error_free(error);
  free(debugs);

  if (state_ == WaitingForType &&
      message_str == "Could not determine type of stream.") {
    // Don't give up - assume it's a playlist and see if one of our parsers can
    // read it.
    state_ = WaitingForMagic;
    return;
  }

  StopTypefindAsync(false);
}

void SongLoader::EndOfStreamReached() {
  switch (state_) {
  case Finished:
    break;

  case WaitingForMagic:
    // Do the magic on the data we have already
    MagicReady();
    if (state_ == Finished)
      break;
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
  parser_ = playlist_parser_->MaybeGetParserForMagic(buffer_, mime_type_);

  if (!parser_) {
    qWarning() << url_.toString() << "is text, but not a recognised playlist";
    // It doesn't look like a playlist, so just finish
    StopTypefindAsync(false);
    return;
  }

  // It is a playlist - we'll get more data and parse the whole thing in
  // EndOfStreamReached
  qDebug() << "Magic says" << parser_->name();
  if (parser_->name() == "ASX/INI" && url_.scheme() == "http") {
    // This is actually a weird MS-WMSP stream. Changing the protocol to MMS from
    // HTTP makes it playable.
    parser_ = NULL;
    url_.setScheme("mms");
    StopTypefindAsync(true);
  }
  state_ = WaitingForData;
}

void SongLoader::StopTypefindAsync(bool success) {
  state_ = Finished;
  success_ = success;

  metaObject()->invokeMethod(this, "StopTypefind", Qt::QueuedConnection);
}
