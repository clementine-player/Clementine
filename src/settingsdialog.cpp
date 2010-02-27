#include "settingsdialog.h"
#include "enginebase.h"
#include "osd.h"

#include <QSettings>

SettingsDialog::SettingsDialog(QWidget* parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  // Last.fm
  connect(ui_.lastfm, SIGNAL(ValidationComplete(bool)), SLOT(LastFMValidationComplete(bool)));

  // List box
  connect(ui_.list, SIGNAL(currentTextChanged(QString)), SLOT(CurrentTextChanged(QString)));
  ui_.list->setCurrentRow(0);

  // Notifications
  connect(ui_.notifications_none, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_.notifications_native, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_.notifications_tray, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));

  if (!OSD::SupportsNativeNotifications())
    ui_.notifications_native->setEnabled(false);
  if (!OSD::SupportsTrayPopups())
    ui_.notifications_tray->setEnabled(false);
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

  s.beginGroup(OSD::kSettingsGroup);
  s.setValue("Behaviour", int(osd_behaviour));
  s.setValue("Timeout", ui_.notifications_duration->value() * 1000);
  s.setValue("ShowOnVolumeChange", ui_.notifications_volume->isChecked());
  s.endGroup();

  QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent*) {
  QSettings s;

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

    case OSD::Disabled:
    default:
      ui_.notifications_none->setChecked(true);
      break;
  }
  ui_.notifications_duration->setValue(s.value("Timeout", 5000).toInt() / 1000);
  ui_.notifications_volume->setChecked(s.value("ShowOnVolumeChange", false).toBool());
  s.endGroup();
}

void SettingsDialog::NotificationTypeChanged() {
  bool enabled = !ui_.notifications_none->isChecked();
  ui_.notifications_options->setEnabled(enabled);
  ui_.notifications_volume->setEnabled(enabled);
}
