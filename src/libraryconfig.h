#ifndef LIBRARYCONFIG_H
#define LIBRARYCONFIG_H

#include <QWidget>

#include "ui_libraryconfig.h"

class LibraryDirectoryModel;

class LibraryConfig : public QWidget {
  Q_OBJECT

 public:
  LibraryConfig(QWidget* parent = 0);

  void SetModel(LibraryDirectoryModel* model);

 private slots:
  void Add();
  void Remove();
  void BackendReady();

  void CurrentRowChanged(const QModelIndex& index);

 private:
  static const char* kSettingsGroup;

  Ui::LibraryConfig ui_;
  LibraryDirectoryModel* model_;
};

#endif // LIBRARYCONFIG_H
