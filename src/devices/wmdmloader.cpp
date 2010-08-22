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

#include "devicelister.h"
#include "wmdmdevice.h"
#include "wmdmlister.h"
#include "wmdmloader.h"
#include "wmdmthread.h"
#include "core/taskmanager.h"
#include "library/librarybackend.h"

#include <boost/scoped_array.hpp>

#include <cwchar>
#include <mswmdm.h>

#include <QUuid>

// Copied from the WMDM SDK 11, it doesn't seem to be included in SDK 9 for
// some reason, even though the devices use it.
typedef enum tagWMDM_FORMATCODE{
  WMDM_FORMATCODE_NOTUSED  =  0x0000,
  WMDM_FORMATCODE_ALLIMAGES  =  0xFFFFFFFF,
  WMDM_FORMATCODE_UNDEFINED  =  0x3000,
  WMDM_FORMATCODE_ASSOCIATION  =  0x3001,
  WMDM_FORMATCODE_SCRIPT  =  0x3002,
  WMDM_FORMATCODE_EXECUTABLE  =  0x3003,
  WMDM_FORMATCODE_TEXT  =  0x3004,
  WMDM_FORMATCODE_HTML  =  0x3005,
  WMDM_FORMATCODE_DPOF  =  0x3006,
  WMDM_FORMATCODE_AIFF  =  0x3007,
  WMDM_FORMATCODE_WAVE  =  0x3008,
  WMDM_FORMATCODE_MP3  =  0x3009,
  WMDM_FORMATCODE_AVI  =  0x300A,
  WMDM_FORMATCODE_MPEG  =  0x300B,
  WMDM_FORMATCODE_ASF  =  0x300C,
  WMDM_FORMATCODE_RESERVED_FIRST  =  0x300D,
  WMDM_FORMATCODE_RESERVED_LAST  =  0x37FF,
  WMDM_FORMATCODE_IMAGE_UNDEFINED  =  0x3800,
  WMDM_FORMATCODE_IMAGE_EXIF  =  0x3801,
  WMDM_FORMATCODE_IMAGE_TIFFEP  =  0x3802,
  WMDM_FORMATCODE_IMAGE_FLASHPIX  =  0x3803,
  WMDM_FORMATCODE_IMAGE_BMP  =  0x3804,
  WMDM_FORMATCODE_IMAGE_CIFF  =  0x3805,
  WMDM_FORMATCODE_IMAGE_GIF  =  0x3807,
  WMDM_FORMATCODE_IMAGE_JFIF  =  0x3808,
  WMDM_FORMATCODE_IMAGE_PCD  =  0x3809,
  WMDM_FORMATCODE_IMAGE_PICT  =  0x380A,
  WMDM_FORMATCODE_IMAGE_PNG  =  0x380B,
  WMDM_FORMATCODE_IMAGE_TIFF  =  0x380D,
  WMDM_FORMATCODE_IMAGE_TIFFIT  =  0x380E,
  WMDM_FORMATCODE_IMAGE_JP2  =  0x380F,
  WMDM_FORMATCODE_IMAGE_JPX  =  0x3810,
  WMDM_FORMATCODE_IMAGE_RESERVED_FIRST  =  0x3811,
  WMDM_FORMATCODE_IMAGE_RESERVED_LAST  =  0x3FFF,
  WMDM_FORMATCODE_UNDEFINEDFIRMWARE  =  0xB802,
  WMDM_FORMATCODE_WINDOWSIMAGEFORMAT  =  0xB881,
  WMDM_FORMATCODE_UNDEFINEDAUDIO  =  0xB900,
  WMDM_FORMATCODE_WMA  =  0xB901,
  WMDM_FORMATCODE_OGG = 0xB902,
  WMDM_FORMATCODE_AAC  = 0xB903,
  WMDM_FORMATCODE_AUDIBLE = 0xB904,
  WMDM_FORMATCODE_FLAC = 0xB906,
  WMDM_FORMATCODE_UNDEFINEDVIDEO  =  0xB980,
  WMDM_FORMATCODE_WMV  =  0xB981,
  WMDM_FORMATCODE_MP4 = 0xB982,
  WMDM_FORMATCODE_MP2 = 0xB983,
  WMDM_FORMATCODE_UNDEFINEDCOLLECTION  =  0xBA00,
  WMDM_FORMATCODE_ABSTRACTMULTIMEDIAALBUM  =  0xBA01,
  WMDM_FORMATCODE_ABSTRACTIMAGEALBUM  =  0xBA02,
  WMDM_FORMATCODE_ABSTRACTAUDIOALBUM  =  0xBA03,
  WMDM_FORMATCODE_ABSTRACTVIDEOALBUM  =  0xBA04,
  WMDM_FORMATCODE_ABSTRACTAUDIOVIDEOPLAYLIST  =  0xBA05,
  WMDM_FORMATCODE_ABSTRACTCONTACTGROUP  =  0xBA06,
  WMDM_FORMATCODE_ABSTRACTMESSAGEFOLDER  =  0xBA07,
  WMDM_FORMATCODE_ABSTRACTCHAPTEREDPRODUCTION  =  0xBA08,
  WMDM_FORMATCODE_WPLPLAYLIST  =  0xBA10,
  WMDM_FORMATCODE_M3UPLAYLIST  =  0xBA11,
  WMDM_FORMATCODE_MPLPLAYLIST  =  0xBA12,
  WMDM_FORMATCODE_ASXPLAYLIST  =  0xBA13,
  WMDM_FORMATCODE_PLSPLAYLIST  =  0xBA14,
  WMDM_FORMATCODE_UNDEFINEDDOCUMENT  =  0xBA80,
  WMDM_FORMATCODE_ABSTRACTDOCUMENT  =  0xBA81,
  WMDM_FORMATCODE_XMLDOCUMENT = 0xBA82,
  WMDM_FORMATCODE_MICROSOFTWORDDOCUMENT= 0xBA83,
  WMDM_FORMATCODE_MHTCOMPILEDHTMLDOCUMENT = 0xBA84,
  WMDM_FORMATCODE_MICROSOFTEXCELSPREADSHEET = 0xBA85,
  WMDM_FORMATCODE_MICROSOFTPOWERPOINTDOCUMENT = 0xBA86,
  WMDM_FORMATCODE_UNDEFINEDMESSAGE  =  0xBB00,
  WMDM_FORMATCODE_ABSTRACTMESSAGE  =  0xBB01,
  WMDM_FORMATCODE_UNDEFINEDCONTACT  =  0xBB80,
  WMDM_FORMATCODE_ABSTRACTCONTACT  =  0xBB81,
  WMDM_FORMATCODE_VCARD2  =  0xBB82,
  WMDM_FORMATCODE_VCARD3  =  0xBB83,
  WMDM_FORMATCODE_UNDEFINEDCALENDARITEM  =  0xBE00,
  WMDM_FORMATCODE_ABSTRACTCALENDARITEM  =  0xBE01,
  WMDM_FORMATCODE_VCALENDAR1  =  0xBE02,
  WMDM_FORMATCODE_VCALENDAR2 = 0xBE03,
  WMDM_FORMATCODE_UNDEFINEDWINDOWSEXECUTABLE  =  0xBE80,
  WMDM_FORMATCODE_MEDIA_CAST = 0xBE81,
  WMDM_FORMATCODE_SECTION = 0xBE82
} WMDM_FORMATCODE;



WmdmLoader::WmdmLoader(TaskManager* task_manager, LibraryBackend* backend,
                       boost::shared_ptr<ConnectedDevice> device)
  : QObject(NULL),
    device_(device),
    task_manager_(task_manager),
    backend_(backend)
{
  original_thread_ = thread();
}

WmdmLoader::~WmdmLoader() {
}

void WmdmLoader::LoadDatabase() {
  int task_id = task_manager_->StartTask(tr("Loading Windows Media device"));
  emit TaskStarted(task_id);

  boost::scoped_ptr<WmdmThread> thread(new WmdmThread);

  // Get the device's canonical name
  boost::shared_ptr<WmdmDevice> connected_device =
      boost::static_pointer_cast<WmdmDevice>(device_);
  WmdmLister* lister = static_cast<WmdmLister*>(connected_device->lister());
  QString canonical_name = lister->DeviceCanonicalName(connected_device->unique_id());

  // Find the device
  boost::scoped_array<wchar_t> name(new wchar_t[canonical_name.length() + 1]);
  canonical_name.toWCharArray(name.get());
  name[canonical_name.length()] = '\0';

  IWMDMDevice* device = NULL;
  if (thread->manager()->GetDeviceFromCanonicalName(name.get(), &device)) {
    qWarning() << "Error in GetDeviceFromCanonicalName for" << canonical_name;
    return;
  }

  // Get the list of storages from the device
  IWMDMEnumStorage* storage_it = NULL;
  device->EnumStorage(&storage_it);

  ULONG storage_fetched = 0;
  IWMDMStorage* storage = NULL;
  while (storage_it->Next(1, &storage, &storage_fetched) == S_OK) {
    RecursiveExploreStorage(storage);
    storage->Release();
  }

  storage_it->Release();
  device->Release();

  thread.reset();

  // Need to remove all the existing songs in the database first
  backend_->DeleteSongs(backend_->FindSongsInDirectory(1));

  // Add the songs we've just loaded
  backend_->AddOrUpdateSongs(songs_);

  task_manager_->SetTaskFinished(task_id);
  emit LoadFinished();
}

void WmdmLoader::RecursiveExploreStorage(IWMDMStorage* parent) {
  IWMDMEnumStorage* child_it = NULL;
  parent->EnumStorage(&child_it);

  IWMDMStorage* child = NULL;
  ULONG num_retreived = 0;
  while (child_it->Next(1, &child, &num_retreived) == S_OK && num_retreived == 1) {
    const int kMaxLen = 255;
    wchar_t name[kMaxLen];
    child->GetName(name, kMaxLen);

    DWORD attributes = 0;
    _WAVEFORMATEX audio_format;
    child->GetAttributes(&attributes, &audio_format);

    if (attributes & WMDM_FILE_ATTR_FILE) {
      LoadFile(child);
    } else if (attributes & WMDM_FILE_ATTR_FOLDER) {
      RecursiveExploreStorage(child);
    }

    child->Release();
  }
  child_it->Release();
}

void WmdmLoader::LoadFile(IWMDMStorage* file) {
  // Convert to a IWMDMStorage3 so we can get metadata
  IWMDMStorage3* storage3 = NULL;
  if (file->QueryInterface(IID_IWMDMStorage3, (void**) &storage3))
    return;

  // Get the metadata interface
  IWMDMMetaData* metadata = NULL;
  if (storage3->GetMetadata(&metadata)) {
    storage3->Release();
    return;
  }
  storage3->Release();

  // Store the metadata in here
  Song song;
  bool non_consumable = false;
  int format = 0;

  // How much metadata is there?
  uint count = 0;
  metadata->GetItemCount(&count);

  for (int i=0 ; i<count ; ++i) {
    // Get this metadata item
    wchar_t* name = NULL;
    WMDM_TAG_DATATYPE type;
    BYTE* value = NULL;
    uint length = 0;

    metadata->QueryByIndex(i, &name, &type, &value, &length);

    QVariant item_value = ReadValue(type, value, length);

    // Store it in the song if it's something we recognise
    if (wcscmp(name, g_wszWMDMTitle) == 0)
      song.set_title(item_value.toString());
    else if (wcscmp(name, g_wszWMDMAuthor) == 0)
      song.set_artist(item_value.toString());
    else if (wcscmp(name, g_wszWMDMDescription) == 0)
      song.set_comment(item_value.toString());
    else if (wcscmp(name, g_wszWMDMAlbumTitle) == 0)
      song.set_album(item_value.toString());
    else if (wcscmp(name, g_wszWMDMTrack) == 0)
      song.set_track(item_value.toInt());
    else if (wcscmp(name, g_wszWMDMGenre) == 0)
      song.set_genre(item_value.toString());
    else if (wcscmp(name, g_wszWMDMYear) == 0)
      song.set_year(item_value.toInt());
    else if (wcscmp(name, g_wszWMDMComposer) == 0)
      song.set_composer(item_value.toString());
    else if (wcscmp(name, g_wszWMDMBitrate) == 0)
      song.set_bitrate(item_value.toInt());
    else if (wcscmp(name, g_wszWMDMFileName) == 0)
      song.set_filename(item_value.toString());
    else if (wcscmp(name, g_wszWMDMDuration) == 0)
      song.set_length(item_value.toULongLong() / 10000000ll);
    else if (wcscmp(name, L"WMDM/FileSize") == 0)
      song.set_filesize(item_value.toULongLong());
    else if (wcscmp(name, L"WMDM/NonConsumable") == 0)
      non_consumable = item_value.toBool();
    else if (wcscmp(name, L"WMDM/FormatCode") == 0)
      format = item_value.toInt();

    CoTaskMemFree(name);
    CoTaskMemFree(value);
  }

  metadata->Release();

  // Decide if this is music or not
  if (count == 0 || non_consumable)
    return;

  switch (format) {
  case WMDM_FORMATCODE_AIFF:
    song.set_filetype(Song::Type_Aiff);
    break;

  case WMDM_FORMATCODE_WAVE:
    song.set_filetype(Song::Type_Wav);
    break;

  case WMDM_FORMATCODE_MP2:
  case WMDM_FORMATCODE_MP3:
  case WMDM_FORMATCODE_MPEG:
    song.set_filetype(Song::Type_Mpeg);
    break;

  case WMDM_FORMATCODE_WMA:
  case WMDM_FORMATCODE_ASF:
    song.set_filetype(Song::Type_Asf);
    break;

  case WMDM_FORMATCODE_OGG:
    song.set_filetype(Song::Type_OggVorbis);
    break;

  case WMDM_FORMATCODE_AAC:
  case WMDM_FORMATCODE_MP4:
    song.set_filetype(Song::Type_Mp4);
    break;

  case WMDM_FORMATCODE_FLAC:
    song.set_filetype(Song::Type_Flac);
    break;

  case WMDM_FORMATCODE_AUDIBLE:
  case WMDM_FORMATCODE_UNDEFINEDAUDIO:
    song.set_filetype(Song::Type_Unknown);
    break;

  default:
    return; // It's not music
  }

  song.set_directory_id(1);
  song.set_valid(true);
  song.set_mtime(0);
  song.set_ctime(0);
  songs_ << song;
}

QVariant WmdmLoader::ReadValue(int type, uchar* data, uint length) {
  switch (type) {
  case WMDM_TYPE_DWORD:
    return QVariant::fromValue(uint(*reinterpret_cast<DWORD*>(data)));
  case WMDM_TYPE_WORD:
    return QVariant::fromValue(uint(*reinterpret_cast<WORD*>(data)));
  case WMDM_TYPE_QWORD:
    return QVariant::fromValue(qulonglong(*reinterpret_cast<quint64*>(data)));
  case WMDM_TYPE_STRING:
    return QString::fromWCharArray(reinterpret_cast<wchar_t*>(data));
  case WMDM_TYPE_BINARY:
    return QByteArray(reinterpret_cast<char*>(data), length);
  case WMDM_TYPE_BOOL:
    return bool(*reinterpret_cast<int*>(data));
  case WMDM_TYPE_GUID:
    return QUuid(*reinterpret_cast<GUID*>(data)).toString();
  }
  return QVariant();
}
