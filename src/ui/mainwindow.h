/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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
class ArtistInfoView;
class ArtLoader;
class BackgroundStreams;
class CommandlineOptions;
class Database;
class DeviceManager;
class DeviceView;
class EditTagDialog;
class Equalizer;
class ErrorDialog;
class FileView;
class GlobalShortcuts;
class GroupByDialog;
class Library;
class LibraryViewContainer;
class MimeData;
class MultiLoadingIndicator;
class OrganiseDialog;
class OSD;
class Player;
class PlaylistBackend;
class PlaylistManager;
class QueueManager;
class RadioItem;
class RadioModel;
class RadioViewContainer;
class Remote;
class ScriptDialog;
class ScriptManager;
class Song;
class SongInfoBase;
class SongInfoView;
class SystemTrayIcon;
class TagFetcher;
class TaskManager;
class TrackSelectionDialog;
class TranscodeDialog;
class VisualisationContainer;
class WiimotedevShortcuts;
class Windows7ThumbBar;
class Ui_MainWindow;

class QSortFilterProxyModel;

#ifdef HAVE_LIBLASTFM
  class AlbumCoverManager;
#endif

class MainWindow : public QMainWindow, public PlatformInterface {
  Q_OBJECT

 public:
  MainWindow(BackgroundThread<Database>* database,
             TaskManager* task_manager,
             PlaylistManager* playlists,
             RadioModel* radio_model,
             Player* player,
             SystemTrayIcon* tray_icon,
             OSD* osd,
             ArtLoader* art_loader,
             QWidget *parent = 0);
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

  // Don't change the values
  enum AddBehaviour {
    AddBehaviour_Append = 1,
    AddBehaviour_Enqueue = 2,
    AddBehaviour_Load = 3,
    AddBehaviour_OpenInNew = 4
  };

  // Don't change the values
  enum PlayBehaviour {
    PlayBehaviour_Never = 1,
    PlayBehaviour_IfStopped = 2,
    PlayBehaviour_Always = 3,
  };

  void SetHiddenInTray(bool hidden);
  void CommandlineOptionsReceived(const CommandlineOptions& options);

 protected:
  void resizeEvent(QResizeEvent* event);
  void closeEvent(QCloseEvent* event);

#ifdef Q_OS_WIN32
  bool winEvent(MSG* message, long* result);
#endif

  // PlatformInterface
  void Activate();
  bool LoadUrl(const QString& url);

 private slots:
  void FilePathChanged(const QString& path);

  void MediaStopped();
  void MediaPaused();
  void MediaPlaying();
  void TrackSkipped(PlaylistItemPtr item);
  void ForceShowOSD(const Song& song);

  void PlaylistRightClick(const QPoint& global_pos, const QModelIndex& index);
  void PlaylistCurrentChanged(const QModelIndex& current);
  void PlaylistViewSelectionModelChanged();
  void PlaylistPlay();
  void PlaylistStopAfter();
  void PlaylistQueue();
  void PlaylistRemoveCurrent();
  void PlaylistEditFinished(const QModelIndex& index);
  void EditTracks();
  void EditTagDialogAccepted();
  void RenumberTracks();
  void SelectionSetValue();
  void EditValue();
  void AutoCompleteTags();
  void AutoCompleteTagsAccepted();
  void PlaylistUndoRedoChanged(QAction* undo, QAction* redo);

  void PlaylistCopyToLibrary();
  void PlaylistMoveToLibrary();
  void PlaylistCopyToDevice();
  void PlaylistOrganiseSelected(bool copy);
  void PlaylistDelete();
  void PlaylistOpenInBrowser();

  void ChangeLibraryQueryMode(QAction* action);

  void PlayIndex(const QModelIndex& index);
  void StopAfterCurrent();

  void SongChanged(const Song& song);
  void VolumeChanged(int volume);

  void CopyFilesToLibrary(const QList<QUrl>& urls);
  void MoveFilesToLibrary(const QList<QUrl>& urls);
  void CopyFilesToDevice(const QList<QUrl>& urls);

  void AddToPlaylist(QMimeData* data);
  void AddToPlaylist(QAction* action);

  void VolumeWheelEvent(int delta);
  void ToggleShowHide();

  void UpdateTrackPosition();

  void LastFMButtonVisibilityChanged(bool value);
#ifdef HAVE_LIBLASTFM
  void ScrobblingEnabledChanged(bool value);
  void Love();
#endif

  void TaskCountChanged(int count);

  void ShowLibraryConfig();
  void ReloadSettings();
  void ReloadAllSettings();
  void SetHiddenInTray() { SetHiddenInTray(true); }

  void AddFile();
  void AddFolder();
  void AddStream();
  void AddStreamAccepted();

  void CommandlineOptionsReceived(const QByteArray& serialized_options);

  void CheckForUpdates();

  void NowPlayingWidgetPositionChanged(bool above_status_bar);

  void SongSaveComplete();

#ifdef HAVE_LIBLASTFM
  void ShowCoverManager();
#endif
  void ShowAboutDialog();
  void ShowTranscodeDialog();
  void ShowErrorDialog(const QString& message);
  void ShowQueueManager();
  void ShowVisualisations();
  void EnsureSettingsDialogCreated();
  void EnsureEditTagDialogCreated();
  void OpenSettingsDialog();
  void OpenSettingsDialogAtPage(SettingsDialog::Page page);
  void ShowSongInfoConfig();
  void ShowScriptDialog();

  void SaveGeometry();

  void AddSongInfoGenerator(smart_playlists::GeneratorPtr gen);

  void DeleteFinished(const SongList& songs_with_errors);

  void Raise();

  void Exit();

 private:
  void ConnectInfoView(SongInfoBase* view);

  void ApplyAddBehaviour(AddBehaviour b, MimeData* data) const;
  void ApplyPlayBehaviour(PlayBehaviour b, MimeData* data) const;

  void CheckFullRescanRevisions();

 private:
  Ui_MainWindow* ui_;
  Windows7ThumbBar* thumbbar_;

  SystemTrayIcon* tray_icon_;
  OSD* osd_;
  boost::scoped_ptr<EditTagDialog> edit_tag_dialog_;
  TaskManager* task_manager_;
  boost::scoped_ptr<About> about_dialog_;

  BackgroundThread<Database>* database_;
  RadioModel* radio_model_;
  PlaylistBackend* playlist_backend_;
  PlaylistManager* playlists_;
  Player* player_;
  Library* library_;
  GlobalShortcuts* global_shortcuts_;
  Remote* remote_;

  DeviceManager* devices_;

  LibraryViewContainer* library_view_;
  FileView* file_view_;
  RadioViewContainer* radio_view_;
  DeviceView* device_view_;
  SongInfoView* song_info_view_;
  ArtistInfoView* artist_info_view_;

  boost::scoped_ptr<SettingsDialog> settings_dialog_;
  boost::scoped_ptr<AddStreamDialog> add_stream_dialog_;
#ifdef HAVE_LIBLASTFM
  boost::scoped_ptr<AlbumCoverManager> cover_manager_;
#endif
  boost::scoped_ptr<Equalizer> equalizer_;
  boost::scoped_ptr<TranscodeDialog> transcode_dialog_;
  boost::scoped_ptr<ErrorDialog> error_dialog_;
  boost::scoped_ptr<OrganiseDialog> organise_dialog_;
  boost::scoped_ptr<QueueManager> queue_manager_;

  boost::scoped_ptr<TagFetcher> tag_fetcher_;
  boost::scoped_ptr<TrackSelectionDialog> track_selection_dialog_;
  PlaylistItemList autocomplete_tag_items_;

#ifdef ENABLE_VISUALISATIONS
  boost::scoped_ptr<VisualisationContainer> visualisation_;
#endif

#ifdef HAVE_WIIMOTEDEV
  boost::scoped_ptr<WiimotedevShortcuts> wiimotedev_shortcuts_;
#endif

  ScriptManager* scripts_;
  boost::scoped_ptr<ScriptDialog> script_dialog_;

  QAction* library_show_all_;
  QAction* library_show_duplicates_;
  QAction* library_show_untagged_;

  QMenu* playlist_menu_;
  QAction* playlist_play_pause_;
  QAction* playlist_stop_after_;
  QAction* playlist_undoredo_;
  QAction* playlist_organise_;
  QAction* playlist_copy_to_library_;
  QAction* playlist_move_to_library_;
  QAction* playlist_copy_to_device_;
  QAction* playlist_delete_;
  QAction* playlist_open_in_browser_;
  QAction* playlist_queue_;
  QAction* playlist_add_to_another_;
  QModelIndex playlist_menu_index_;

  QSortFilterProxyModel* library_sort_model_;

  QTimer* track_position_timer_;
  QSettings settings_;

  bool was_maximized_;
  AddBehaviour doubleclick_addmode_;
  PlayBehaviour doubleclick_playmode_;
  PlayBehaviour menu_playmode_;

  BackgroundStreams* background_streams_;
};

#endif // MAINWINDOW_H
