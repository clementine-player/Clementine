#ifndef LASTFMCONFIG_H
#define LASTFMCONFIG_H

#include <QWidget>

#include "ui_lastfmconfig.h"

class LastFMService;

class LastFMConfig : public QWidget {
  Q_OBJECT

 public:
  LastFMConfig(QWidget* parent = 0);

  bool NeedsValidation() const;

 public slots:
  void Validate();
  void Load();
  void Save();

 signals:
  void ValidationComplete(bool success);

 private slots:
  void AuthenticationComplete(bool success);

 private:
  LastFMService* service_;
  Ui::LastFMConfig ui_;
};

#endif // LASTFMCONFIG_H
