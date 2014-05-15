#include "streamserver.h"

#include <QByteArray>
#include <QSettings>
#include <QTcpSocket>
#include <QTcpServer>

#include "core/closure.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/urlhandler.h"
#include "core/signalchecker.h"
#include "engines/gstengine.h"

#include <gst/gst.h>

const char* StreamServer::kSettingsGroup = "Streaming";
const quint16 StreamServer::kDefaultServerPort = 8080;

StreamServer::StreamServer(Player* player, QObject* parent)
    : QObject(parent),
      player_(player),
      server_(new QTcpServer(this)) {
  //GstEngine::InitialiseGstreamer();
  gst_init(nullptr, nullptr);
}

void StreamServer::Listen() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  // Streaming activated?
  if (!s.value("use_streaming", false).toBool()) {
    return;
  }

  server_->listen(QHostAddress::LocalHost, s.value("port", kDefaultServerPort).toInt());
  connect(server_, SIGNAL(newConnection()), SLOT(AcceptConnection()));
}

void StreamServer::StopListening() {
  server_->close();
}

void StreamServer::AcceptConnection() {
  QTcpSocket* socket = server_->nextPendingConnection();
  QByteArray buffer;
  NewClosure(socket, SIGNAL(readyRead()),
             this, SLOT(ReadyRead(QTcpSocket*, QByteArray)), socket, buffer);
}

namespace {

static void NewPadCallback(GstElement*, GstPad* pad, gpointer data) {
  qLog(Debug) << Q_FUNC_INFO;
  GstElement* converter = reinterpret_cast<GstElement*>(data);
  GstPad* const audiopad = gst_element_get_static_pad(converter, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << "Pad already linked";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);
}

static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage* msg, gpointer data) {
  QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(data);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
      GError* err = NULL;
      gchar* dbg_info = NULL;
      gst_message_parse_error(msg, &err, &dbg_info);
      g_printerr("Error from element %s: %s\n",
                 GST_OBJECT_NAME(msg->src), err->message);
      g_error_free(err);
      g_free(dbg_info);
      socket->close();
      break;
    }
    default:
      break;
  }

  return GST_BUS_PASS;
}

}

void StreamServer::ReadyRead(QTcpSocket* socket, QByteArray buffer) {
  buffer.append(socket->readAll());
  if (socket->atEnd() || buffer.endsWith("\r\n\r\n")) {
    QByteArray response = ParseRequest(buffer);
    qLog(Debug) << response;
    socket->write("HTTP/1.0 200 OK\r\n");
    socket->write("Content-type: application/ogg\r\n");
    socket->write("Connection: close\r\n");
    socket->write("\r\n");
    socket->flush();

    QUrl url(QString::fromUtf8(response), QUrl::StrictMode);
    UrlHandler* handler = player_->HandlerForUrl(url);
    if (handler) {
      connect(handler, SIGNAL(AsyncLoadComplete(const UrlHandler::LoadResult&)),
              SLOT(AsyncLoadComplete(const UrlHandler::LoadResult&)), Qt::UniqueConnection);
      UrlHandler::LoadResult result = handler->StartLoading(url);
      if (result.type_ == UrlHandler::LoadResult::TrackAvailable) {
        SendStream(result.media_url_, socket);
      } else if (result.type_ == UrlHandler::LoadResult::WillLoadAsynchronously) {
        sockets_[url] = socket;
      }
    } else {
      SendStream(url, socket);
    }

  } else {
    NewClosure(socket, SIGNAL(readyRead()),
               this, SLOT(ReadyRead(QTcpSocket*, QByteArray)), socket, buffer);
  }
}

void StreamServer::SendStream(const QUrl& url, QTcpSocket* socket) {
  GstElement* pipeline = gst_pipeline_new("stream_pipeline");

  GstElement* decodebin = gst_element_factory_make("uridecodebin", NULL);
  GstElement* audioconvert = gst_element_factory_make("audioconvert", NULL);
  GstElement* audioresample = gst_element_factory_make("audioresample", NULL);
  GstElement* vorbisenc = gst_element_factory_make("vorbisenc", NULL);
  GstElement* oggmux = gst_element_factory_make("oggmux", NULL);
  GstElement* fdsink = gst_element_factory_make("fdsink", NULL);
  gst_bin_add_many(GST_BIN(pipeline),
      decodebin, audioconvert, audioresample, vorbisenc, oggmux, fdsink,
      NULL);

  g_object_set(vorbisenc, "quality", getQuality(), NULL);
  g_object_set(decodebin, "uri", url.toString().toUtf8().constData(), NULL);
  g_object_set(fdsink, "fd", socket->socketDescriptor(), NULL);

  gst_element_link_many(
      audioconvert, audioresample, vorbisenc, oggmux, fdsink, NULL);

  CHECKED_GCONNECT(decodebin, "pad-added", &NewPadCallback, audioconvert);

  gst_bus_set_sync_handler(
      gst_pipeline_get_bus(GST_PIPELINE(pipeline)), &BusCallbackSync, socket);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

QByteArray StreamServer::ParseRequest(const QByteArray& data) {
  QList<QByteArray> lines = data.split('\r');
  QByteArray path = lines[0].split(' ')[1];
  QByteArray id = path.mid(1);
  return id;
}

void StreamServer::AsyncLoadComplete(const UrlHandler::LoadResult& result) {
  QTcpSocket* socket = sockets_.take(result.original_url_);
  SendStream(result.media_url_, socket);
}

double StreamServer::getQuality() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  return s.value("quality", 0.3).toDouble();
}
