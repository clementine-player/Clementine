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
#include <QPainter>
#include <QSettings>

#include "config.h"
#include "iconloader.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "ui_appearancesettingspage.h"
#include "core/appearance.h"
#include "core/application.h"
#include "core/logging.h"
#include "playlist/playlistview.h"
#include "ui/albumcoverchoicecontroller.h"

#ifdef HAVE_MOODBAR
#include "moodbar/moodbarrenderer.h"
#endif

const int AppearanceSettingsPage::kMoodbarPreviewWidth = 150;
const int AppearanceSettingsPage::kMoodbarPreviewHeight = 18;

AppearanceSettingsPage::AppearanceSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_AppearanceSettingsPage),
      original_use_a_custom_color_set_(false),
      playlist_view_background_image_type_(PlaylistView::Default),
      initialised_moodbar_previews_(false) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("view-media-visualization", IconLoader::Base));

  connect(ui_->blur_slider, SIGNAL(valueChanged(int)),
          SLOT(BlurLevelChanged(int)));
  connect(ui_->opacity_slider, SIGNAL(valueChanged(int)),
          SLOT(OpacityLevelChanged(int)));

  Load();

  connect(ui_->select_foreground_color, SIGNAL(pressed()),
          SLOT(SelectForegroundColor()));
  connect(ui_->select_background_color, SIGNAL(pressed()),
          SLOT(SelectBackgroundColor()));
  connect(ui_->use_a_custom_color_set, SIGNAL(toggled(bool)),
          SLOT(UseCustomColorSetOptionChanged(bool)));

  connect(ui_->select_background_image_filename_button, SIGNAL(pressed()),
          SLOT(SelectBackgroundImage()));
  connect(ui_->use_custom_background_image, SIGNAL(toggled(bool)),
          ui_->background_image_filename, SLOT(setEnabled(bool)));
  connect(ui_->use_custom_background_image, SIGNAL(toggled(bool)),
          ui_->select_background_image_filename_button, SLOT(setEnabled(bool)));

  connect(ui_->use_custom_background_image, SIGNAL(toggled(bool)),
          ui_->blur_slider, SLOT(setEnabled(bool)));
  connect(ui_->use_album_cover_background, SIGNAL(toggled(bool)),
          ui_->blur_slider, SLOT(setEnabled(bool)));

  connect(ui_->use_default_background, SIGNAL(toggled(bool)),
          SLOT(DisableBlurAndOpacitySliders(bool)));
  connect(ui_->use_no_background, SIGNAL(toggled(bool)),
          SLOT(DisableBlurAndOpacitySliders(bool)));
#if !defined(Q_OS_UNIX) || defined(Q_OS_MAC)
  ui_->b_use_sys_icons->setDisabled(true);
#endif
}

AppearanceSettingsPage::~AppearanceSettingsPage() { delete ui_; }

void AppearanceSettingsPage::Load() {
  QSettings s;
  s.beginGroup(Appearance::kSettingsGroup);

  QPalette p = QApplication::palette();

  // Keep in mind originals colors, in case the user clicks on Cancel, to be
  // able to restore colors
  original_use_a_custom_color_set_ =
      s.value(Appearance::kUseCustomColorSet, false).toBool();

  original_foreground_color_ =
      s.value(Appearance::kForegroundColor, p.color(QPalette::WindowText))
          .value<QColor>();
  current_foreground_color_ = original_foreground_color_;
  original_background_color_ =
      s.value(Appearance::kBackgroundColor, p.color(QPalette::Window))
          .value<QColor>();
  current_background_color_ = original_background_color_;

  InitColorSelectorsColors();
  ui_->b_use_sys_icons->setChecked(s.value("b_use_sys_icons", false).toBool());
  ui_->b_hide_filter_toolbar->setChecked(s.value("b_hide_filter_toolbar",false).toBool());
  s.endGroup();

  // Playlist settings
  s.beginGroup(Playlist::kSettingsGroup);
  playlist_view_background_image_type_ =
      static_cast<PlaylistView::BackgroundImageType>(
          s.value(PlaylistView::kSettingBackgroundImageType).toInt());
  playlist_view_background_image_filename_ =
      s.value(PlaylistView::kSettingBackgroundImageFilename).toString();

  ui_->use_system_color_set->setChecked(!original_use_a_custom_color_set_);
  ui_->use_a_custom_color_set->setChecked(original_use_a_custom_color_set_);

  switch (playlist_view_background_image_type_) {
    case PlaylistView::None:
      ui_->use_no_background->setChecked(true);
      DisableBlurAndOpacitySliders(true);
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
      DisableBlurAndOpacitySliders(true);
  }
  ui_->background_image_filename->setText(
      playlist_view_background_image_filename_);
  ui_->blur_slider->setValue(
      s.value("blur_radius", PlaylistView::kDefaultBlurRadius).toInt());
  ui_->opacity_slider->setValue(
      s.value("opacity_level", PlaylistView::kDefaultOpacityLevel).toInt());

  s.endGroup();

  // Moodbar settings
  s.beginGroup("Moodbar");
  ui_->moodbar_show->setChecked(s.value("show", true).toBool());
  ui_->moodbar_style->setCurrentIndex(s.value("style", 0).toInt());
  ui_->moodbar_calculate->setChecked(!s.value("calculate", true).toBool());
  ui_->moodbar_save->setChecked(
      s.value("save_alongside_originals", false).toBool());
  s.endGroup();

  InitMoodbarPreviews();
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
  s.setValue("b_use_sys_icons", ui_->b_use_sys_icons->isChecked());
  s.setValue("b_hide_filter_toolbar", ui_->b_hide_filter_toolbar->isChecked());
  s.endGroup();

  // Playlist settings
  s.beginGroup(Playlist::kSettingsGroup);
  playlist_view_background_image_filename_ =
      ui_->background_image_filename->text();
  if (ui_->use_no_background->isChecked()) {
    playlist_view_background_image_type_ = PlaylistView::None;
  } else if (ui_->use_album_cover_background->isChecked()) {
    playlist_view_background_image_type_ = PlaylistView::AlbumCover;
  } else if (ui_->use_default_background->isChecked()) {
    playlist_view_background_image_type_ = PlaylistView::Default;
  } else if (ui_->use_custom_background_image->isChecked()) {
    playlist_view_background_image_type_ = PlaylistView::Custom;
    s.setValue(PlaylistView::kSettingBackgroundImageFilename,
               playlist_view_background_image_filename_);
  }
  s.setValue(PlaylistView::kSettingBackgroundImageType,
             playlist_view_background_image_type_);
  s.setValue("blur_radius", ui_->blur_slider->value());
  s.setValue("opacity_level", ui_->opacity_slider->value());
  s.endGroup();

  // Moodbar settings
  s.beginGroup("Moodbar");
  s.setValue("calculate", !ui_->moodbar_calculate->isChecked());
  s.setValue("show", ui_->moodbar_show->isChecked());
  s.setValue("style", ui_->moodbar_style->currentIndex());
  s.setValue("save_alongside_originals", ui_->moodbar_save->isChecked());
  s.endGroup();
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
  if (!color_selected.isValid()) return;

  current_foreground_color_ = color_selected;
  dialog()->appearance()->ChangeForegroundColor(color_selected);

  UpdateColorSelectorColor(ui_->select_foreground_color, color_selected);
}

void AppearanceSettingsPage::SelectBackgroundColor() {
  QColor color_selected = QColorDialog::getColor(current_background_color_);
  if (!color_selected.isValid()) return;

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
  UpdateColorSelectorColor(ui_->select_foreground_color,
                           current_foreground_color_);
  UpdateColorSelectorColor(ui_->select_background_color,
                           current_background_color_);
}

void AppearanceSettingsPage::UpdateColorSelectorColor(QWidget* color_selector,
                                                      const QColor& color) {
  QString css =
      QString("background-color: rgb(%1, %2, %3); color: rgb(255, 255, 255)")
          .arg(color.red())
          .arg(color.green())
          .arg(color.blue());
  color_selector->setStyleSheet(css);
}

void AppearanceSettingsPage::SelectBackgroundImage() {
  QString selected_filename = QFileDialog::getOpenFileName(
      this, tr("Select background image"),
      playlist_view_background_image_filename_,
      tr(AlbumCoverChoiceController::kLoadImageFileFilter) + ";;" +
          tr(AlbumCoverChoiceController::kAllFilesFilter));
  if (selected_filename.isEmpty()) return;
  playlist_view_background_image_filename_ = selected_filename;
  ui_->background_image_filename->setText(
      playlist_view_background_image_filename_);
}

void AppearanceSettingsPage::BlurLevelChanged(int value) {
  ui_->background_blur_radius_label->setText(QString("%1px").arg(value));
}

void AppearanceSettingsPage::OpacityLevelChanged(int percent) {
  ui_->background_opacity_label->setText(QString("%1\%").arg(percent));
}

void AppearanceSettingsPage::InitMoodbarPreviews() {
#ifdef HAVE_MOODBAR
  if (initialised_moodbar_previews_) return;
  initialised_moodbar_previews_ = true;

  const QSize preview_size(kMoodbarPreviewWidth, kMoodbarPreviewHeight);
  ui_->moodbar_style->setIconSize(preview_size);

  // Read the sample data
  QFile file(":sample.mood");
  if (!file.open(QIODevice::ReadOnly)) {
    qLog(Warning) << "Unable to open moodbar sample file";
    return;
  }
  QByteArray data(file.readAll());

  // Render and set each preview
  for (int i = 0; i < MoodbarRenderer::StyleCount; ++i) {
    const MoodbarRenderer::MoodbarStyle style =
        MoodbarRenderer::MoodbarStyle(i);
    const ColorVector colors = MoodbarRenderer::Colors(data, style, palette());

    QPixmap pixmap(preview_size);
    QPainter p(&pixmap);
    MoodbarRenderer::Render(colors, &p, pixmap.rect());
    p.end();

    ui_->moodbar_style->addItem(MoodbarRenderer::StyleName(style));
    ui_->moodbar_style->setItemData(i, pixmap, Qt::DecorationRole);
  }
#else
  ui_->moodbar_group->hide();
#endif
}

void AppearanceSettingsPage::DisableBlurAndOpacitySliders(bool checked) {
  // Blur slider
  ui_->blur_slider->setDisabled(checked);
  ui_->background_blur_radius_label->setDisabled(checked);
  ui_->select_background_blur_label->setDisabled(checked);

  // Opacity slider
  ui_->opacity_slider->setDisabled(checked);
  ui_->background_opacity_label->setDisabled(checked);
  ui_->select_opacity_level_label->setDisabled(checked);
}
