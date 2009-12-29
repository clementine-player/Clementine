#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "ui_mainwindow.h"

class Playlist;
class Player;
class Library;
class LibraryConfig;
class RadioModel;
class Song;

class QSortFilterProxyModel;
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
  void QueueFiles(const QList<QUrl>& urls);
  void FilePathChanged(const QString& path);

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

  void UpdateTrackPosition();

  void RadioDoubleClick(const QModelIndex& index);
  void ScrobblingEnabledChanged(bool value);
  void Love();

 private:
  void SaveGeometry();

 private:
  static const int kStateVersion;
  static const char* kSettingsGroup;

  Ui::MainWindow ui_;
  SystemTrayIcon* tray_icon_;

  RadioModel* radio_model_;
  Playlist* playlist_;
  Player* player_;
  Library* library_;

  QSortFilterProxyModel* library_sort_model_;

  QTimer* track_position_timer_;
};

#endif // MAINWINDOW_H
