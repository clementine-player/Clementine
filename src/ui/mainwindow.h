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

#include <memory>

#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>

#include "config.h"
#include "core/lazy.h"
#include "core/mac_startup.h"
#include "core/tagreaderclient.h"
#include "engines/engine_fwd.h"
#include "library/librarymodel.h"
#include "playlist/playlistitem.h"
#include "songinfo/streamdiscoverer.h"
#include "ui/organisedialog.h"
#include "ui/settingsdialog.h"
#include "ui/streamdetailsdialog.h"

class About;
class AddStreamDialog;
class AlbumCoverManager;
class Appearance;
class Application;
class ArtistInfoView;
class BackgroundStreams;
class CommandlineOptions;
class CoverProviders;
class Database;
class DeviceManager;
class DeviceView;
class DeviceViewContainer;
class EditTagDialog;
class Equalizer;
class ErrorDialog;
class FileView;
class GlobalSearch;
class GlobalSearchView;
class GlobalShortcuts;
class GroupByDialog;
class Library;
class LibraryViewContainer;
class MimeData;
class MultiLoadingIndicator;
class OSD;
class Player;
class PlaylistBackend;
class PlaylistListContainer;
class PlaylistManager;
class QueueManager;
class InternetItem;
class InternetModel;
class InternetViewContainer;
class RipCDDialog;
class Song;
class SongInfoBase;
class SongInfoView;
class StreamDetailsDialog;
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

class MainWindow : public QMainWindow, public PlatformInterface {
  Q_OBJECT

 public:
  MainWindow(Application* app, SystemTrayIcon* tray_icon, OSD* osd,
             const CommandlineOptions& options, QWidget* parent = nullptr);
  ~MainWindow();

  static const char* kSettingsGroup;
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

  // Don't change the values
  enum PlaylistAddBehaviour {
    PlaylistAddBehaviour_Play = 1,
    PlaylistAddBehaviour_Enqueue = 2,
  };

  void SetHiddenInTray(bool hidden);
  void CommandlineOptionsReceived(const CommandlineOptions& options);

 protected:
  void keyPressEvent(QKeyEvent* event);
  void changeEvent(QEvent*);
  void resizeEvent(QResizeEvent*);
  void closeEvent(QCloseEvent* event);

#ifdef Q_OS_WIN32
  bool winEvent(MSG* message, long* result);
#endif

  // PlatformInterface
  void Activate();
  bool LoadUrl(const QString& url);

signals:
  // Signals that stop playing after track was toggled.
  void StopAfterToggled(bool stop);

  void IntroPointReached();
 private slots:
  void FilePathChanged(const QString& path);

  void MediaStopped();
  void MediaPaused();
  void MediaPlaying();
  void TrackSkipped(PlaylistItemPtr item);
  void ForceShowOSD(const Song& song, const bool toggle);

  void PlaylistRightClick(const QPoint& global_pos, const QModelIndex& index);
  void PlaylistCurrentChanged(const QModelIndex& current);
  void PlaylistViewSelectionModelChanged();
  void PlaylistPlay();
  void PlaylistStopAfter();
  void PlaylistQueue();
  void PlaylistQueuePlayNext();
  void PlaylistSkip();
  void PlaylistRemoveCurrent();
  void PlaylistEditFinished(const QModelIndex& index);
  void EditTracks();
  void EditTagDialogAccepted();
  void DiscoverStreamDetails();
  void ShowStreamDetails(const StreamDetails& details);
  void RenumberTracks();
  void SelectionSetValue();
  void EditValue();
  void AutoCompleteTags();
  void AutoCompleteTagsAccepted();
  void PlaylistUndoRedoChanged(QAction* undo, QAction* redo);
  void AddFilesToTranscoder();

  void SearchForArtist();
  void SearchForAlbum();

  void PlaylistCopyToLibrary();
  void PlaylistMoveToLibrary();
  void PlaylistCopyToDevice();
  void PlaylistOrganiseSelected(bool copy);
  void PlaylistDelete();
  void PlaylistOpenInBrowser();
  void ShowInLibrary();

  void ChangeLibraryQueryMode(QAction* action);

  void PlayIndex(const QModelIndex& index);
  void PlaylistDoubleClick(const QModelIndex& index);
  void StopAfterCurrent();

  void SongChanged(const Song& song);
  void VolumeChanged(int volume);

  void CopyFilesToLibrary(const QList<QUrl>& urls);
  void MoveFilesToLibrary(const QList<QUrl>& urls);
  void CopyFilesToDevice(const QList<QUrl>& urls);
  void EditFileTags(const QList<QUrl>& urls);

  void AddToPlaylist(QMimeData* data);
  void AddToPlaylist(QAction* action);

  void VolumeWheelEvent(int delta);
  void ToggleShowHide();

  void Seeked(qlonglong microseconds);
  void UpdateTrackPosition();
  void UpdateTrackSliderPosition();

  // Handle visibility of LastFM icons
  void LastFMButtonVisibilityChanged(bool value);
  void ScrobbleButtonVisibilityChanged(bool value);
  void SetToggleScrobblingIcon(bool value);
#ifdef HAVE_LIBLASTFM
  void ScrobblingEnabledChanged(bool value);
  void Love();
  void ScrobbledRadioStream();
#endif

  void TaskCountChanged(int count);

  void ShowLibraryConfig();
  void ReloadSettings();
  void ReloadAllSettings();
  void RefreshStyleSheet();
  void SetHiddenInTray() { SetHiddenInTray(true); }

  void AddFile();
  void AddFolder();
  void AddStream();
  void AddStreamAccepted();
  void OpenRipCDDialog();
  void AddCDTracks();
  void AddPodcast();

  void CommandlineOptionsReceived(const QByteArray& serialized_options);

  void CheckForUpdates();

  void NowPlayingWidgetPositionChanged(bool above_status_bar);

  void SongSaveComplete(TagReaderReply* reply,
                        const QPersistentModelIndex& index);

  void ShowCoverManager();
#ifdef HAVE_LIBLASTFM
  void ScrobbleSubmitted();
  void ScrobbleError(int value);
#endif
  void ShowAboutDialog();
  void ShowTranscodeDialog();
  void ShowErrorDialog(const QString& message);
  void ShowQueueManager();
  void ShowVisualisations();
  SettingsDialog* CreateSettingsDialog();
  EditTagDialog* CreateEditTagDialog();
  StreamDiscoverer* CreateStreamDiscoverer();
  void OpenSettingsDialog();
  void OpenSettingsDialogAtPage(SettingsDialog::Page page);
  void ShowSongInfoConfig();

  void SaveGeometry();
  void SavePlaybackStatus();
  void LoadPlaybackStatus();
  void ResumePlayback();

  void AddSongInfoGenerator(smart_playlists::GeneratorPtr gen);

  void DeleteFinished(const SongList& songs_with_errors);

  void Raise();

  void Exit();

  void HandleNotificationPreview(OSD::Behaviour type, QString line1,
                                 QString line2);

  void ScrollToInternetIndex(const QModelIndex& index);
  void FocusLibraryTab();
  void FocusGlobalSearchField();
  void DoGlobalSearch(const QString& query);

  void ShowConsole();

 private:
  void ConnectInfoView(SongInfoBase* view);

  void ApplyAddBehaviour(AddBehaviour b, MimeData* data) const;
  void ApplyPlayBehaviour(PlayBehaviour b, MimeData* data) const;

  void CheckFullRescanRevisions();

  // creates the icon by painting the full one depending on the current position
  QPixmap CreateOverlayedIcon(int position, int scrobble_point);

 private:
  Ui_MainWindow* ui_;
  Windows7ThumbBar* thumbbar_;

  Application* app_;
  SystemTrayIcon* tray_icon_;
  OSD* osd_;
  Lazy<EditTagDialog> edit_tag_dialog_;
  Lazy<About> about_dialog_;
  Lazy<StreamDiscoverer> stream_discoverer_;

  GlobalShortcuts* global_shortcuts_;

  GlobalSearchView* global_search_view_;
  LibraryViewContainer* library_view_;
  FileView* file_view_;
#ifdef HAVE_AUDIOCD
  std::unique_ptr<RipCDDialog> rip_cd_dialog_;
#endif
  PlaylistListContainer* playlist_list_;
  InternetViewContainer* internet_view_;
  DeviceViewContainer* device_view_container_;
  DeviceView* device_view_;
  SongInfoView* song_info_view_;
  ArtistInfoView* artist_info_view_;

  Lazy<SettingsDialog> settings_dialog_;
  Lazy<AddStreamDialog> add_stream_dialog_;
  Lazy<AlbumCoverManager> cover_manager_;
  std::unique_ptr<Equalizer> equalizer_;
  Lazy<TranscodeDialog> transcode_dialog_;
  Lazy<ErrorDialog> error_dialog_;
  Lazy<OrganiseDialog> organise_dialog_;
  Lazy<QueueManager> queue_manager_;

  std::unique_ptr<TagFetcher> tag_fetcher_;
  std::unique_ptr<TrackSelectionDialog> track_selection_dialog_;
  PlaylistItemList autocomplete_tag_items_;

#ifdef HAVE_VISUALISATIONS
  std::unique_ptr<VisualisationContainer> visualisation_;
#endif

#ifdef HAVE_WIIMOTEDEV
  std::unique_ptr<WiimotedevShortcuts> wiimotedev_shortcuts_;
#endif

  QAction* library_show_all_;
  QAction* library_show_duplicates_;
  QAction* library_show_untagged_;

  QMenu* playlist_menu_;
  QAction* playlist_play_pause_;
  QAction* playlist_stop_after_;
  QAction* playlist_undoredo_;
  QAction* playlist_organise_;
  QAction* playlist_show_in_library_;
  QAction* playlist_copy_to_library_;
  QAction* playlist_move_to_library_;
  QAction* playlist_copy_to_device_;
  QAction* playlist_delete_;
  QAction* playlist_open_in_browser_;
  QAction* playlist_queue_;
  QAction* playlist_queue_play_next_;
  QAction* playlist_skip_;
  QAction* playlist_add_to_another_;
  QList<QAction*> playlistitem_actions_;
  QAction* playlistitem_actions_separator_;
  QModelIndex playlist_menu_index_;

  QAction* search_for_artist_;
  QAction* search_for_album_;

  QSortFilterProxyModel* library_sort_model_;

  QTimer* track_position_timer_;
  QTimer* track_slider_timer_;
  QSettings settings_;

  bool initialized_;
  bool was_maximized_;
  int saved_playback_position_;
  Engine::State saved_playback_state_;
  AddBehaviour doubleclick_addmode_;
  PlayBehaviour doubleclick_playmode_;
  PlaylistAddBehaviour doubleclick_playlist_addmode_;
  PlayBehaviour menu_playmode_;

  BackgroundStreams* background_streams_;
};

#endif  // MAINWINDOW_H
