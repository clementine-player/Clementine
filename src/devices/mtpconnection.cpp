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

#include "mtpconnection.h"

#include <QRegExp>
#include <QtDebug>

MtpConnection::MtpConnection(const QString& hostname)
  : device_(NULL)
{
  // Parse the URL
  QRegExp host_re("^usb-(\\d+)-(\\d+)$");

  if (host_re.indexIn(hostname) == -1) {
    qWarning() << "Invalid MTP device:" << hostname;
    return;
  }

  const unsigned int bus_location = host_re.cap(1).toInt();
  const unsigned int device_num = host_re.cap(2).toInt();

  // Get a list of devices from libmtp and figure out which one is ours
  int count = 0;
  LIBMTP_raw_device_t* raw_devices = NULL;
  LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&raw_devices, &count);
  if (err != LIBMTP_ERROR_NONE) {
    qWarning() << "MTP error:" << err;
    return;
  }

  LIBMTP_raw_device_t* raw_device = NULL;
  for (int i=0 ; i<count ; ++i) {
    if (raw_devices[i].bus_location == bus_location &&
        raw_devices[i].devnum == device_num) {
      raw_device = &raw_devices[i];
      break;
    }
  }

  if (!raw_device) {
    qWarning() << "MTP device not found";
    free(raw_devices);
    return;
  }

  // Connect to the device
  device_ = LIBMTP_Open_Raw_Device(raw_device);

  free(raw_devices);
}

MtpConnection::~MtpConnection() {
  if (device_)
    LIBMTP_Release_Device(device_);
}
