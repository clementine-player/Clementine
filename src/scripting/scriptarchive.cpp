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

#include "scriptarchive.h"

#include <QFile>

#include <archive.h>
#include <archive_entry.h>

namespace {
  // Read callbacks for libarchive
  struct IODeviceReadState {
    QIODevice* device_;
    char buf_[1024];
  };

  ssize_t IODeviceRead(struct archive* a, void* client_data, const void** buf) {
    IODeviceReadState* state = reinterpret_cast<IODeviceReadState*>(client_data);

    *buf = state->buf_;
    return state->device_->read(state->buf_, sizeof(state->buf_));
  }

  int IODeviceClose(struct archive* a, void* client_data) {
    IODeviceReadState* state = reinterpret_cast<IODeviceReadState*>(client_data);

    state->device_->close();
    return 0;
  }
}

bool ScriptArchive::Load(const QString& filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    return false;
  }

  return Load(&file);
}

bool ScriptArchive::Load(QIODevice* device) {
  archive* a = archive_read_new();
  archive_read_support_compression_gzip(a);
  archive_read_support_format_tar(a);

  IODeviceReadState read_state;
  read_state.device_ = device;

  if (archive_read_open(a, &read_state, NULL, IODeviceRead, IODeviceClose)) {
    archive_read_finish(a);
    return false;
  }

  archive_entry* entry;
  while (archive_read_next_header(a, &entry) == 0) {
    // TODO
  }

  archive_read_finish(a);

  return true;
}
