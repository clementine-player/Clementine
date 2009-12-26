#ifndef LASTFMCONFIG_H
#define LASTFMCONFIG_H

#include <QDialog>

#include "ui_lastfmconfig.h"

class LastFMService;

class LastFMConfig : public QDialog {
  Q_OBJECT

 public:
  LastFMConfig(LastFMService* service, QWidget* parent = 0);

  void accept();

 private slots:
  void AuthenticationComplete(bool success);

 private:
  Ui::LastFMConfig ui_;
  LastFMService* service_;
};

#endif // LASTFMCONFIG_H
