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

  Ui::LastFMConfig ui_;

 signals:
  void ScrobblingEnabledChanged(bool value);

 private slots:
  void AuthenticationComplete(bool success);

 private:
  LastFMService* service_;
};

#endif // LASTFMCONFIG_H
