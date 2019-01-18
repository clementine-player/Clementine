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

#include "devicemanager.h"
#include "mtpconnection.h"
#include "mtpdevice.h"
#include "mtploader.h"
#include "core/application.h"
#include "core/logging.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include <libmtp.h>

#include <QFile>
#include <QThread>

bool MtpDevice::sInitialisedLibMTP = false;

MtpDevice::MtpDevice(const QUrl& url, DeviceLister* lister,
                     const QString& unique_id, DeviceManager* manager,
                     Application* app, int database_id, bool first_time)
    : ConnectedDevice(url, lister, unique_id, manager, app, database_id,
                      first_time),
      loader_thread_(new QThread(this)),
      loader_(nullptr) {
  if (!sInitialisedLibMTP) {
    LIBMTP_Init();
    sInitialisedLibMTP = true;
  }
}

MtpDevice::~MtpDevice() {}

void MtpDevice::Init() {
  InitBackendDirectory("/", first_time_, false);
  model_->Init();

  loader_ =
      new MtpLoader(url_, app_->task_manager(), backend_, shared_from_this());
  loader_->moveToThread(loader_thread_);

  connect(loader_, SIGNAL(Error(QString)), SLOT(LoaderError(QString)));
  connect(loader_, SIGNAL(TaskStarted(int)), SIGNAL(TaskStarted(int)));
  connect(loader_, SIGNAL(LoadFinished(bool)), SLOT(LoadFinished(bool)));
  connect(loader_thread_, SIGNAL(started()), loader_, SLOT(LoadDatabase()));
}

void MtpDevice::ConnectAsync() {
  db_busy_.lock();
  loader_thread_->start();
}

void MtpDevice::LoadFinished(bool success) {
  loader_->deleteLater();
  loader_ = nullptr;
  db_busy_.unlock();
  emit ConnectFinished(unique_id_, success);
}

void MtpDevice::LoaderError(const QString& message) { app_->AddError(message); }

bool MtpDevice::StartCopy(QList<Song::FileType>* supported_types) {
  // Ensure only one "organise files" can be active at any one time
  db_busy_.lock();

  // Connect to the device
  connection_.reset(new MtpConnection(url_));

  // Did the caller want a list of supported types?
  if (supported_types) {
    if (!GetSupportedFiletypes(supported_types, connection_->device())) {
      FinishCopy(false);
      return false;
    }
  }

  return true;
}

static int ProgressCallback(uint64_t const sent, uint64_t const total,
                            void const* const data) {
  const MusicStorage::CopyJob* job =
      reinterpret_cast<const MusicStorage::CopyJob*>(data);
  job->progress_(float(sent) / total);

  return 0;
}

bool MtpDevice::CopyToStorage(const CopyJob& job) {
  if (!connection_->is_valid()) return false;

  // Convert metadata
  LIBMTP_track_t track;
  job.metadata_.ToMTP(&track);

  // Send the file
  int ret = LIBMTP_Send_Track_From_File(connection_->device(),
                                        job.source_.toUtf8().constData(),
                                        &track, ProgressCallback, &job);
  if (ret != 0) return false;

  // Add it to our LibraryModel
  Song metadata_on_device;
  metadata_on_device.InitFromMTP(&track, url_.host());
  metadata_on_device.set_directory_id(1);
  songs_to_add_ << metadata_on_device;

  // Remove the original if requested
  if (job.remove_original_) {
    if (!QFile::remove(job.source_)) return false;
  }

  return true;
}

void MtpDevice::FinishCopy(bool success) {
  if (success) {
    if (!songs_to_add_.isEmpty()) backend_->AddOrUpdateSongs(songs_to_add_);
    if (!songs_to_remove_.isEmpty()) backend_->DeleteSongs(songs_to_remove_);
  }

  songs_to_add_.clear();
  songs_to_remove_.clear();

  connection_.reset();

  db_busy_.unlock();

  ConnectedDevice::FinishCopy(success);
}

void MtpDevice::StartDelete() { StartCopy(nullptr); }

bool MtpDevice::DeleteFromStorage(const DeleteJob& job) {
  // Extract the ID from the song's URL
  QString filename = job.metadata_.url().path();
  filename.remove('/');

  bool ok = false;
  uint32_t id = filename.toUInt(&ok);
  if (!ok) return false;

  // Remove the file
  int ret = LIBMTP_Delete_Object(connection_->device(), id);
  if (ret != 0) return false;

  // Remove it from our library model
  songs_to_remove_ << job.metadata_;

  return true;
}

void MtpDevice::FinishDelete(bool success) { FinishCopy(success); }

bool MtpDevice::GetSupportedFiletypes(QList<Song::FileType>* ret) {
  QMutexLocker l(&db_busy_);
  MtpConnection connection(url_);
  if (!connection.is_valid()) {
    qLog(Warning) << "Error connecting to MTP device, couldn't get list of "
                     "supported filetypes";
    return false;
  }

  return GetSupportedFiletypes(ret, connection.device());
}

bool MtpDevice::GetSupportedFiletypes(QList<Song::FileType>* ret,
                                      LIBMTP_mtpdevice_t* device) {
  uint16_t* list = nullptr;
  uint16_t length = 0;

  if (LIBMTP_Get_Supported_Filetypes(device, &list, &length) || !list ||
      !length)
    return false;

  for (int i = 0; i < length; ++i) {
    switch (LIBMTP_filetype_t(list[i])) {
      case LIBMTP_FILETYPE_WAV:
        *ret << Song::Type_Wav;
        break;
      case LIBMTP_FILETYPE_MP2:
      case LIBMTP_FILETYPE_MP3:
        *ret << Song::Type_Mpeg;
        break;
      case LIBMTP_FILETYPE_WMA:
        *ret << Song::Type_Asf;
        break;
      case LIBMTP_FILETYPE_MP4:
      case LIBMTP_FILETYPE_M4A:
      case LIBMTP_FILETYPE_AAC:
        *ret << Song::Type_Mp4;
        break;
      case LIBMTP_FILETYPE_FLAC:
        *ret << Song::Type_Flac;
        *ret << Song::Type_OggFlac;
        break;
      case LIBMTP_FILETYPE_OGG:
        *ret << Song::Type_OggVorbis;
        *ret << Song::Type_OggSpeex;
        *ret << Song::Type_OggFlac;
        break;
      default:
        qLog(Error) << "Unknown MTP file format"
                    << LIBMTP_Get_Filetype_Description(
                           LIBMTP_filetype_t(list[i]));
        break;
    }
  }

  free(list);
  return true;
}
