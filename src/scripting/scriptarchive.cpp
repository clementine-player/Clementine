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

#include "config.h"
#include "scriptarchive.h"
#include "core/logging.h"
#include "core/utilities.h"

#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QtConcurrentRun>

#ifdef HAVE_LIBARCHIVE
# include <archive.h>
# include <archive_entry.h>
#endif

#ifdef HAVE_LIBARCHIVE
namespace {
  // Read callbacks for libarchive
  struct IODeviceReadState {
    QIODevice* device_;
    char buf_[4096];
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

  // Utility function to copy an entry to a QIODevice
  void CopyArchiveEntry(struct archive* in, struct archive* out) {
    char buf[4096];
    forever {
      size_t bytes_read = archive_read_data(in, buf, sizeof(buf));
      if (bytes_read == ARCHIVE_FATAL ||
          bytes_read == ARCHIVE_WARN ||
          bytes_read == ARCHIVE_RETRY) {
        qLog(Warning) << "Error reading archive:" << archive_error_string(in);
        return;
      }
      if (bytes_read == 0) {
        break;
      }

      if (archive_write_data(out, buf, bytes_read) == -1) {
        qLog(Warning) << "Error extracting archive:" << archive_error_string(out);
        return;
      }
    }
  }
}
#endif // HAVE_LIBARCHIVE

ScriptArchive::ScriptArchive(ScriptManager* manager)
  : manager_(manager)
{
}

ScriptArchive::~ScriptArchive() {
  if (!temp_dir_name_.isEmpty() && QFile::exists(temp_dir_name_)) {
    Utilities::RemoveRecursive(temp_dir_name_);
  }
}

QFuture<bool> ScriptArchive::LoadFromFileAsync(const QString& filename) {
  return QtConcurrent::run(this, &ScriptArchive::LoadFromFile, filename);
}

QFuture<bool> ScriptArchive::LoadFromDeviceAsync(QIODevice* device) {
  return QtConcurrent::run(this, &ScriptArchive::LoadFromDevice, device);
}

bool ScriptArchive::LoadFromFile(const QString& filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    return false;
  }

  return LoadFromDevice(&file);
}

bool ScriptArchive::LoadFromDevice(QIODevice* device) {
#ifdef HAVE_LIBARCHIVE
  archive* a = archive_read_new();
  archive_read_support_compression_gzip(a);
  archive_read_support_format_tar(a);

  IODeviceReadState read_state;
  read_state.device_ = device;

  // Open the archive
  if (archive_read_open(a, &read_state, NULL, IODeviceRead, IODeviceClose)) {
    archive_read_finish(a);
    return false;
  }

  info_.clear();

  // Open a writer to a location in /tmp
  temp_dir_name_ = Utilities::MakeTempDir();
  archive* writer = archive_write_disk_new();
  archive_write_disk_set_options(writer,
    ARCHIVE_EXTRACT_SECURE_SYMLINKS | ARCHIVE_EXTRACT_SECURE_NODOTDOT);

  archive_entry* entry;
  archive_entry* out_entry = archive_entry_new();
  while (archive_read_next_header(a, &entry) == 0) {
    // Figure out where we'll extract this file
    const QString pathname = QString::fromUtf8(archive_entry_pathname(entry));
    const QString destination = temp_dir_name_ + "/" + pathname;

    // Copy the header and change the path name
    archive_entry_clear(out_entry);
    archive_entry_set_size(out_entry, archive_entry_size(entry));
    archive_entry_set_filetype(out_entry, archive_entry_filetype(entry));
    archive_entry_set_mode(out_entry, archive_entry_mode(entry));
    archive_entry_copy_pathname(out_entry, destination.toLocal8Bit().constData());

    // Write the file
    archive_write_header(writer, out_entry);
    CopyArchiveEntry(a, writer);

    // Have we found a script.ini?
    const QStringList source_parts = pathname.split('/');

    if (source_parts.count() == 2 && source_parts[1] == ScriptInfo::kIniFileName) {
      // Parse it
      ScriptInfo info;
      info.InitFromFile(manager_,
                        destination.section('/', -2, -2),
                        destination.section('/', 0, -2),
                        destination);

      if (info.is_valid()) {
        info_ << info;
      }
    }
  }

  archive_entry_free(out_entry);
  archive_read_finish(a);

  return !info_.isEmpty();
#else // HAVE_LIBARCHIVE
  return false;
#endif // HAVE_LIBARCHIVE
}

bool ScriptArchive::Install() const {
  // Where should they go?
  QString destination = Utilities::GetConfigPath(Utilities::Path_Scripts);

  // Copy each directory
  foreach (const ScriptInfo& info, info_) {
    if (!Utilities::CopyRecursive(info.path(), destination))
      return false;
  }

  return true;
}
