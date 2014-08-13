#ifndef PLAYLISTSAVEOPTIONSDIALOG_H
#define PLAYLISTSAVEOPTIONSDIALOG_H

#include <QDialog>

struct PlaylistSaveOptions;

namespace Ui {
class PlaylistSaveOptionsDialog;
}

class PlaylistSaveOptionsDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit PlaylistSaveOptionsDialog(QWidget *parent = 0);
  ~PlaylistSaveOptionsDialog();

  void accept();
  PlaylistSaveOptions options() const;

 private:
  static const char* kSettingsGroup;

  Ui::PlaylistSaveOptionsDialog *ui;
};

#endif // PLAYLISTSAVEOPTIONSDIALOG_H
