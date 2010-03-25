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

#include "settingsdialog.h"
#include "enginebase.h"
#include "osd.h"
#include "osdpretty.h"

#include <QSettings>
#include <QColorDialog>

#include <QtDebug>

SettingsDialog::SettingsDialog(QWidget* parent)
  : QDialog(parent),
    loading_settings_(false),
    pretty_popup_(new OSDPretty)
{
  ui_.setupUi(this);
  pretty_popup_->SetMode(OSDPretty::Mode_Draggable);
  pretty_popup_->SetMessage(tr("OSD Preview"), tr("Drag to reposition"),
                            QImage(":nocover.png"));

  ui_.notifications_bg_preset->setItemData(0, QColor(OSDPretty::kPresetBlue), Qt::DecorationRole);
  ui_.notifications_bg_preset->setItemData(1, QColor(OSDPretty::kPresetOrange), Qt::DecorationRole);

  // Last.fm
  connect(ui_.lastfm, SIGNAL(ValidationComplete(bool)), SLOT(LastFMValidationComplete(bool)));

  // List box
  connect(ui_.list, SIGNAL(currentTextChanged(QString)), SLOT(CurrentTextChanged(QString)));
  ui_.list->setCurrentRow(0);

  // Notifications
  connect(ui_.notifications_none, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_.notifications_native, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_.notifications_tray, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_.notifications_pretty, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_.notifications_opacity, SIGNAL(valueChanged(int)), SLOT(PrettyOpacityChanged(int)));
  connect(ui_.notifications_bg_preset, SIGNAL(activated(int)), SLOT(PrettyColorPresetChanged(int)));
  connect(ui_.notifications_fg_choose, SIGNAL(clicked()), SLOT(ChooseFgColor()));

  if (!OSD::SupportsNativeNotifications())
    ui_.notifications_native->setEnabled(false);
  if (!OSD::SupportsTrayPopups())
    ui_.notifications_tray->setEnabled(false);

  connect(ui_.stacked_widget, SIGNAL(currentChanged(int)), SLOT(UpdatePopupVisible()));
  connect(ui_.notifications_pretty, SIGNAL(toggled(bool)), SLOT(UpdatePopupVisible()));
}

SettingsDialog::~SettingsDialog() {
  delete pretty_popup_;
}

void SettingsDialog::CurrentTextChanged(const QString &text) {
  ui_.title->setText("<b>" + text + "</b>");
}

void SettingsDialog::SetLibraryDirectoryModel(LibraryDirectoryModel* model) {
  ui_.library_config->SetModel(model);
}

void SettingsDialog::LastFMValidationComplete(bool success) {
  ui_.buttonBox->setEnabled(true);

  if (success)
    accept();
}

void SettingsDialog::accept() {
  if (ui_.lastfm->NeedsValidation()) {
    ui_.lastfm->Validate();
    ui_.buttonBox->setEnabled(false);
    return;
  } else {
    ui_.lastfm->Save();
  }

  QSettings s;

  // Playback
  s.beginGroup(Engine::Base::kSettingsGroup);
  s.setValue("FadeoutEnabled", ui_.fadeout->isChecked());
  s.setValue("FadeoutDuration", ui_.fadeout_duration->value());
  s.endGroup();

  // Notifications
  OSD::Behaviour osd_behaviour;
  if      (ui_.notifications_none->isChecked())   osd_behaviour = OSD::Disabled;
  else if (ui_.notifications_native->isChecked()) osd_behaviour = OSD::Native;
  else if (ui_.notifications_tray->isChecked())   osd_behaviour = OSD::TrayPopup;
  else if (ui_.notifications_pretty->isChecked()) osd_behaviour = OSD::Pretty;

  s.beginGroup(OSD::kSettingsGroup);
  s.setValue("Behaviour", int(osd_behaviour));
  s.setValue("Timeout", ui_.notifications_duration->value() * 1000);
  s.setValue("ShowOnVolumeChange", ui_.notifications_volume->isChecked());
  s.setValue("ShowArt", ui_.notifications_art->isChecked());
  s.endGroup();

  s.beginGroup(OSDPretty::kSettingsGroup);
  s.setValue("foreground_color", pretty_popup_->foreground_color());
  s.setValue("background_color", pretty_popup_->background_color());
  s.setValue("background_opacity", pretty_popup_->background_opacity());
  s.setValue("popup_display", pretty_popup_->current_display());
  s.setValue("popup_pos", pretty_popup_->current_pos());
  s.endGroup();

  QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent*) {
  QSettings s;
  loading_settings_ = true;

  // Last.fm
  ui_.lastfm->Load();

  // Playback
  s.beginGroup(Engine::Base::kSettingsGroup);
  if (s.value("FadeoutEnabled", true).toBool())
    ui_.fadeout->setChecked(true);
  else
    ui_.no_fadeout->setChecked(true);
  ui_.fadeout_duration->setValue(s.value("FadeoutDuration", 2000).toInt());
  s.endGroup();

  // Notifications
  s.beginGroup(OSD::kSettingsGroup);
  OSD::Behaviour osd_behaviour = OSD::Behaviour(s.value("Behaviour", OSD::Native).toInt());
  switch (osd_behaviour) {
    case OSD::Native:
      if (OSD::SupportsNativeNotifications()) {
        ui_.notifications_native->setChecked(true);
        break;
      }
      // Fallthrough

    case OSD::TrayPopup:
      if (OSD::SupportsTrayPopups()) {
        ui_.notifications_tray->setChecked(true);
        break;
      }
      // Fallthrough

    case OSD::Pretty:
      ui_.notifications_pretty->setChecked(true);
      break;

    case OSD::Disabled:
    default:
      ui_.notifications_none->setChecked(true);
      break;
  }
  ui_.notifications_duration->setValue(s.value("Timeout", 5000).toInt() / 1000);
  ui_.notifications_volume->setChecked(s.value("ShowOnVolumeChange", false).toBool());
  ui_.notifications_art->setChecked(s.value("ShowArt", true).toBool());
  s.endGroup();

  // Pretty OSD
  pretty_popup_->ReloadSettings();
  ui_.notifications_opacity->setValue(pretty_popup_->background_opacity() * 100);

  QRgb color = pretty_popup_->background_color();
  if (color == OSDPretty::kPresetBlue)
    ui_.notifications_bg_preset->setCurrentIndex(0);
  else if (color == OSDPretty::kPresetOrange)
    ui_.notifications_bg_preset->setCurrentIndex(1);
  else
    ui_.notifications_bg_preset->setCurrentIndex(2);
  ui_.notifications_bg_preset->setItemData(2, QColor(color), Qt::DecorationRole);
  UpdatePopupVisible();

  loading_settings_ = false;
}

void SettingsDialog::hideEvent(QHideEvent *) {
  pretty_popup_->hide();
}

void SettingsDialog::NotificationTypeChanged() {
  bool enabled = !ui_.notifications_none->isChecked();
  bool pretty = ui_.notifications_pretty->isChecked();

  ui_.notifications_general->setEnabled(enabled);
  ui_.notifications_pretty_group->setEnabled(pretty);
}

void SettingsDialog::PrettyOpacityChanged(int value) {
  pretty_popup_->set_background_opacity(qreal(value) / 100.0);
}

void SettingsDialog::UpdatePopupVisible() {
  pretty_popup_->setVisible(
      isVisible() &&
      ui_.notifications_pretty->isChecked() &&
      ui_.stacked_widget->currentWidget() == ui_.notifications_page);
}

void SettingsDialog::PrettyColorPresetChanged(int index) {
  if (loading_settings_)
    return;

  switch (index) {
  case 0:
    pretty_popup_->set_background_color(OSDPretty::kPresetBlue);
    break;

  case 1:
    pretty_popup_->set_background_color(OSDPretty::kPresetOrange);
    break;

  case 2:
  default:
    ChooseBgColor();
    break;
  }
}

void SettingsDialog::ChooseBgColor() {
  QColor color = QColorDialog::getColor(pretty_popup_->background_color(), this);
  if (!color.isValid())
    return;

  pretty_popup_->set_background_color(color.rgb());
  ui_.notifications_bg_preset->setItemData(2, color, Qt::DecorationRole);
}

void SettingsDialog::ChooseFgColor() {
  QColor color = QColorDialog::getColor(pretty_popup_->foreground_color(), this);
  if (!color.isValid())
    return;

  pretty_popup_->set_foreground_color(color.rgb());
}
