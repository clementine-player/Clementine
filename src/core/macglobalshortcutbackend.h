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

#ifndef MACGLOBALSHORTCUTBACKEND_H
#define MACGLOBALSHORTCUTBACKEND_H

#include <memory>

#include "globalshortcutbackend.h"

#include <QKeySequence>
#include <QMap>

class MacGlobalShortcutBackendPrivate;
class QAction;

class MacGlobalShortcutBackend : public GlobalShortcutBackend {
  Q_OBJECT

 public:
  MacGlobalShortcutBackend(GlobalShortcuts* parent);
  virtual ~MacGlobalShortcutBackend();

  bool IsAccessibilityEnabled() const;
  void ShowAccessibilityDialog();

  void MacMediaKeyPressed(int key);

 protected:
  bool DoRegister();
  void DoUnregister();

 private:
  bool KeyPressed(const QKeySequence& sequence);

  QMap<QKeySequence, QAction*> shortcuts_;

  friend class MacGlobalShortcutBackendPrivate;
  std::unique_ptr<MacGlobalShortcutBackendPrivate> p_;
};

#endif  // MACGLOBALSHORTCUTBACKEND_H
