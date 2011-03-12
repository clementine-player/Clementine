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

#ifndef FINGERPRINTER_H
#define FINGERPRINTER_H

#include <gst/gst.h>

#include <QString>

class QEventLoop;

class Fingerprinter {
  // Creates an OFA fingerprint from a song.
  // Uses GStreamer to open and decode the file, also uses gstreamer's ofa
  // element to generate the fingerprint.  The fingerprint identifies one
  // particular encoding of a song.  Pass the fingerprint to MusicDNS to
  // identify it.
  // You should create one Fingerprinter for each file you want to fingerprint.
  // This class works well with QtConcurrentMap.

public:
  Fingerprinter(const QString& filename);
  ~Fingerprinter();

  // Checks if this gstreamer installation has the required ofa plugin.
  static bool GstreamerHasOfa();

  // Creates a fingerprint from the song.  This method is blocking, so you want
  // to call it in another thread.  Returns an empty string if no fingerprint
  // could be created.
  QString CreateFingerprint();

private:
  GstElement* CreateElement(const QString& factory_name, GstElement* bin = NULL);

  void ReportError(GstMessage* message);
  void TagMessageReceived(GstMessage* message);

  static void NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data);
  static gboolean BusCallback(GstBus*, GstMessage* msg, gpointer data);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage* msg, gpointer data);

private:
  QString filename_;
  QString fingerprint_;
  QEventLoop* event_loop_;

  GstElement* convert_element_;
};

#endif // FINGERPRINTER_H
