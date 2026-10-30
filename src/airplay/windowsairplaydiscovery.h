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

#ifndef AIRPLAY_WINDOWSAIRPLAYDISCOVERY_H_
#define AIRPLAY_WINDOWSAIRPLAYDISCOVERY_H_

#include "airplaydiscovery.h"

class WindowsDnsSdBrowser;

// Browses with the mDNS responder built into Windows 10 1809 and later,
// through a WindowsDnsSdBrowser.
class WindowsAirPlayDiscovery : public AirPlayDiscovery {
  Q_OBJECT

 public:
  explicit WindowsAirPlayDiscovery(QObject* parent = nullptr);
  ~WindowsAirPlayDiscovery();

  void Start() override;

 private:
  WindowsDnsSdBrowser* browser_;
};

#endif  // AIRPLAY_WINDOWSAIRPLAYDISCOVERY_H_
