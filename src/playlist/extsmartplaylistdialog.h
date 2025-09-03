#ifndef EXTSMARTPLAYLISTDIALOG_H
#define EXTSMARTPLAYLISTDIALOG_H

#include <QDialog>
#include <QObject>

class PlaylistManager;
class QLineEdit;
class QTableWidget;

class ExtSmartPlaylistDialog : public QDialog {
  Q_OBJECT

public:
  explicit ExtSmartPlaylistDialog(PlaylistManager* playlist_manager, QWidget* parent = nullptr);
  ~ExtSmartPlaylistDialog();

private slots:
  void addRule();
  void removeRule();

private:
  void setupUi();

  PlaylistManager* playlist_manager_;
  QLineEdit* name_edit_;
  QTableWidget* rules_table_;
};

#endif // EXTSMARTPLAYLISTDIALOG_H
