#ifndef LIBRARYCONFIGDIALOG_H
#define LIBRARYCONFIGDIALOG_H

#include <QDialog>

#include "ui_libraryconfigdialog.h"

class LibraryDirectoryModel;

class LibraryConfigDialog : public QDialog {
  Q_OBJECT
 public:
  LibraryConfigDialog(QWidget* parent = 0);

  void SetModel(LibraryDirectoryModel* model);

 private:
  Ui::LibraryConfigDialog ui_;
};

#endif // LIBRARYCONFIGDIALOG_H
