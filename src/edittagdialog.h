#ifndef EDITTAGDIALOG_H
#define EDITTAGDIALOG_H

#include <QDialog>

#include "ui_edittagdialog.h"
#include "song.h"

class EditTagDialog : public QDialog {
  Q_OBJECT

 public:
  EditTagDialog(QWidget* parent = 0);

  bool SetSongs(const SongList& songs);

 public slots:
  void accept();

 signals:
  void SongEdited(const Song& old_song, const Song& new_song);

 private:
  Ui::EditTagDialog ui_;

  SongList songs_;
};

#endif // EDITTAGDIALOG_H
