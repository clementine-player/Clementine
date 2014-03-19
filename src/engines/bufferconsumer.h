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

#ifndef BUFFERCONSUMER_H
#define BUFFERCONSUMER_H

#include <gst/gstbuffer.h>

class GstEnginePipeline;

class BufferConsumer {
 public:
  virtual ~BufferConsumer() {}

  // This is called in some unspecified GStreamer thread.
  // Ownership of the buffer is transferred to the BufferConsumer and it should
  // gst_buffer_unref it.
  virtual void ConsumeBuffer(GstBuffer* buffer, int pipeline_id) = 0;
};

#endif  // BUFFERCONSUMER_H
