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

#include "windowsairplaydiscovery.h"

#include "core/logging.h"
#include "networkremote/windowsdnssdbrowser.h"

WindowsAirPlayDiscovery::WindowsAirPlayDiscovery(const char* service_type,
                                                 QObject* parent)
    : AirPlayDiscovery(service_type, parent),
      browser_(new WindowsDnsSdBrowser(service_type, this)) {
  connect(browser_, &WindowsDnsSdBrowser::ServiceAdded, this,
          [this](const QString& service) { ServiceAdded(service); });
  connect(browser_, &WindowsDnsSdBrowser::ServiceRemoved, this,
          [this](const QString& service) { ServiceRemoved(service); });
  connect(browser_, &WindowsDnsSdBrowser::ServiceResolved, this,
          [this](const QString& service,
                 const WindowsDnsSdBrowser::Service& resolved) {
            AirPlayDevice device;
            // The instance name, up to the service type.
            const QString instance =
                resolved.instance.section(QString(".") + service_type(), 0, 0);
            if (!ParseService(instance, resolved.txt, &device)) {
              qLog(Debug) << "AirPlay service" << service << "has no device id";
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

WindowsAirPlayDiscovery::~WindowsAirPlayDiscovery() {}

void WindowsAirPlayDiscovery::Start() {
  if (!WindowsDnsSdBrowser::Available()) {
    qLog(Warning) << "This version of Windows can't look for AirPlay receivers";
    return;
  }
  if (browser_->Start()) qLog(Debug) << "Looking for AirPlay receivers";
}
