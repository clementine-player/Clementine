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

#ifndef __GST_SPOTIFYTCPSRC_H__
#define __GST_SPOTIFYTCPSRC_H__

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>

class QByteArray;
class QEventLoop;
class QTcpServer;
class QTcpSocket;

class SpotifyService;


extern "C" {
  void spotifytcpsrc_register_static();
}

G_BEGIN_DECLS

#define GST_TYPE_SPOTIFYTCPSRC \
  (gst_spotifytcp_src_get_type())
#define GST_SPOTIFYTCPSRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_SPOTIFYTCPSRC,GstSpotifyTcpSrc))
#define GST_SPOTIFYTCPSRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_SPOTIFYTCPSRC,GstSpotifyTcpSrcClass))
#define GST_IS_SPOTIFYTCPSRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_SPOTIFYTCPSRC))
#define GST_IS_SPOTIFYTCPSRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_SPOTIFYTCPSRC))

typedef struct _GstSpotifyTcpSrc      GstSpotifyTcpSrc;
typedef struct _GstSpotifyTcpSrcClass GstSpotifyTcpSrcClass;

struct _GstSpotifyTcpSrc {
  GstPushSrc element;

  SpotifyService* service_;

  QByteArray* uri_;
  QTcpServer* server_;
  QTcpSocket* socket_;

  bool unlock_;
};

struct _GstSpotifyTcpSrcClass {
  GstPushSrcClass parent_class;
};

GType gst_spotifytcp_src_get_type (void);

G_END_DECLS

#endif
