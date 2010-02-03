#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "ui_settingsdialog.h"

class LibraryDirectoryModel;

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  SettingsDialog(QWidget* parent = 0);

  void SetLibraryDirectoryModel(LibraryDirectoryModel* model);

  // QDialog
  void accept();

  // QWidget
  void showEvent(QShowEvent* e);

 private slots:
  void CurrentTextChanged(const QString& text);
  void NotificationTypeChanged();
  void LastFMValidationComplete(bool success);

 private:
  Ui::SettingsDialog ui_;
};

#endif // SETTINGSDIALOG_H
