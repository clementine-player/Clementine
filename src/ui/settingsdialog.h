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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "config.h"
#include "ui_settingsdialog.h"

class LibraryDirectoryModel;
class OSDPretty;
#ifdef HAVE_GSTREAMER
class GstEngine;
#endif

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  SettingsDialog(QWidget* parent = 0);
  ~SettingsDialog();

  void SetLibraryDirectoryModel(LibraryDirectoryModel* model);
#ifdef HAVE_GSTREAMER
  void SetGstEngine(const GstEngine* engine);
#endif

  // QDialog
  void accept();

  // QWidget
  void showEvent(QShowEvent* e);
  void hideEvent(QHideEvent *);

 private slots:
  void CurrentTextChanged(const QString& text);
  void NotificationTypeChanged();
  void LastFMValidationComplete(bool success);

  void PrettyOpacityChanged(int value);
  void PrettyColorPresetChanged(int index);
  void ChooseBgColor();
  void ChooseFgColor();

  void UpdatePopupVisible();
  void ShowTrayIconToggled(bool on);
  void GstPluginChanged(int index);
  void FadingOptionsChanged();
  void RgPreampChanged(int value);

 private:
  Ui::SettingsDialog ui_;
  bool loading_settings_;

  OSDPretty* pretty_popup_;
};

#endif // SETTINGSDIALOG_H
