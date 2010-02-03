#include "lastfmconfigdialog.h"
#include "ui_lastfmconfigdialog.h"

LastFMConfigDialog::LastFMConfigDialog(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  connect(ui_.lastfm, SIGNAL(ValidationComplete(bool)), SLOT(ValidationComplete(bool)));
}

void LastFMConfigDialog::showEvent(QShowEvent *) {
  ui_.lastfm->Load();
}

void LastFMConfigDialog::accept() {
  if (ui_.lastfm->NeedsValidation()) {
    ui_.lastfm->Validate();
    ui_.buttonBox->setEnabled(false);
  } else {
    ui_.lastfm->Save();
    QDialog::accept();
  }
}

void LastFMConfigDialog::ValidationComplete(bool success) {
  ui_.buttonBox->setEnabled(true);

  if (success)
    QDialog::accept();
}
