/* This file is part of Clementine.
   Copyright 2021, Lukas Prediger <lumip@lumip.de>

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

#include "deviceerror.h"

#include <QString>

std::string DeviceError::CreateExceptionMessage(const QString& device_id,
                                                const QString error_message) {
  QString exception_message =
      QString("Error accessing device <%1>").arg(device_id);
  if (!error_message.isEmpty()) exception_message += ": " + error_message;

  return exception_message.toLocal8Bit().toStdString();
}

DeviceError::DeviceError(const QString& device_id, const QString& error_message)
    : std::runtime_error(CreateExceptionMessage(device_id, error_message)) {}

DeviceError::DeviceError(const QString& device_id)
    : DeviceError(device_id, "") {}

DeviceError::~DeviceError() {}
