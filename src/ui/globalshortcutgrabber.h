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

#ifndef GLOBALSHORTCUTGRABBER_H
#define GLOBALSHORTCUTGRABBER_H

#include <QDialog>

class MacMonitorWrapper;
class Ui_GlobalShortcutGrabber;

#ifdef __OBJC__
@class NSEvent;
#else
class NSEvent;
#endif

class GlobalShortcutGrabber : public QDialog {
  Q_OBJECT

 public:
  GlobalShortcutGrabber(QWidget* parent = nullptr);
  ~GlobalShortcutGrabber();

  QKeySequence GetKey(const QString& name);

 protected:
  bool event(QEvent*);
  void showEvent(QShowEvent*);
  void hideEvent(QHideEvent*);
  void grabKeyboard();
  void releaseKeyboard();

 private:
  void UpdateText();
  void SetupMacEventHandler();
  void TeardownMacEventHandler();
  bool HandleMacEvent(NSEvent*);

  Ui_GlobalShortcutGrabber* ui_;
  QKeySequence ret_;

  QList<int> modifier_keys_;

  MacMonitorWrapper* wrapper_{};
};

#endif  // GLOBALSHORTCUTGRABBER_H
