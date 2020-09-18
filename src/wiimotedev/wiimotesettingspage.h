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

#ifndef WIIMOTESETTINGSPAGE_H
#define WIIMOTESETTINGSPAGE_H

#include <QSettings>

#include "ui/settingspage.h"

class QTreeWidgetItem;
class Ui_WiimoteSettingsPage;

class WiimoteSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  WiimoteSettingsPage(SettingsDialog* dialog);

  void Load();
  void Save();

  QString GetReadableWiiremoteSequence(quint64 value);
  QStringList TextActions() const { return text_actions_.values(); }

 signals:
  void SetWiimotedevInterfaceActived(bool);

 private slots:
  void AddAction();
  void AddShortcut(quint64 button, quint32 action);
  void DefaultSettings();
  void DeleteAction();
  void ItemClicked(QTreeWidgetItem*);

 private:
  struct Shortcut {
    QTreeWidgetItem* object;
    quint64 button;
    quint32 action;
  };

  Ui_WiimoteSettingsPage* ui_;

  QString current_id_;

  QList<Shortcut> actions_;
  QMap<quint64, QString> text_buttons_;
  QMap<quint32, QString> text_actions_;

  QTreeWidgetItem* selected_item_;
};

#endif  // WIIMOTESETTINGSPAGE_H
