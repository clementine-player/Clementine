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

#ifndef WIIMOTEDEVSHORTCUTSCONFIG_H
#define WIIMOTEDEVSHORTCUTSCONFIG_H

#include <QSettings>
#include <QWidget>

class QTreeWidgetItem;
class Ui_WiimotedevShortcutsConfig;
class WiimotedevShortcutGrabber;

class WiimotedevShortcutsConfig : public QWidget {
  Q_OBJECT
public:
  WiimotedevShortcutsConfig(QWidget* parent = 0);
  Ui_WiimotedevShortcutsConfig* ui_;

  struct Shortcut {
    QTreeWidgetItem* object;
    quint64 button;
    quint32 action;
  };

  QList <struct Shortcut> actions_;
  QMap <quint64, QString> text_buttons_;
  QMap <quint32, QString> text_actions_;

  QString GetReadableWiiremoteSequence(quint64 value);

private:
  void AddShortcut(quint64 button, quint32 action);

private:
  QSettings settings_;
  QString current_id_;

  QTreeWidgetItem* selected_item_;

private slots:
  void AddAction();
  void DefaultSettings();
  void DeleteAction();
  void LoadSettings();

  void WiimotedevEnabledChecked(bool checked);
  void ItemClicked(QTreeWidgetItem*);

signals:
  void SetWiimotedevInterfaceActived(bool);

};

#endif // WIIMOTEDEVSHORTCUTSCONFIG_H
