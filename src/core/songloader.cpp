/* This file is part of Clementine.

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
#include "playlistparsers/parserbase.h"
#include "playlistparsers/playlistparser.h"
#include "radio/fixlastfm.h"

#include <QBuffer>
#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>
#include <QtConcurrentRun>
#include <QtDebug>

#include <boost/bind.hpp>

SongLoader::SongLoader(QObject *parent)
  : QObject(parent),
    timeout_timer_(new QTimer(this)),
    playlist_parser_(new PlaylistParser(this)),
    state_(WaitingForType),
    success_(false),
    parser_(NULL)
{
  timeout_timer_->setSingleShot(true);
  connect(timeout_timer_, SIGNAL(timeout()), SLOT(Timeout()));
}

SongLoader::~SongLoader() {
  if (pipeline_) {
    state_ = Finished;
    gst_element_set_state(pipeline_.get(), GST_STATE_NULL);
  }
}

SongLoader::Result SongLoader::Load(const QUrl& url, int timeout_msec) {
  url_ = url;

  if (url_.scheme() == "file") {
    return LoadLocal();
  }

  timeout_timer_->start(timeout_msec);
  return LoadRemote();
}

SongLoader::Result SongLoader::LoadLocal() {
  qDebug() << "Loading local file" << url_;

  // First check to see if it's a directory - if so we can load all the songs
  // inside right away.
  QString filename = url_.toLocalFile();
  if (QFileInfo(filename).isDir()) {
    QtConcurrent::run(this, &SongLoader::LoadLocalDirectory, filename);
    return WillLoadAsync;
  }

  // It's a local file, so check if it looks like a playlist.
  // Read the first few bytes.
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    return Error;
  QByteArray data(file.read(PlaylistParser::kMagicSize));

  ParserBase* parser = playlist_parser_->MaybeGetParserForMagic(data);
  if (parser) {
    qDebug() << "Parsing using" << parser->name();

    // It's a playlist!
    file.reset();
    songs_ = parser->Load(&file, QFileInfo(filename).path());
  } else {
    // Not a playlist, so just assume it's a song
    Song song;
    song.InitFromFile(filename, -1);
    if (song.is_valid())
      songs_ << song;
  }

  return Success;
}

static bool CompareSongs(const Song& left, const Song& right) {
  // Order by artist, album, disc, track
  if (left.artist() < right.artist()) return true;
  if (left.artist() > right.artist()) return true;
  if (left.album() < right.album()) return true;
  if (left.album() > right.album()) return true;
  if (left.disc() < right.disc()) return true;
  if (left.disc() > right.disc()) return true;
  return left.track() < right.track();
}

void SongLoader::LoadLocalDirectory(const QString& filename) {
  QDirIterator it(filename, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                  QDirIterator::Subdirectories);

  while (it.hasNext()) {
    QString path = it.next();
    Song song;
    song.InitFromFile(path, -1);
    if (song.is_valid())
      songs_ << song;
  }

  qStableSort(songs_.begin(), songs_.end(), CompareSongs);

  emit LoadFinished(true);
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
    qWarning() << "Couldn't create gstreamer source element for" << url_;
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
      instance->mime_type_ == "text/uri-list") {
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

  g_error_free(error);
  free(debugs);

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
  state_ = WaitingForData;
}

void SongLoader::StopTypefindAsync(bool success) {
  state_ = Finished;
  success_ = success;

  metaObject()->invokeMethod(this, "StopTypefind", Qt::QueuedConnection);
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
    Song song;
    song.set_valid(true);
    song.set_filetype(Song::Type_Stream);
    song.set_title(url_.toString());
    songs_ << song;
  }

  emit LoadFinished(success_);
}

void SongLoader::Timeout() {
  state_ = Finished;
  success_ = false;
  StopTypefind();
}
