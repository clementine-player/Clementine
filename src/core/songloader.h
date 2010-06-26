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

#ifndef SONGLOADER_H
#define SONGLOADER_H

#include <QObject>
#include <QUrl>

#include "config.h"
#include "song.h"

#include <boost/shared_ptr.hpp>

#ifdef HAVE_GSTREAMER
# include <gst/gst.h>
#endif

class ParserBase;
class PlaylistParser;

class SongLoader : public QObject {
  Q_OBJECT
public:
  SongLoader(QObject* parent = 0);
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

signals:
  void LoadFinished(bool success);

private slots:
  void Timeout();
  void StopTypefind();

private:
  enum State {
    WaitingForType,
    WaitingForMagic,
    WaitingForData,
    Finished,
  };

  Result LoadLocal();
  void LoadLocalDirectory(const QString& filename);

  void AddAsRawStream();

#ifdef HAVE_GSTREAMER
  Result LoadRemote();

  // GStreamer callbacks
  static void TypeFound(GstElement* typefind, uint probability, GstCaps* caps, void* self);
  static void DataReady(GstPad*, GstBuffer* buf, void* self);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage*, gpointer);
  static gboolean BusCallback(GstBus*, GstMessage*, gpointer);

  void StopTypefindAsync(bool success);
  void ErrorMessageReceived(GstMessage* msg);
  void EndOfStreamReached();
  void MagicReady();
#endif // HAVE_GSTREAMER

private:
  static QSet<QString> sRawUriSchemes;

  QUrl url_;
  SongList songs_;

  QTimer* timeout_timer_;
  PlaylistParser* playlist_parser_;

  // For async loads
  int timeout_;
  State state_;
  bool success_;
  ParserBase* parser_;
  QString mime_type_;
  QByteArray buffer_;

#ifdef HAVE_GSTREAMER
  boost::shared_ptr<GstElement> pipeline_;
#endif
};

#endif // SONGLOADER_H
