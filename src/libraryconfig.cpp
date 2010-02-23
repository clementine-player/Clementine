#include "libraryconfig.h"
#include "librarydirectorymodel.h"

#include <QFileDialog>
#include <QSettings>
#include <QDir>

const char* LibraryConfig::kSettingsGroup = "LibraryConfig";

LibraryConfig::LibraryConfig(QWidget* parent)
  : QWidget(parent),
    model_(NULL)
{
  ui_.setupUi(this);

  connect(ui_.add, SIGNAL(clicked()), SLOT(Add()));
  connect(ui_.remove, SIGNAL(clicked()), SLOT(Remove()));
}

void LibraryConfig::SetModel(LibraryDirectoryModel *model) {
  model_ = model;
  ui_.list->setModel(model_);

  connect(ui_.list->selectionModel(),
          SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
          SLOT(CurrentRowChanged(QModelIndex)));


  if (model_->IsBackendReady())
    BackendReady();
  else
    connect(model_, SIGNAL(BackendReady()), SLOT(BackendReady()));
}

void LibraryConfig::Add() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  QString path(settings.value("last_path", QDir::homePath()).toString());
  path = QFileDialog::getExistingDirectory(this, tr("Add directory..."), path);

  if (!path.isNull()) {
    model_->AddDirectory(path);
  }

  settings.setValue("last_path", path);
}

void LibraryConfig::Remove() {
  model_->RemoveDirectory(ui_.list->currentIndex());
}

void LibraryConfig::CurrentRowChanged(const QModelIndex& index) {
  ui_.remove->setEnabled(index.isValid());
}

void LibraryConfig::BackendReady() {
  ui_.list->setEnabled(true);
  ui_.add->setEnabled(true);
  ui_.remove->setEnabled(true);
}
