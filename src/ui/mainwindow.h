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

#include <boost/scoped_ptr.hpp>

#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>

#include "config.h"
#include "core/mac_startup.h"
#include "engines/engine_fwd.h"
#include "library/librarymodel.h"
#include "playlist/playlistitem.h"
#include "ui/settingsdialog.h"

class About;
class AddStreamDialog;
class AlbumCoverManager;
class CommandlineOptions;
class Database;
class DeviceManager;
class EditTagDialog;
class Equalizer;
class ErrorDialog;
class GlobalShortcuts;
class GroupByDialog;
class Library;
class MultiLoadingIndicator;
class NetworkAccessManager;
class OrganiseDialog;
class OSD;
class Player;
class PlaylistBackend;
class PlaylistManager;
class PlaylistParser;
class QueueManager;
class RadioItem;
class RadioModel;
class Song;
class SystemTrayIcon;
class TaskManager;
class TranscodeDialog;
class VisualisationContainer;
class WiimotedevShortcuts;
class Ui_MainWindow;

class QSortFilterProxyModel;

class MainWindow : public QMainWindow, public PlatformInterface {
  Q_OBJECT

 public:
  MainWindow(NetworkAccessManager* network, Engine::Type engine, QWidget *parent = 0);
  ~MainWindow();

  static const char* kSettingsGroup;
  static const char* kMusicFilterSpec;
  static const char* kAllFilesFilterSpec;

  // Don't change the values
  enum StartupBehaviour {
    Startup_Remember = 1,
    Startup_AlwaysShow = 2,
    Startup_AlwaysHide = 3,
  };

  void SetHiddenInTray(bool hidden);
  void CommandlineOptionsReceived(const CommandlineOptions& options);

 protected:
  void resizeEvent(QResizeEvent* event);
  void closeEvent(QCloseEvent* event);

  // PlatformInterface
  void Activate();
  bool LoadUrl(const QString& url);

 private slots:
  void FilePathChanged(const QString& path);

  void MediaStopped();
  void MediaPaused();
  void MediaPlaying();
  void ForceShowOSD(const Song& song);

  void PlaylistRightClick(const QPoint& global_pos, const QModelIndex& index);
  void PlaylistPlay();
  void PlaylistStopAfter();
  void PlaylistQueue();
  void PlaylistRemoveCurrent();
  void PlaylistEditFinished(const QModelIndex& index);
  void EditTracks();
  void RenumberTracks();
  void SelectionSetValue();
  void EditValue();
  void PlaylistUndoRedoChanged(QAction* undo, QAction* redo);

  void PlaylistCopyToLibrary();
  void PlaylistMoveToLibrary();
  void PlaylistCopyToDevice();
  void PlaylistOrganiseSelected(bool copy);
  void PlaylistDelete();

  void PlayIndex(const QModelIndex& index);
  void StopAfterCurrent();

  void LoadLibraryItemToPlaylist(const QModelIndexList& indexes);
  void AddLibraryItemToPlaylist(const QModelIndexList& indexes);
  void LibraryItemDoubleClicked(const QModelIndex& index);

  void LoadFilesToPlaylist(const QList<QUrl>& urls);
  void AddFilesToPlaylist(const QList<QUrl>& urls);
  void FilesDoubleClicked(const QList<QUrl>& urls);

  void CopyFilesToLibrary(const QList<QUrl>& urls);
  void MoveFilesToLibrary(const QList<QUrl>& urls);

  void LoadLibrarySongsToPlaylist(const SongList& songs);
  void AddLibrarySongsToPlaylist(const SongList& songs);
  void LibrarySongsDoubleClicked(const SongList& songs);

  void LoadDeviceSongsToPlaylist(const SongList& songs);
  void AddDeviceSongsToPlaylist(const SongList& songs);
  void DeviceSongsDoubleClicked(const SongList& songs);

  void VolumeWheelEvent(int delta);
  void ToggleShowHide();

  void UpdateTrackPosition();

  void RadioDoubleClick(const QModelIndex& index);
  void InsertRadioItem(RadioItem*);
  void InsertRadioItems(const PlaylistItemList& items);
  void ScrobblingEnabledChanged(bool value);
  void LastFMButtonVisibilityChanged(bool value);
  void Love();

  void TaskCountChanged(int count);

  void ShowLibraryConfig();
  void ReloadSettings();
  void SetHiddenInTray() { SetHiddenInTray(true); }

  void AddFile();
  void AddFolder();
  void AddStream();
  void AddStreamAccepted();

  void CommandlineOptionsReceived(const QByteArray& serialized_options);

  void CheckForUpdates();

  void NowPlayingWidgetPositionChanged(bool above_status_bar);

  void SongSaveComplete();

  void ShowCoverManager();
  void ShowAboutDialog();
  void ShowTranscodeDialog();
  void ShowErrorDialog(const QString& message);
  void ShowQueueManager();
  void ShowVisualisations();
  void EnsureSettingsDialogCreated();
  void EnsureEditTagDialogCreated();
  void OpenSettingsDialog();
  void OpenSettingsDialogAtPage(SettingsDialog::Page page);

 private:
  void SaveGeometry();
  void AddFilesToPlaylist(bool clear_first, const QList<QUrl>& urls);
  void AddLibraryItemToPlaylist(bool clear_first, const QModelIndexList& indexes);
  void AddLibrarySongsToPlaylist(bool clear_first, const SongList& songs);
  void AddDeviceSongsToPlaylist(bool clear_first, const SongList& songs);
  void AddUrls(bool play_now, const QList<QUrl>& urls);

 private:
  Ui_MainWindow* ui_;
  NetworkAccessManager* network_;

  SystemTrayIcon* tray_icon_;
  OSD* osd_;
  boost::scoped_ptr<EditTagDialog> edit_tag_dialog_;
  TaskManager* task_manager_;
  boost::scoped_ptr<About> about_dialog_;

  BackgroundThread<Database>* database_;
  RadioModel* radio_model_;
  PlaylistBackend* playlist_backend_;
  PlaylistManager* playlists_;
  PlaylistParser* playlist_parser_;
  Player* player_;
  Library* library_;
  GlobalShortcuts* global_shortcuts_;

  DeviceManager* devices_;

  boost::scoped_ptr<SettingsDialog> settings_dialog_;
  boost::scoped_ptr<AddStreamDialog> add_stream_dialog_;
  boost::scoped_ptr<AlbumCoverManager> cover_manager_;
  boost::scoped_ptr<Equalizer> equalizer_;
  boost::scoped_ptr<TranscodeDialog> transcode_dialog_;
  boost::scoped_ptr<ErrorDialog> error_dialog_;
  boost::scoped_ptr<OrganiseDialog> organise_dialog_;
  boost::scoped_ptr<QueueManager> queue_manager_;

#ifdef ENABLE_VISUALISATIONS
  boost::scoped_ptr<VisualisationContainer> visualisation_;
#endif

#ifdef ENABLE_WIIMOTEDEV
  boost::scoped_ptr<WiimotedevShortcuts> wiimotedev_shortcuts_;
#endif

  QMenu* playlist_menu_;
  QAction* playlist_play_pause_;
  QAction* playlist_stop_after_;
  QAction* playlist_undoredo_;
  QAction* playlist_organise_;
  QAction* playlist_copy_to_library_;
  QAction* playlist_move_to_library_;
  QAction* playlist_copy_to_device_;
  QAction* playlist_delete_;
  QAction* playlist_queue_;
  QModelIndex playlist_menu_index_;

  QSortFilterProxyModel* library_sort_model_;

  QTimer* track_position_timer_;
  QSettings settings_;

  bool was_maximized_;
  bool autoclear_playlist_;
};

#endif // MAINWINDOW_H
