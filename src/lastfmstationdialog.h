#ifndef LASTFMSTATIONDIALOG_H
#define LASTFMSTATIONDIALOG_H

#include <QDialog>

#include "ui_lastfmstationdialog.h"

class LastFMStationDialog : public QDialog {
  Q_OBJECT

 public:
  LastFMStationDialog(QWidget* parent = 0);

  enum Type {
    Artist,
    Tag,
  };

  void SetType(Type type);
  QString content() const;

 private:
  Ui::LastFMStationDialog ui_;
};

#endif // LASTFMSTATIONDIALOG_H
