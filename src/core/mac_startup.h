/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2011-2012, John Maguire <john.maguire@gmail.com>

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

#ifndef MAC_STARTUP_H
#define MAC_STARTUP_H

#include <QKeySequence>

class MacGlobalShortcutBackend;
class QObject;
class QWidget;

class PlatformInterface {
 public:
  // Called when the application should show itself.
  virtual void Activate() = 0;
  virtual bool LoadUrl(const QString& url) = 0;

  virtual ~PlatformInterface() {}
};

namespace mac {

void MacMain();
void SetShortcutHandler(MacGlobalShortcutBackend* handler);
void SetApplicationHandler(PlatformInterface* handler);
void CheckForUpdates();

QString GetBundlePath();
QString GetResourcesPath();
QString GetApplicationSupportPath();
QString GetMusicDirectory();

void EnableFullScreen(const QWidget& main_window);

}  // namespace mac

#endif  // MAC_STARTUP_H
