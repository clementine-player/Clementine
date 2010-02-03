#ifndef LASTFMCONFIGDIALOG_H
#define LASTFMCONFIGDIALOG_H

#include <QDialog>

#include "ui_lastfmconfigdialog.h"

class LastFMConfigDialog : public QDialog {
  Q_OBJECT
 public:
  LastFMConfigDialog(QWidget* parent = 0);

  void accept();
  void showEvent(QShowEvent *);

 private slots:
  void ValidationComplete(bool success);

 private:
  Ui::LastFMConfigDialog ui_;
};

#endif // LASTFMCONFIGDIALOG_H
