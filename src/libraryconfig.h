#ifndef LIBRARYCONFIG_H
#define LIBRARYCONFIG_H

#include <QDialog>

#include <boost/shared_ptr.hpp>

#include "ui_libraryconfig.h"
#include "directory.h"

class LibraryBackend;

class LibraryConfig : public QDialog {
  Q_OBJECT

 public:
  LibraryConfig(QWidget* parent = 0);

  void SetBackend(boost::shared_ptr<LibraryBackend> backend);

 private slots:
  void Add();
  void Remove();

  void DirectoriesDiscovered(const DirectoryList&);
  void DirectoriesDeleted(const DirectoryList&);

  void CurrentRowChanged(int);

 private:
  static const char* kSettingsGroup;

  Ui::LibraryConfig ui_;
  QIcon dir_icon_;

  boost::shared_ptr<LibraryBackend> backend_;
};

#endif // LIBRARYCONFIG_H
