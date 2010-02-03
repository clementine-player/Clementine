#ifndef LIBRARYDIRECTORYMODEL_H
#define LIBRARYDIRECTORYMODEL_H

#include <QStandardItemModel>
#include <QIcon>

#include <boost/shared_ptr.hpp>

#include "directory.h"

class LibraryBackend;

class LibraryDirectoryModel : public QStandardItemModel {
  Q_OBJECT

 public:
  LibraryDirectoryModel(QObject* parent = 0);

  void SetBackend(boost::shared_ptr<LibraryBackend> backend);
  bool IsBackendReady() const { return backend_; }

  // To be called by GUIs
  void AddDirectory(const QString& path);
  void RemoveDirectory(const QModelIndex& index);

 signals:
  void BackendReady();

 private slots:
  // To be called by the backend
  void DirectoriesDiscovered(const DirectoryList& directories);
  void DirectoriesDeleted(const DirectoryList& directories);

 private:
  static const int kIdRole = Qt::UserRole + 1;

  QIcon dir_icon_;
  boost::shared_ptr<LibraryBackend> backend_;
};

#endif // LIBRARYDIRECTORYMODEL_H
