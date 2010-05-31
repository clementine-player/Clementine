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

#include "engines/engine_fwd.h"
#include "library/librarymodel.h"
#include "playlist/playlistitem.h"

class About;
class AddStreamDialog;
class AlbumCoverManager;
class CommandlineOptions;
class Database;
class EditTagDialog;
class Equalizer;
class ErrorDialog;
class GlobalShortcuts;
class GlobalShortcutsDialog;
class GroupByDialog;
class Library;
class MultiLoadingIndicator;
class NetworkAccessManager;
class OSD;
class Player;
class PlaylistBackend;
class PlaylistManager;
class PlaylistParser;
class PlaylistSequence;
class RadioItem;
class RadioModel;
class SettingsDialog;
class Song;
class SystemTrayIcon;
class TrackSlider;
class TranscodeDialog;
class Ui_MainWindow;

class QSortFilterProxyModel;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(NetworkAccessManager* network, Engine::Type engine, QWidget *parent = 0);
  ~MainWindow();

  static const char* kSettingsGroup;
  static const char* kMusicFilterSpec;
  static const char* kPlaylistFilterSpec;
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

  bool event(QEvent* event);

 private slots:
  void FilePathChanged(const QString& path);

  void MediaStopped();
  void MediaPaused();
  void MediaPlaying();
  void ForceShowOSD(const Song& song);

  void PlaylistRightClick(const QPoint& global_pos, const QModelIndex& index);
  void PlaylistPlay();
  void PlaylistStopAfter();
  void PlaylistRemoveCurrent();
  void PlaylistEditFinished(const QModelIndex& index);
  void EditTracks();
  void RenumberTracks();
  void SelectionSetValue();
  void EditValue();
  void PlaylistUndoRedoChanged(QAction* undo, QAction* redo);

  void PlayIndex(const QModelIndex& index);
  void StopAfterCurrent();

  void LoadLibraryItemToPlaylist(const QModelIndex& index);
  void AddLibraryItemToPlaylist(const QModelIndex& index);
  void LibraryItemDoubleClicked(const QModelIndex& index);

  void LoadFilesToPlaylist(const QList<QUrl>& urls);
  void AddFilesToPlaylist(const QList<QUrl>& urls);
  void FilesDoubleClicked(const QList<QUrl>& urls);

  void VolumeWheelEvent(int delta);
  void TrayClicked(QSystemTrayIcon::ActivationReason reason);

  void UpdateTrackPosition();

  void RadioDoubleClick(const QModelIndex& index);
  void InsertRadioItem(RadioItem*);
  void InsertRadioItems(const PlaylistItemList& items);
  void ScrobblingEnabledChanged(bool value);
  void LastFMButtonVisibilityChanged(bool value);
  void Love();

  void LibraryScanStarted();
  void LibraryScanFinished();

  void ReloadSettings();

  void AddFile();
  void AddFolder();
  void AddStream();
  void AddStreamAccepted();

  void CommandlineOptionsReceived(const QByteArray& serialized_options);

  void CheckForUpdates();

 private:
  void SaveGeometry();
  void AddFilesToPlaylist(bool clear_first, const QList<QUrl>& urls);
  void AddLibraryItemToPlaylist(bool clear_first, const QModelIndex& index);

 private:
  static const int kStateVersion;

  Ui_MainWindow* ui_;
  SystemTrayIcon* tray_icon_;
  OSD* osd_;
  TrackSlider* track_slider_;
  PlaylistSequence* playlist_sequence_;
  boost::scoped_ptr<EditTagDialog> edit_tag_dialog_;
  MultiLoadingIndicator* multi_loading_indicator_;
  boost::scoped_ptr<About> about_dialog_;

  Database* database_;
  RadioModel* radio_model_;
  PlaylistBackend* playlist_backend_;
  PlaylistManager* playlists_;
  PlaylistParser* playlist_parser_;
  Player* player_;
  Library* library_;
  GlobalShortcuts* global_shortcuts_;

  boost::scoped_ptr<SettingsDialog> settings_dialog_;
  boost::scoped_ptr<AddStreamDialog> add_stream_dialog_;
  boost::scoped_ptr<AlbumCoverManager> cover_manager_;
  boost::scoped_ptr<Equalizer> equalizer_;
  boost::scoped_ptr<TranscodeDialog> transcode_dialog_;
  boost::scoped_ptr<GlobalShortcutsDialog> global_shortcuts_dialog_;
  boost::scoped_ptr<ErrorDialog> error_dialog_;

  QMenu* playlist_menu_;
  QAction* playlist_play_pause_;
  QAction* playlist_stop_after_;
  QAction* playlist_undoredo_;
  QModelIndex playlist_menu_index_;

  QSortFilterProxyModel* library_sort_model_;

  QTimer* track_position_timer_;
  QSettings settings_;

  bool was_maximized_;
  bool autoclear_playlist_;
};

#endif // MAINWINDOW_H
