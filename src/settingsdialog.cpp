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
#include "engines/enginebase.h"
#include "osd.h"
#include "osdpretty.h"
#include "mainwindow.h"
#ifdef HAVE_GSTREAMER
# include "engines/gstengine.h"
#endif

#include <QSettings>
#include <QColorDialog>

#include <QtDebug>

SettingsDialog::SettingsDialog(QWidget* parent)
  : QDialog(parent),
    loading_settings_(false),
    pretty_popup_(new OSDPretty(OSDPretty::Mode_Draggable))
{
  ui_.setupUi(this);
  pretty_popup_->SetMessage(tr("OSD Preview"), tr("Drag to reposition"),
                            QImage(":nocover.png"));

  // Playback
  connect(ui_.fading_cross, SIGNAL(toggled(bool)), SLOT(FadingOptionsChanged()));
  connect(ui_.fading_out, SIGNAL(toggled(bool)), SLOT(FadingOptionsChanged()));
  connect(ui_.fading_auto, SIGNAL(toggled(bool)), SLOT(FadingOptionsChanged()));
#ifdef HAVE_GSTREAMER
  connect(ui_.gst_plugin, SIGNAL(currentIndexChanged(int)), SLOT(GstPluginChanged(int)));
#endif

  // Behaviour
  connect(ui_.b_show_tray_icon_, SIGNAL(toggled(bool)), SLOT(ShowTrayIconToggled(bool)));

  // Last.fm
  connect(ui_.lastfm, SIGNAL(ValidationComplete(bool)), SLOT(LastFMValidationComplete(bool)));

  // List box
  connect(ui_.list, SIGNAL(currentTextChanged(QString)), SLOT(CurrentTextChanged(QString)));
  ui_.list->setCurrentRow(0);

  // Notifications
  ui_.notifications_bg_preset->setItemData(0, QColor(OSDPretty::kPresetBlue), Qt::DecorationRole);
  ui_.notifications_bg_preset->setItemData(1, QColor(OSDPretty::kPresetOrange), Qt::DecorationRole);

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

  // Behaviour
  MainWindow::StartupBehaviour behaviour;
  if (ui_.b_always_hide_->isChecked()) behaviour = MainWindow::Startup_AlwaysHide;
  if (ui_.b_always_show_->isChecked()) behaviour = MainWindow::Startup_AlwaysShow;
  if (ui_.b_remember_->isChecked())    behaviour = MainWindow::Startup_Remember;

  s.beginGroup(MainWindow::kSettingsGroup);
  s.setValue("showtray", ui_.b_show_tray_icon_->isChecked());
  s.setValue("startupbehaviour", int(behaviour));
  s.endGroup();

  // Playback
  s.beginGroup(Engine::Base::kSettingsGroup);
  s.setValue("FadeoutEnabled", ui_.fading_out->isChecked());
  s.setValue("FadeoutDuration", ui_.fading_duration->value());
  s.setValue("CrossfadeEnabled", ui_.fading_cross->isChecked());
  s.setValue("AutoCrossfadeEnabled", ui_.fading_auto->isChecked());
  s.endGroup();

#ifdef HAVE_GSTREAMER
  s.beginGroup(GstEngine::kSettingsGroup);
  s.setValue("sink", ui_.gst_plugin->itemData(ui_.gst_plugin->currentIndex()).toString());
  s.setValue("device", ui_.gst_device->text());
  s.endGroup();
#endif

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
  s.setValue("popup_display", pretty_popup_->popup_display());
  s.setValue("popup_pos", pretty_popup_->popup_pos());
  s.endGroup();

  ui_.library_config->Save();

  QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent*) {
  QSettings s;
  loading_settings_ = true;

  // Behaviour
  s.beginGroup(MainWindow::kSettingsGroup);
  ui_.b_show_tray_icon_->setChecked(s.value("showtray", true).toBool());
  MainWindow::StartupBehaviour behaviour = MainWindow::StartupBehaviour(
      s.value("startupbehaviour", MainWindow::Startup_Remember).toInt());
  switch (behaviour) {
    case MainWindow::Startup_AlwaysHide: ui_.b_always_hide_->setChecked(true); break;
    case MainWindow::Startup_AlwaysShow: ui_.b_always_show_->setChecked(true); break;
    case MainWindow::Startup_Remember:   ui_.b_remember_->setChecked(true);    break;
  }
  s.endGroup();

  // Last.fm
  ui_.lastfm->Load();

  // Playback
  s.beginGroup(Engine::Base::kSettingsGroup);
  ui_.fading_out->setChecked(s.value("FadeoutEnabled", true).toBool());
  ui_.fading_cross->setChecked(s.value("CrossfadeEnabled", true).toBool());
  ui_.fading_auto->setChecked(s.value("AutoCrossfadeEnabled", false).toBool());
  ui_.fading_duration->setValue(s.value("FadeoutDuration", 2000).toInt());
  s.endGroup();

#ifdef HAVE_GSTREAMER
  s.beginGroup(GstEngine::kSettingsGroup);
  QString sink = s.value("sink", GstEngine::kAutoSink).toString();
  ui_.gst_plugin->setCurrentIndex(0);
  for (int i=0 ; i<ui_.gst_plugin->count() ; ++i) {
    if (ui_.gst_plugin->itemData(i).toString() == sink) {
      ui_.gst_plugin->setCurrentIndex(i);
      break;
    }
  }
  ui_.gst_device->setText(s.value("device").toString());
  s.endGroup();
#endif

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

    case OSD::Pretty:
      ui_.notifications_pretty->setChecked(true);
      break;

    case OSD::TrayPopup:
      if (OSD::SupportsTrayPopups()) {
        ui_.notifications_tray->setChecked(true);
        break;
      }
      // Fallthrough

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

void SettingsDialog::ShowTrayIconToggled(bool on) {
  ui_.b_always_hide_->setEnabled(on);
  if (!on && ui_.b_always_hide_->isChecked())
    ui_.b_remember_->setChecked(true);
}

#ifdef HAVE_GSTREAMER
void SettingsDialog::SetGstEngine(const GstEngine *engine) {
  GstEngine::PluginDetailsList list = engine->GetOutputsList();

  ui_.gst_plugin->setItemData(0, GstEngine::kAutoSink);
  foreach (const GstEngine::PluginDetails& details, list) {
    if (details.name == "autoaudiosink")
      continue;

    ui_.gst_plugin->addItem(details.long_name, details.name);
  }
  ui_.gst_group->setEnabled(true);
}

void SettingsDialog::GstPluginChanged(int index) {
  QString name = ui_.gst_plugin->itemData(index).toString();

  bool enabled = GstEngine::DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(name);

  ui_.gst_device->setEnabled(enabled);
  ui_.gst_device_label->setEnabled(enabled);
}
#endif

void SettingsDialog::FadingOptionsChanged() {
  ui_.fading_options->setEnabled(
      ui_.fading_out->isChecked() || ui_.fading_cross->isChecked() ||
      ui_.fading_auto->isChecked());
}
