/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef APPEARANCESETTINGSPAGE_H
#define APPEARANCESETTINGSPAGE_H

#include "playlist/playlistview.h"
#include "settingspage.h"

class QWidget;

class Ui_AppearanceSettingsPage;

class AppearanceSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  AppearanceSettingsPage(SettingsDialog* dialog);
  ~AppearanceSettingsPage();

  void Load();
  void Save();
  void Cancel();

 private slots:
  void SelectForegroundColor();
  void SelectBackgroundColor();
  void UseCustomColorSetOptionChanged(bool);
  void SelectBackgroundImage();
  void BlurLevelChanged(int);
  void OpacityLevelChanged(int);
  void DisableBlurAndOpacitySliders(bool);

 private:
  static const int kMoodbarPreviewWidth;
  static const int kMoodbarPreviewHeight;

  // Set the widget's background to new_color
  void UpdateColorSelectorColor(QWidget* color_selector,
                                const QColor& new_color);
  // Init (or refresh) the colorSelectors colors
  void InitColorSelectorsColors();

  void InitMoodbarPreviews();

  Ui_AppearanceSettingsPage* ui_;
  bool original_use_a_custom_color_set_;
  QColor original_foreground_color_;
  QColor original_background_color_;
  QColor current_foreground_color_;
  QColor current_background_color_;
  PlaylistView::BackgroundImageType playlist_view_background_image_type_;
  QString playlist_view_background_image_filename_;

  bool initialised_moodbar_previews_;
};

#endif  // APPEARANCESETTINGSPAGE_H
