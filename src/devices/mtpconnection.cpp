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

#include "mtpconnection.h"
#include "core/logging.h"

#include <QRegExp>
#include <QtDebug>

MtpConnection::MtpConnection(const QUrl& url) : device_(nullptr) {
  QString hostname = url.host();
  // Parse the URL
  QRegExp host_re("^usb-(\\d+)-(\\d+)$");

  unsigned int bus_location;
  unsigned int device_num;

  if (host_re.indexIn(hostname) >= 0) {
    bus_location = host_re.cap(1).toUInt();
    device_num = host_re.cap(2).toUInt();
  } else if (url.hasQueryItem("busnum")) {
    bus_location = url.queryItemValue("busnum").toUInt();
    device_num = url.queryItemValue("devnum").toUInt();
  } else {
    qLog(Warning) << "Invalid MTP device:" << hostname;
    return;
  }

  if (url.hasQueryItem("vendor")) {
    LIBMTP_raw_device_t* raw_device =
        (LIBMTP_raw_device_t*)malloc(sizeof(LIBMTP_raw_device_t));
    raw_device->device_entry.vendor =
        url.queryItemValue("vendor").toAscii().data();
    raw_device->device_entry.product =
        url.queryItemValue("product").toAscii().data();
    raw_device->device_entry.vendor_id =
        url.queryItemValue("vendor_id").toUShort();
    raw_device->device_entry.product_id =
        url.queryItemValue("product_id").toUShort();
    raw_device->device_entry.device_flags =
        url.queryItemValue("quirks").toUInt();

    raw_device->bus_location = bus_location;
    raw_device->devnum = device_num;

    device_ = LIBMTP_Open_Raw_Device(raw_device);
    return;
  }

  // Get a list of devices from libmtp and figure out which one is ours
  int count = 0;
  LIBMTP_raw_device_t* raw_devices = nullptr;
  LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&raw_devices, &count);
  if (err != LIBMTP_ERROR_NONE) {
    qLog(Warning) << "MTP error:" << err;
    return;
  }

  LIBMTP_raw_device_t* raw_device = nullptr;
  for (int i = 0; i < count; ++i) {
    if (raw_devices[i].bus_location == bus_location &&
        raw_devices[i].devnum == device_num) {
      raw_device = &raw_devices[i];
      break;
    }
  }

  if (!raw_device) {
    qLog(Warning) << "MTP device not found";
    free(raw_devices);
    return;
  }

  // Connect to the device
  device_ = LIBMTP_Open_Raw_Device(raw_device);

  free(raw_devices);
}

MtpConnection::~MtpConnection() {
  if (device_) LIBMTP_Release_Device(device_);
}
