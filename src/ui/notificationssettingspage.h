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

#ifndef NOTIFICATIONSSETTINGSPAGE_H
#define NOTIFICATIONSSETTINGSPAGE_H

#include "settingspage.h"
#include "widgets/osd.h"

class Ui_NotificationsSettingsPage;

class NotificationsSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  NotificationsSettingsPage(SettingsDialog* dialog);
  ~NotificationsSettingsPage();

  void Load();
  void Save();

 protected:
  void hideEvent(QHideEvent*);
  void showEvent(QShowEvent*);

 signals:
  void NotificationPreview(OSD::Behaviour, QString, QString);

 private slots:
  void NotificationTypeChanged();
  void NotificationCustomTextChanged(bool enabled);
  void PrepareNotificationPreview();
  void InsertVariableFirstLine(QAction* action);
  void InsertVariableSecondLine(QAction* action);
  void ShowMenuTooltip(QAction* action);

  void PrettyOpacityChanged(int value);
  void PrettyColorPresetChanged(int index);
  void ChooseBgColor();
  void ChooseFgColor();
  void ChooseFont();

  void UpdatePopupVisible();

 private:
  Ui_NotificationsSettingsPage* ui_;
  OSDPretty* pretty_popup_;
};

#endif  // NOTIFICATIONSSETTINGSPAGE_H
