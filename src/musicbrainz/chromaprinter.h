/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef CHROMAPRINTER_H
#define CHROMAPRINTER_H

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <QBuffer>
#include <QString>

class Chromaprinter {
  // Creates a Chromaprint fingerprint from a song.
  // Uses GStreamer to open and decode the file as PCM data and passes this
  // to Chromaprint's code generator. The generated code can be used to identify
  // a song via Acoustid.
  // You should create one Chromaprinter for each file you want to fingerprint.
  // This class works well with QtConcurrentMap.

 public:
  Chromaprinter(const QString& filename);
  ~Chromaprinter();

  // Creates a fingerprint from the song.  This method is blocking, so you want
  // to call it in another thread.  Returns an empty string if no fingerprint
  // could be created.
  QString CreateFingerprint();

 private:
  GstElement* CreateElement(const QString& factory_name,
                            GstElement* bin = nullptr);

  static void NewPadCallback(GstElement*, GstPad* pad, gpointer data);
  static GstFlowReturn NewBufferCallback(GstAppSink* app_sink, gpointer self);

 private:
  QString filename_;

  GstElement* convert_element_;

  QBuffer buffer_;
};

#endif  // CHROMAPRINTER_H
