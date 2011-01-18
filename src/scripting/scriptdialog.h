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

#ifndef SCRIPTDIALOG_H
#define SCRIPTDIALOG_H

#include <QDialog>
#include <QFont>
#include <QFontMetrics>
#include <QStyledItemDelegate>

class ScriptManager;
class Ui_ScriptDialog;

class ScriptDelegate : public QStyledItemDelegate {
public:
  ScriptDelegate(QObject* parent = 0);

  static const int kIconSize;
  static const int kPadding;
  static const int kItemHeight;
  static const int kLinkSpacing;

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const;
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const;

private:
  QFont bold_;
  QFontMetrics bold_metrics_;

  QFont link_;
};

class ScriptDialog : public QDialog {
  Q_OBJECT

public:
  ScriptDialog(QWidget* parent = 0);
  ~ScriptDialog();

  static const char* kSettingsGroup;

  void SetManager(ScriptManager* manager);

private slots:
  void DataChanged(const QModelIndex& top_left, const QModelIndex& bottom_right);
  void CurrentChanged(const QModelIndex& index);
  void LogLineAdded(const QString& html);

  void Enable();
  void Disable();
  void Settings();
  void Reload();

  void InstallFromFile();
  void InstallFromFileLoaded();

private:
  void ReloadSettings();

private:
  Ui_ScriptDialog* ui_;

  ScriptManager* manager_;

  QString last_open_dir_;
};

#endif // SCRIPTDIALOG_H
