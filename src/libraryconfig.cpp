#include "libraryconfig.h"
#include "librarybackend.h"

#include <QFileDialog>
#include <QSettings>
#include <QDir>

const char* LibraryConfig::kSettingsGroup = "LibraryConfig";

LibraryConfig::LibraryConfig(QWidget* parent)
  : QDialog(parent),
    dir_icon_(":folder.png")
{
  ui_.setupUi(this);

  connect(ui_.add, SIGNAL(clicked()), SLOT(Add()));
  connect(ui_.remove, SIGNAL(clicked()), SLOT(Remove()));
  connect(ui_.list, SIGNAL(currentRowChanged(int)), SLOT(CurrentRowChanged(int)));
}

void LibraryConfig::SetBackend(boost::shared_ptr<LibraryBackend> backend) {
  backend_ = backend;

  connect(backend_.get(), SIGNAL(DirectoriesDiscovered(DirectoryList)), SLOT(DirectoriesDiscovered(DirectoryList)));
  connect(backend_.get(), SIGNAL(DirectoriesDeleted(DirectoryList)), SLOT(DirectoriesDeleted(DirectoryList)));

  ui_.list->setEnabled(true);
  ui_.add->setEnabled(true);
}

void LibraryConfig::Add() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  QString path(settings.value("last_path", QDir::homePath()).toString());
  path = QFileDialog::getExistingDirectory(this, "Add directory...", path);

  if (!path.isNull()) {
    backend_->AddDirectory(path);
  }

  settings.setValue("last_path", path);
}

void LibraryConfig::Remove() {
  QListWidgetItem* item = ui_.list->currentItem();
  if (item == NULL)
    return;

  Directory dir;
  dir.path = item->text();
  dir.id = item->type();

  backend_->RemoveDirectory(dir);
}

void LibraryConfig::DirectoriesDiscovered(const DirectoryList& list) {
  foreach (const Directory& dir, list) {
    new QListWidgetItem(dir_icon_, dir.path, ui_.list, dir.id);
  }
}

void LibraryConfig::DirectoriesDeleted(const DirectoryList& list) {
  foreach (const Directory& dir, list) {
    for (int i=0 ; i<ui_.list->count() ; ++i) {
      if (ui_.list->item(i)->type() == dir.id) {
        delete ui_.list->takeItem(i);
        break;
      }
    }
  }
}

void LibraryConfig::CurrentRowChanged(int row) {
  ui_.remove->setEnabled(row != -1);
}
