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

#ifndef MACGLOBALSHORTCUTBACKEND_H
#define MACGLOBALSHORTCUTBACKEND_H

#include "globalshortcutbackend.h"

#include <QKeySequence>
#include <QMap>

#include <boost/scoped_ptr.hpp>

class MacGlobalShortcutBackendPrivate;
class QAction;

class MacGlobalShortcutBackend : public GlobalShortcutBackend {
  Q_OBJECT

public:
  MacGlobalShortcutBackend(GlobalShortcuts* parent);
  virtual ~MacGlobalShortcutBackend();

  void MacMediaKeyPressed(int key);

protected:
  bool DoRegister();
  void DoUnregister();

private:
  void KeyPressed(const QKeySequence& sequence);
  bool CheckAccessibilityEnabled();

  QMap<QKeySequence, QAction*> shortcuts_;

  enum AccessibilityStatus {
    NOT_CHECKED,
    ENABLED,
    DISABLED
  } accessibility_status_;

  friend class MacGlobalShortcutBackendPrivate;
  boost::scoped_ptr<MacGlobalShortcutBackendPrivate> p_;
};

#endif // MACGLOBALSHORTCUTBACKEND_H
