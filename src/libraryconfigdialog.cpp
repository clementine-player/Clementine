#include "libraryconfigdialog.h"

LibraryConfigDialog::LibraryConfigDialog(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
}

void LibraryConfigDialog::SetModel(LibraryDirectoryModel* model) {
  ui_.config->SetModel(model);
}
