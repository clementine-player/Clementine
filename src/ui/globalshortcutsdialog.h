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

#ifndef GLOBALSHORTCUTSDIALOG_H
#define GLOBALSHORTCUTSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSettings>

#include <boost/scoped_ptr.hpp>

class QTreeWidgetItem;

class Ui_GlobalShortcutsDialog;
class GlobalShortcutGrabber;

class GlobalShortcutsDialog : public QDialog {
 Q_OBJECT

 public:
  GlobalShortcutsDialog(QWidget* parent = 0);
  ~GlobalShortcutsDialog();

  static const char* kSettingsGroup;

 private slots:
  void accept();

  void ResetAll();

  void ItemClicked(QTreeWidgetItem*);
  void NoneClicked();
  void DefaultClicked();
  void ChangeClicked();

  void Save();

 private:
  struct Shortcut {
    QString id;
    QString name;
    QTreeWidgetItem* item;

    QKeySequence default_key;
    QKeySequence key;
  };

  void AddShortcut(const QString& id, const QString& name,
                   const QKeySequence& default_key = QKeySequence(0));
  void SetShortcut(const QString& id, const QKeySequence& key);

 private:
  Ui_GlobalShortcutsDialog* ui_;

  boost::scoped_ptr<GlobalShortcutGrabber> grabber_;

  QSettings settings_;
  QMap<QString, Shortcut> shortcuts_;

  QString current_id_;
};

#endif // GLOBALSHORTCUTSDIALOG_H
