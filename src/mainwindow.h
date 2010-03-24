/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>

#include "ui_mainwindow.h"

class Playlist;
class Player;
class Library;
class LibraryConfigDialog;
class RadioModel;
class Song;
class RadioItem;
class OSD;
class TrackSlider;
class EditTagDialog;
class MultiLoadingIndicator;
class SettingsDialog;
class About;
class AddStreamDialog;
class AlbumCoverManager;
class PlaylistSequence;
class GlobalShortcuts;

class QSortFilterProxyModel;
class SystemTrayIcon;

class QNetworkAccessManager;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QNetworkAccessManager* network, QWidget *parent = 0);
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

  void PlaylistRightClick(const QPoint& global_pos, const QModelIndex& index);
  void PlaylistPlay();
  void PlaylistStopAfter();
  void PlaylistRemoveCurrent();
  void EditTracks();
  void RenumberTracks();
  void SelectionSetValue();

  void PlayIndex(const QModelIndex& index);
  void StopAfterCurrent();

  void LibraryDoubleClick(const QModelIndex& index);
  void ClearLibraryFilter();

  void VolumeWheelEvent(int delta);
  void TrayClicked(QSystemTrayIcon::ActivationReason reason);

  void UpdateTrackPosition();

  void RadioDoubleClick(const QModelIndex& index);
  void InsertRadioItem(RadioItem*);
  void ScrobblingEnabledChanged(bool value);
  void Love();

  void LibraryScanStarted();
  void LibraryScanFinished();

  void PlayerInitFinished();

  void HideShowTrayIcon();

  void AddMedia();
  void AddStream();
  void AddStreamAccepted();

 private:
  void SaveGeometry();

 private:
  static const int kStateVersion;
  static const char* kSettingsGroup;
  static const char* kMediaFilterSpec;

  Ui::MainWindow ui_;
  SystemTrayIcon* tray_icon_;
  OSD* osd_;
  TrackSlider* track_slider_;
  PlaylistSequence* playlist_sequence_;
  EditTagDialog* edit_tag_dialog_;
  MultiLoadingIndicator* multi_loading_indicator_;
  LibraryConfigDialog* library_config_dialog_;
  About* about_dialog_;

  RadioModel* radio_model_;
  Playlist* playlist_;
  Player* player_;
  Library* library_;
  GlobalShortcuts* global_shortcuts_;

  SettingsDialog* settings_dialog_;
  AddStreamDialog* add_stream_dialog_;
  AlbumCoverManager* cover_manager_;

  QMenu* playlist_menu_;
  QAction* playlist_play_pause_;
  QAction* playlist_stop_after_;
  QModelIndex playlist_menu_index_;

  QSortFilterProxyModel* library_sort_model_;

  QTimer* track_position_timer_;
  QSettings settings_;
};

#endif // MAINWINDOW_H
