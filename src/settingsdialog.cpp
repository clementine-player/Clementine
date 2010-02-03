#include "settingsdialog.h"
#include "enginebase.h"
#include "osd.h"

#include <QSettings>

SettingsDialog::SettingsDialog(QWidget* parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  // List box
  connect(ui_.list, SIGNAL(currentTextChanged(QString)), SLOT(CurrentTextChanged(QString)));
  ui_.list->setCurrentRow(0);

  // Notifications
  connect(ui_.notifications_none, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_.notifications_native, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));
  connect(ui_.notifications_tray, SIGNAL(toggled(bool)), SLOT(NotificationTypeChanged()));

#ifdef Q_WS_WIN
  // Sucks to be a windows user
  ui_.notifications_native->setEnabled(false);
#endif
}

void SettingsDialog::CurrentTextChanged(const QString &text) {
  ui_.title->setText("<b>" + text + "</b>");
}

void SettingsDialog::SetLibraryDirectoryModel(LibraryDirectoryModel* model) {
  ui_.library_config->SetModel(model);
}

void SettingsDialog::accept() {
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
  s.endGroup();

  QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent*) {
  QSettings s;

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
    case OSD::Native:    ui_.notifications_native->setChecked(true); break;
    case OSD::TrayPopup: ui_.notifications_tray->setChecked(true); break;
    case OSD::Disabled:
    default:             ui_.notifications_none->setChecked(true); break;
  }
  ui_.notifications_duration->setValue(s.value("Timeout", 5000).toInt() / 1000);
  s.endGroup();
}

void SettingsDialog::NotificationTypeChanged() {
  ui_.notifications_options->setEnabled(!ui_.notifications_none->isChecked());
}
