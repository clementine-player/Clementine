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

#include "appearancesettingspage.h"

#include <QApplication>
#include <QColorDialog>
#include <QFileDialog>
#include <QSettings>

#include "iconloader.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "ui_appearancesettingspage.h"
#include "core/appearance.h"
#include "core/logging.h"
#include "playlist/playlistview.h"
#include "ui/albumcoverchoicecontroller.h"

AppearanceSettingsPage::AppearanceSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    ui_(new Ui_AppearanceSettingsPage),
    original_use_a_custom_color_set_(false),
    playlist_view_background_image_type_(PlaylistView::Default)
{
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("view-media-visualization"));

  Load();

  connect(ui_->select_foreground_color, SIGNAL(pressed()), SLOT(SelectForegroundColor()));
  connect(ui_->select_background_color, SIGNAL(pressed()), SLOT(SelectBackgroundColor()));
  connect(ui_->use_a_custom_color_set, SIGNAL(toggled(bool)), SLOT(UseCustomColorSetOptionChanged(bool)));

  connect(ui_->select_background_image_filename_button, SIGNAL(pressed()), SLOT(SelectBackgroundImage()));
  connect(ui_->use_custom_background_image, SIGNAL(toggled(bool)),
      ui_->background_image_filename, SLOT(setEnabled(bool)));
  connect(ui_->use_custom_background_image, SIGNAL(toggled(bool)),
      ui_->select_background_image_filename_button, SLOT(setEnabled(bool)));
}

AppearanceSettingsPage::~AppearanceSettingsPage() {
  delete ui_;
}

void AppearanceSettingsPage::Load() {
  QSettings s;
  s.beginGroup(Appearance::kSettingsGroup);

  QPalette p = QApplication::palette();

  // Keep in mind originals colors, in case the user clicks on Cancel, to be
  // able to restore colors
  original_use_a_custom_color_set_ = s.value(Appearance::kUseCustomColorSet, false).toBool();
  
  original_foreground_color_  = s.value(Appearance::kForegroundColor,
                                        p.color(QPalette::WindowText)).value<QColor>();
  current_foreground_color_   = original_foreground_color_;
  original_background_color_  = s.value(Appearance::kBackgroundColor,
                                        p.color(QPalette::Window)).value<QColor>();
  current_background_color_   = original_background_color_;

  InitColorSelectorsColors();

  QSettings playlist_settings;
  playlist_settings.beginGroup(Playlist::kSettingsGroup);
  playlist_view_background_image_type_ =
      static_cast<PlaylistView::BackgroundImageType>(
          playlist_settings.value(PlaylistView::kSettingBackgroundImageType).toInt());
  playlist_view_background_image_filename_  =
      playlist_settings.value(PlaylistView::kSettingBackgroundImageFilename).toString();

  ui_->use_system_color_set->setChecked(!original_use_a_custom_color_set_);
  ui_->use_a_custom_color_set->setChecked(original_use_a_custom_color_set_);

  switch (playlist_view_background_image_type_) {
    case PlaylistView::None:
      ui_->use_no_background->setChecked(true);
      break;
    case PlaylistView::AlbumCover:
      ui_->use_album_cover_background->setChecked(true);
      break;
    case PlaylistView::Custom:
      ui_->use_custom_background_image->setChecked(true);
      break;
    case PlaylistView::Default:
    default:
      ui_->use_default_background->setChecked(true);
  }
  ui_->background_image_filename->setText(playlist_view_background_image_filename_);
}

void AppearanceSettingsPage::Save() {
  QSettings s;
  s.beginGroup(Appearance::kSettingsGroup);
  bool use_a_custom_color_set = ui_->use_a_custom_color_set->isChecked();
  s.setValue(Appearance::kUseCustomColorSet, use_a_custom_color_set);
  if (use_a_custom_color_set) {
    s.setValue(Appearance::kBackgroundColor, current_background_color_);
    s.setValue(Appearance::kForegroundColor, current_foreground_color_);
  } else {
    dialog()->appearance()->ResetToSystemDefaultTheme();
  }

  QSettings playlist_settings;
  playlist_settings.beginGroup(Playlist::kSettingsGroup);
  playlist_view_background_image_filename_ = ui_->background_image_filename->text();
  if (ui_->use_no_background->isChecked()) {
    playlist_view_background_image_type_ = PlaylistView::None;
  } else if (ui_->use_album_cover_background->isChecked()) {
    playlist_view_background_image_type_ = PlaylistView::AlbumCover;
  } else if (ui_->use_default_background->isChecked()) {
    playlist_view_background_image_type_ = PlaylistView::Default;
  } else if (ui_->use_custom_background_image->isChecked()) {
    playlist_view_background_image_type_ = PlaylistView::Custom;
    playlist_settings.setValue(PlaylistView::kSettingBackgroundImageFilename,
        playlist_view_background_image_filename_);
  }
  playlist_settings.setValue(PlaylistView::kSettingBackgroundImageType,
      playlist_view_background_image_type_);
}

void AppearanceSettingsPage::Cancel() {
  if (original_use_a_custom_color_set_) {
    dialog()->appearance()->ChangeForegroundColor(original_foreground_color_);
    dialog()->appearance()->ChangeBackgroundColor(original_background_color_);
  } else {
    dialog()->appearance()->ResetToSystemDefaultTheme();
  }
}

void AppearanceSettingsPage::SelectForegroundColor() {
  QColor color_selected = QColorDialog::getColor(current_foreground_color_);
  if (!color_selected.isValid())
    return;
  
  current_foreground_color_ = color_selected;
  dialog()->appearance()->ChangeForegroundColor(color_selected);

  UpdateColorSelectorColor(ui_->select_foreground_color, color_selected);
}

void AppearanceSettingsPage::SelectBackgroundColor() {
  QColor color_selected = QColorDialog::getColor(current_background_color_);
  if (!color_selected.isValid())
    return;
  
  current_background_color_ = color_selected;
  dialog()->appearance()->ChangeBackgroundColor(color_selected);

  UpdateColorSelectorColor(ui_->select_background_color, color_selected);
}

void AppearanceSettingsPage::UseCustomColorSetOptionChanged(bool checked) {
  if (checked) {
    dialog()->appearance()->ChangeForegroundColor(current_foreground_color_);
    dialog()->appearance()->ChangeBackgroundColor(current_background_color_);
  } else {
    dialog()->appearance()->ResetToSystemDefaultTheme();
  }
}

void AppearanceSettingsPage::InitColorSelectorsColors() {
  UpdateColorSelectorColor(ui_->select_foreground_color, current_foreground_color_);
  UpdateColorSelectorColor(ui_->select_background_color, current_background_color_);
}

void AppearanceSettingsPage::UpdateColorSelectorColor(QWidget* color_selector, const QColor& color) {
  QString css = QString("background-color: rgb(%1, %2, %3); color: rgb(255, 255, 255)")
      .arg(color.red())
      .arg(color.green())
      .arg(color.blue());
  color_selector->setStyleSheet(css);
}

void AppearanceSettingsPage::SelectBackgroundImage() {
  QString selected_filename =
    QFileDialog::getOpenFileName(this, tr("Select background image"),
      playlist_view_background_image_filename_,
      tr(AlbumCoverChoiceController::kLoadImageFileFilter) + ";;" +
        tr(AlbumCoverChoiceController::kAllFilesFilter));
  if (selected_filename.isEmpty())
    return;
  playlist_view_background_image_filename_ = selected_filename;
  ui_->background_image_filename->setText(playlist_view_background_image_filename_);
}
