/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "windowscastdiscovery.h"

#include "core/logging.h"
#include "networkremote/windowsdnssdbrowser.h"

WindowsCastDiscovery::WindowsCastDiscovery(QObject* parent)
    : CastDiscovery(parent),
      browser_(new WindowsDnsSdBrowser(kServiceType, this)) {
  connect(browser_, &WindowsDnsSdBrowser::ServiceAdded, this,
          [this](const QString& service) { ServiceAdded(service); });
  connect(browser_, &WindowsDnsSdBrowser::ServiceRemoved, this,
          [this](const QString& service) { ServiceRemoved(service); });
  connect(browser_, &WindowsDnsSdBrowser::ServiceResolved, this,
          [this](const QString& service,
                 const WindowsDnsSdBrowser::Service& resolved) {
            CastDevice device;
            if (!device.ParseTxt(resolved.txt)) {
              qLog(Debug) << "Cast service" << service << "has no id";
              return;
            }
            device.port = resolved.port;
            if (resolved.address.isNull()) {
              ResolveHost(service, device, resolved.host);
            } else {
              device.address = resolved.address;
              ServiceResolved(service, device);
            }
          });
}

WindowsCastDiscovery::~WindowsCastDiscovery() {}

void WindowsCastDiscovery::Start() {
  if (!WindowsDnsSdBrowser::Available()) {
    qLog(Warning) << "This version of Windows can't look for Cast devices";
    return;
  }
  if (browser_->Start()) qLog(Debug) << "Looking for Cast devices";
}
