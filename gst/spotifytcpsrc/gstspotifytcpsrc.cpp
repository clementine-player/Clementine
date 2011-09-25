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

#include "gstspotifytcpsrc.h"
#include "core/logging.h"
#include "internet/internetmodel.h"
#include "internet/spotifyserver.h"
#include "internet/spotifyservice.h"

#include <QTcpServer>
#include <QTcpSocket>

#include <stdlib.h>
#include <string.h>
#include <gst/gst.h>

#ifdef Q_OS_UNIX
# include <sys/types.h>
# include <sys/socket.h>
#endif


static const int kPollTimeoutMsec = 100;

// This is about one second of audio at spotify's bitrate.
static const int kSocketBufferSize = 176400;

// Signals
enum {
  LAST_SIGNAL
};

// Properties
enum {
  PROP_0,
  PROP_LOCATION,
};

GST_DEBUG_CATEGORY_STATIC(gst_spotifytcp_src_debug);
#define GST_CAT_DEFAULT gst_spotifytcp_src_debug

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS_ANY
);

static void gst_spotifytcp_src_interface_init(GType type);
static void gst_spotifytcp_src_uri_handler_init(gpointer iface, gpointer data);
static void gst_spotifytcp_src_set_property(GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_spotifytcp_src_get_property(GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static void gst_spotifytcp_src_finalize(GObject* object);
static gboolean gst_spotifytcp_src_start(GstBaseSrc* src);
static gboolean gst_spotifytcp_src_stop(GstBaseSrc* src);
static GstFlowReturn gst_spotifytcp_src_create(GstPushSrc* src, GstBuffer** buffer);
static gboolean gst_spotifytcp_src_is_seekable(GstBaseSrc* src);
static gboolean gst_spotifytcp_src_do_seek(GstBaseSrc* src, GstSegment* segment);
static gboolean gst_spotifytcp_src_unlock(GstBaseSrc* src);
static gboolean gst_spotifytcp_src_unlock_stop(GstBaseSrc* src);

GST_BOILERPLATE_FULL(GstSpotifyTcpSrc, gst_spotifytcp_src, GstPushSrc,
                     GST_TYPE_PUSH_SRC, gst_spotifytcp_src_interface_init);


static void gst_spotifytcp_src_interface_init(GType type) {
  static const GInterfaceInfo urihandler_info = {
    gst_spotifytcp_src_uri_handler_init,
    NULL,
    NULL
  };

  GST_DEBUG_CATEGORY_INIT (gst_spotifytcp_src_debug, "spotifytcpsrc", 0,
                           "libspotify source");

  g_type_add_interface_static(type, GST_TYPE_URI_HANDLER, &urihandler_info);
}

static void gst_spotifytcp_src_base_init(gpointer gclass) {
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "libspotify source",
    "Source/SpotifyTcp",
    "Receive audio data from an external libspotify process",
    "David Sansome <me@davidsansome.com>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
}

static void gst_spotifytcp_src_class_init (GstSpotifyTcpSrcClass* klass) {
  GObjectClass* gobject_class = (GObjectClass*) klass;
  GstBaseSrcClass* gstbasesrc_class = (GstBaseSrcClass*) klass;
  GstPushSrcClass* gstpushsrc_class = (GstPushSrcClass*) klass;

  gobject_class->set_property = gst_spotifytcp_src_set_property;
  gobject_class->get_property = gst_spotifytcp_src_get_property;
  gobject_class->finalize = gst_spotifytcp_src_finalize;

  g_object_class_install_property(gobject_class, PROP_LOCATION,
    g_param_spec_string(
        "location", "URI",
        "The URI of the file to read, must be of the form afc://uuid/filename", NULL,
        static_cast<GParamFlags>(
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | GST_PARAM_MUTABLE_READY)));

  gstpushsrc_class->create = gst_spotifytcp_src_create;
  gstbasesrc_class->start = gst_spotifytcp_src_start;
  gstbasesrc_class->stop = gst_spotifytcp_src_stop;
  gstbasesrc_class->is_seekable = gst_spotifytcp_src_is_seekable;
  gstbasesrc_class->do_seek = gst_spotifytcp_src_do_seek;
  gstbasesrc_class->unlock = gst_spotifytcp_src_unlock;
  gstbasesrc_class->unlock_stop = gst_spotifytcp_src_unlock_stop;
}

static GstURIType gst_spotifytcp_src_uri_get_type() {
  return GST_URI_SRC;
}

static gchar** gst_spotifytcp_src_uri_get_protocols() {
  static const gchar* protocols[] = { "spotify", NULL };
  return (char**) protocols;
}

static const gchar* gst_spotifytcp_src_uri_get_uri(GstURIHandler* handler) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(handler);
  return self->uri_->constData();
}

static gboolean gst_spotifytcp_src_uri_set_uri(GstURIHandler* handler, const gchar* uri) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(handler);
  *(self->uri_) = uri;
  return TRUE;
}

static void gst_spotifytcp_src_uri_handler_init(gpointer g_iface, gpointer data) {
  GstURIHandlerInterface* iface = (GstURIHandlerInterface*) g_iface;

  iface->get_type = gst_spotifytcp_src_uri_get_type;
  iface->get_protocols = gst_spotifytcp_src_uri_get_protocols;
  iface->set_uri = gst_spotifytcp_src_uri_set_uri;
  iface->get_uri = gst_spotifytcp_src_uri_get_uri;
}


static void gst_spotifytcp_src_init(GstSpotifyTcpSrc* element, GstSpotifyTcpSrcClass* gclass) {
  element->service_ = NULL;
  element->uri_ = new QByteArray;
  element->server_ = NULL;
  element->socket_ = NULL;
  element->unlock_ = false;
}

static void gst_spotifytcp_src_finalize(GObject* object) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(object);
  delete self->uri_;
  delete self->socket_;
  delete self->server_;

  G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void gst_spotifytcp_src_set_property(
    GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(object);

  switch (prop_id) {
    case PROP_LOCATION:
      gst_spotifytcp_src_uri_set_uri(reinterpret_cast<GstURIHandler*>(self),
                                     g_value_get_string(value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void gst_spotifytcp_src_get_property(
    GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(object);

  switch (prop_id) {
    case PROP_LOCATION:
      g_value_set_string(value, gst_spotifytcp_src_uri_get_uri(
                           reinterpret_cast<GstURIHandler*>(self)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean gst_spotifytcp_src_start(GstBaseSrc* src) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(src);

  if (self->server_)
    return TRUE;

  self->service_ = InternetModel::Service<SpotifyService>();
  if (!self->service_)
    return FALSE;

  SpotifyServer* server = self->service_->server();
  if (!server)
    return FALSE;

  self->server_ = new QTcpServer;
  if (!self->server_->listen(QHostAddress::LocalHost, 0)) {
    delete self->server_;
    self->server_ = NULL;
    return FALSE;
  }

  qLog(Debug) << "Listening for media on port" << self->server_->serverPort();
  server->StartPlayback(QString::fromAscii(*self->uri_), self->server_->serverPort());

  // Wait for a client to connect
  while (!self->server_->waitForNewConnection(kPollTimeoutMsec)) {
    if (self->unlock_) {
      qLog(Warning) << "Unlock while waiting for connection";
      return FALSE;
    }
  }

  // Take the socket
  self->socket_ = self->server_->nextPendingConnection();
  if (!self->socket_) {
    qLog(Warning) << "Failed to get pending connection";
    return FALSE;
  }

  qLog(Info) << "Media socket connected";

  self->socket_->setReadBufferSize(kSocketBufferSize);
  self->socket_->setSocketOption(QAbstractSocket::LowDelayOption, true);

#ifdef Q_OS_UNIX
  const int size = kSocketBufferSize;
  setsockopt(self->socket_->socketDescriptor(), SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
#endif

  return TRUE;
}

static gboolean gst_spotifytcp_src_stop(GstBaseSrc* src) {
  return TRUE;
}

static GstFlowReturn gst_spotifytcp_src_create(GstPushSrc* src, GstBuffer** buffer) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(src);

  if (self->socket_->state() != QAbstractSocket::ConnectedState) {
    qLog(Info) << "Media socket disconnected";
    return GST_FLOW_UNEXPECTED;
  }

  while (!self->socket_->waitForReadyRead(kPollTimeoutMsec)) {
    if (self->unlock_) {
      qLog(Warning) << "Unlock while reading data";
      return GST_FLOW_WRONG_STATE;
    }
  }

  qint64 length = self->socket_->bytesAvailable();

  GstBuffer* buf = gst_buffer_try_new_and_alloc(length);
  if (buf == NULL) {
    qLog(Error) << "Failed to allocate buffer";
    return GST_FLOW_ERROR;
  }

  qint64 bytes_read = self->socket_->read(
        reinterpret_cast<char*>(GST_BUFFER_DATA(buf)), length);
  if (bytes_read < 0) {
    gst_buffer_unref(buf);
    return GST_FLOW_UNEXPECTED;
  }

  *buffer = buf;
  return GST_FLOW_OK;
}

static gboolean gst_spotifytcp_src_unlock(GstBaseSrc* src) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(src);
  self->unlock_ = true;
  return TRUE;
}

static gboolean gst_spotifytcp_src_unlock_stop(GstBaseSrc* src) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(src);
  self->unlock_ = false;
  return TRUE;
}

static gboolean gst_spotifytcp_src_is_seekable(GstBaseSrc* src) {
  return TRUE;
}

static gboolean gst_spotifytcp_src_do_seek(GstBaseSrc* src, GstSegment* segment) {
  GstSpotifyTcpSrc* self = GST_SPOTIFYTCPSRC(src);

  SpotifyServer* server = self->service_->server();
  if (!server)
    return FALSE;

  // Tell the spotify client to seek.
  server->metaObject()->invokeMethod(server, "Seek", Q_ARG(qint64, segment->start));

  // Throw away any old data that's sitting in the socket's read buffer.
  self->socket_->waitForReadyRead(kPollTimeoutMsec);
  self->socket_->read(self->socket_->bytesAvailable());

  return TRUE;
}

#define PACKAGE "Clementine"

static gboolean spotifytcpsrc_init(GstPlugin* spotifytcpsrc) {
  return gst_element_register(spotifytcpsrc, "spotifytcpsrc", GST_RANK_PRIMARY, GST_TYPE_SPOTIFYTCPSRC);
}

void spotifytcpsrc_register_static() {
  gst_plugin_register_static(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "spotifytcpsrc",
    const_cast<gchar*>("libspotify source"),
    spotifytcpsrc_init,
    "0.1",
    "GPL",
    "Clementine",
    "Clementine",
    "http://www.clementine-player.org/");
}

