#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "ui_mainwindow.h"

class Playlist;
class Player;
class Library;
class LibraryConfig;

class QFileSystemModel;
class QSortFilterProxyModel;
class QUndoStack;
class SystemTrayIcon;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void SetHiddenInTray(bool hidden);

 protected:
  void resizeEvent(QResizeEvent* event);
  void closeEvent(QCloseEvent* event);

 private slots:
  void FileUp();
  void FileBack();
  void FileForward();
  void FileHome();
  void ChangeFilePath(const QString& new_path);
  void FileClicked(const QModelIndex& index);
  void FileDoubleClicked(const QModelIndex& index);

  void ReportError(const QString& message);
  void MediaStopped();
  void MediaPaused();
  void MediaPlaying();

  void PlayIndex(const QModelIndex& index);
  void StopAfterCurrent();

  void LibraryDoubleClick(const QModelIndex& index);
  void ClearLibraryFilter();

  void VolumeWheelEvent(int delta);
  void TrayClicked(QSystemTrayIcon::ActivationReason reason);

 private:
  void ChangeFilePathWithoutUndo(const QString& new_path);
  void SaveGeometry();

 private:
  static const int kStateVersion;
  static const char* kSettingsGroup;

  Ui::MainWindow ui_;

  Playlist* playlist_;
  Player* player_;
  Library* library_;

  QSortFilterProxyModel* library_sort_model_;

  QFileSystemModel* file_model_;
  QUndoStack* file_undo_stack_;

  SystemTrayIcon* tray_icon_;
};

#endif // MAINWINDOW_H
