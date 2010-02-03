#include "librarydirectorymodel.h"
#include "librarybackend.h"

LibraryDirectoryModel::LibraryDirectoryModel(QObject* parent)
  : QStandardItemModel(parent),
    dir_icon_(":folder.png")
{
}

void LibraryDirectoryModel::SetBackend(boost::shared_ptr<LibraryBackend> backend) {
  backend_ = backend;

  connect(backend_.get(), SIGNAL(DirectoriesDiscovered(DirectoryList)), SLOT(DirectoriesDiscovered(DirectoryList)));
  connect(backend_.get(), SIGNAL(DirectoriesDeleted(DirectoryList)), SLOT(DirectoriesDeleted(DirectoryList)));

  emit BackendReady();
}

void LibraryDirectoryModel::DirectoriesDiscovered(const DirectoryList &directories) {
  foreach (const Directory& dir, directories) {
    QStandardItem* item = new QStandardItem(dir.path);
    item->setData(dir.id, kIdRole);
    item->setIcon(dir_icon_);
    appendRow(item);
  }
}

void LibraryDirectoryModel::DirectoriesDeleted(const DirectoryList &directories) {
  foreach (const Directory& dir, directories) {
    for (int i=0 ; i<rowCount() ; ++i) {
      if (item(i, 0)->data(kIdRole).toInt() == dir.id) {
        removeRow(i);
        break;
      }
    }
  }
}

void LibraryDirectoryModel::AddDirectory(const QString& path) {
  if (!backend_)
    return;

  backend_->AddDirectory(path);
}

void LibraryDirectoryModel::RemoveDirectory(const QModelIndex& index) {
  if (!backend_ || !index.isValid())
    return;

  Directory dir;
  dir.path = index.data().toString();
  dir.id = index.data(kIdRole).toInt();

  backend_->RemoveDirectory(dir);
}
