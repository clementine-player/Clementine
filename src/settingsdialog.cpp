#include "settingsdialog.h"
#include "enginebase.h"

#include <QSettings>

SettingsDialog::SettingsDialog(QWidget* parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  connect(ui_.list, SIGNAL(currentTextChanged(QString)), SLOT(CurrentTextChanged(QString)));
  ui_.list->setCurrentRow(0);
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
}
