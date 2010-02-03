#include "settingsdialog.h"
#include "ui_settingsdialog.h"

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
