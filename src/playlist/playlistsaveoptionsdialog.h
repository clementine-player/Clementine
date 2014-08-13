#ifndef PLAYLISTSAVEOPTIONSDIALOG_H
#define PLAYLISTSAVEOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class PlaylistSaveOptionsDialog;
}

class PlaylistSaveOptionsDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit PlaylistSaveOptionsDialog(QWidget *parent = 0);
  ~PlaylistSaveOptionsDialog();

 private:
  Ui::PlaylistSaveOptionsDialog *ui;
};

#endif // PLAYLISTSAVEOPTIONSDIALOG_H
