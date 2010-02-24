#ifndef ADDSTREAMDIALOG_H
#define ADDSTREAMDIALOG_H

#include <QDialog>

#include "ui_addstreamdialog.h"

class SavedRadio;

class AddStreamDialog : public QDialog {
  Q_OBJECT

 public:
  AddStreamDialog(QWidget *parent = 0);

  QUrl url() const;

  void accept();

 private slots:
  void TextChanged(const QString& text);

 private:
  static const char* kSettingsGroup;

  Ui::AddStreamDialog ui_;

  SavedRadio* saved_radio_;
};

#endif // ADDSTREAMDIALOG_H
