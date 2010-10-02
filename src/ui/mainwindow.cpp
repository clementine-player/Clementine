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

#include "mainwindow.h"
#include "stylehelper.h"
#include "ui_mainwindow.h"
#include "core/commandlineoptions.h"
#include "core/database.h"
#include "core/globalshortcuts.h"
#include "core/mac_startup.h"
#include "core/mergedproxymodel.h"
#include "core/modelfuturewatcher.h"
#include "core/player.h"
#include "core/songloader.h"
#include "core/stylesheetloader.h"
#include "core/taskmanager.h"
#include "devices/devicemanager.h"
#include "devices/devicestatefiltermodel.h"
#include "devices/deviceview.h"
#include "engines/enginebase.h"
#include "library/groupbydialog.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/libraryconfig.h"
#include "library/librarydirectorymodel.h"
#include "library/libraryfilterwidget.h"
#include "library/libraryviewcontainer.h"
#include "lyrics/lyricfetcher.h"
#include "lyrics/lyricview.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#include "playlist/playlistview.h"
#include "playlist/queue.h"
#include "playlist/queuemanager.h"
#include "playlist/songloaderinserter.h"
#include "playlist/songplaylistitem.h"
#include "playlistparsers/playlistparser.h"
#include "radio/lastfmservice.h"
#include "radio/magnatuneservice.h"
#include "radio/radiomodel.h"
#include "radio/radioview.h"
#include "radio/radioviewcontainer.h"
#include "radio/savedradio.h"
#include "transcoder/transcodedialog.h"
#include "ui/about.h"
#include "ui/addstreamdialog.h"
#include "ui/albumcovermanager.h"
#include "ui/edittagdialog.h"
#include "ui/equalizer.h"
#include "ui/iconloader.h"
#ifdef Q_OS_DARWIN
#include "ui/macsystemtrayicon.h"
#endif
#include "ui/organisedialog.h"
#include "ui/qtsystemtrayicon.h"
#include "ui/settingsdialog.h"
#include "ui/systemtrayicon.h"
#include "widgets/errordialog.h"
#include "widgets/fileview.h"
#include "widgets/multiloadingindicator.h"
#include "widgets/osd.h"
#include "widgets/trackslider.h"

#ifdef ENABLE_WIIMOTEDEV
# include "wiimotedev/shortcuts.h"
#endif

#ifdef HAVE_GSTREAMER
# include "engines/gstengine.h"
#endif

#ifdef ENABLE_VISUALISATIONS
# include "visualisations/visualisationcontainer.h"
#endif

#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QLinearGradient>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QtDebug>
#include <QTimer>
#include <QUndoStack>


#include <cmath>

using boost::shared_ptr;
using boost::scoped_ptr;

#ifdef Q_OS_DARWIN
// Non exported mac-specific function.
void qt_mac_set_dock_menu(QMenu*);
#endif

const char* MainWindow::kSettingsGroup = "MainWindow";
const char* MainWindow::kMusicFilterSpec =
    QT_TR_NOOP("Music (*.mp3 *.ogg *.flac *.mpc *.m4a *.aac *.wma *.mp4 *.spx *.wav)");
const char* MainWindow::kAllFilesFilterSpec =
    QT_TR_NOOP("All Files (*)");

MainWindow::MainWindow(NetworkAccessManager* network, Engine::Type engine, QWidget *parent)
  : QMainWindow(parent),
    ui_(new Ui_MainWindow),
    network_(network),
    tray_icon_(SystemTrayIcon::CreateSystemTrayIcon(this)),
    osd_(new OSD(tray_icon_, network, this)),
    task_manager_(new TaskManager(this)),
    database_(new BackgroundThreadImplementation<Database, Database>(this)),
    radio_model_(NULL),
    playlist_backend_(NULL),
    playlists_(new PlaylistManager(task_manager_, this)),
    playlist_parser_(new PlaylistParser(this)),
    player_(NULL),
    library_(NULL),
    global_shortcuts_(new GlobalShortcuts(this)),
    devices_(NULL),
    library_view_(new LibraryViewContainer(this)),
    file_view_(new FileView(this)),
    radio_view_(new RadioViewContainer(this)),
    device_view_(new DeviceView(this)),
    lyric_view_(new LyricView(this)),
    settings_dialog_(NULL),
    cover_manager_(NULL),
    equalizer_(new Equalizer),
    error_dialog_(NULL),
    organise_dialog_(new OrganiseDialog(task_manager_)),
    queue_manager_(NULL),
#ifdef ENABLE_VISUALISATIONS
    visualisation_(NULL),
#endif
#ifdef ENABLE_WIIMOTEDEV
    wiimotedev_shortcuts_(NULL),
#endif
    playlist_menu_(new QMenu(this)),
    library_sort_model_(new QSortFilterProxyModel(this)),
    track_position_timer_(new QTimer(this)),
    was_maximized_(false)
{
  // Wait for the database thread to start - lots of stuff depends on it.
  database_->Start(true);

  // Create some objects in the database thread
  playlist_backend_ = new PlaylistBackend;
  playlist_backend_->moveToThread(database_);
  playlist_backend_->SetDatabase(database_->Worker());

  // Create stuff that needs the database
  radio_model_ = new RadioModel(database_, network, task_manager_, this);
  player_ = new Player(playlists_, radio_model_->GetLastFMService(), engine, this);
  library_ = new Library(database_, task_manager_, this);
  devices_ = new DeviceManager(database_, task_manager_, this);

  // Initialise the UI
  ui_->setupUi(this);
  ui_->multi_loading_indicator->SetTaskManager(task_manager_);
  ui_->volume->setValue(player_->GetVolume());

  // Add tabs to the fancy tab widget
  AddFancyTab(library_view_, IconLoader::Load("folder-sound"), tr("Library"));
  AddFancyTab(file_view_, IconLoader::Load("document-open"), tr("Files"));
  AddFancyTab(radio_view_, QIcon(":last.fm/icon_radio.png"), tr("Internet"));
  AddFancyTab(device_view_, IconLoader::Load("multimedia-player-ipod-mini-blue"), tr("Devices"));
  ui_->tabs->addSpacer();
  AddFancyTab(lyric_view_, IconLoader::Load("view-media-lyrics"), tr("Lyrics"));
  AddFancyTab(new QWidget, IconLoader::Load("view-media-lyrics"), tr("Song info"));
  AddFancyTab(new QWidget, IconLoader::Load("view-media-lyrics"), tr("Artist info"));

  ui_->tabs->statusBar()->hide();
  ui_->tabs->setBackgroundPixmap(QPixmap(":/sidebar_background.png"));
  StyleHelper::setBaseColor(palette().color(QPalette::Highlight).darker());

  track_position_timer_->setInterval(1000);
  connect(track_position_timer_, SIGNAL(timeout()), SLOT(UpdateTrackPosition()));

  // Start initialising the player
  player_->Init();

#ifdef HAVE_GSTREAMER
  if (qobject_cast<GstEngine*>(player_->GetEngine()) == NULL) {
    ui_->action_transcode->setEnabled(false);
  }
#else // HAVE_GSTREAMER
  ui_->action_transcode->setEnabled(false);
#endif // HAVE_GSTREAMER

  // Models
  library_sort_model_->setSourceModel(library_->model());
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  ui_->playlist->SetManager(playlists_);

  library_view_->view()->setModel(library_sort_model_);
  library_view_->view()->SetLibrary(library_->model());
  library_view_->view()->SetTaskManager(task_manager_);
  library_view_->view()->SetDeviceManager(devices_);

  radio_view_->SetModel(radio_model_);

  device_view_->SetDeviceManager(devices_);
  device_view_->SetLibrary(library_->model());

  organise_dialog_->SetDestinationModel(library_->model()->directory_model());

  // Icons
  ui_->action_about->setIcon(IconLoader::Load("help-about"));
  ui_->action_about_qt->setIcon(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"));
  ui_->action_add_file->setIcon(IconLoader::Load("document-open"));
  ui_->action_add_folder->setIcon(IconLoader::Load("document-open-folder"));
  ui_->action_add_stream->setIcon(IconLoader::Load("document-open-remote"));
  ui_->action_clear_playlist->setIcon(IconLoader::Load("edit-clear-list"));
  ui_->action_configure->setIcon(IconLoader::Load("configure"));
  ui_->action_cover_manager->setIcon(IconLoader::Load("download"));
  ui_->action_edit_track->setIcon(IconLoader::Load("edit-rename"));
  ui_->action_equalizer->setIcon(IconLoader::Load("view-media-equalizer"));
  ui_->action_jump->setIcon(IconLoader::Load("go-jump"));
  ui_->action_next_track->setIcon(IconLoader::Load("media-skip-forward"));
  ui_->action_open_media->setIcon(IconLoader::Load("document-open"));
  ui_->action_play_pause->setIcon(IconLoader::Load("media-playback-start"));
  ui_->action_previous_track->setIcon(IconLoader::Load("media-skip-backward"));
  ui_->action_quit->setIcon(IconLoader::Load("application-exit"));
  ui_->action_remove_from_playlist->setIcon(IconLoader::Load("list-remove"));
  ui_->action_repeat_mode->setIcon(IconLoader::Load("media-playlist-repeat"));
  ui_->action_shuffle->setIcon(IconLoader::Load("x-clementine-shuffle"));
  ui_->action_shuffle_mode->setIcon(IconLoader::Load("media-playlist-shuffle"));
  ui_->action_stop->setIcon(IconLoader::Load("media-playback-stop"));
  ui_->action_stop_after_this_track->setIcon(IconLoader::Load("media-playback-stop"));
  ui_->action_new_playlist->setIcon(IconLoader::Load("document-new"));
  ui_->action_load_playlist->setIcon(IconLoader::Load("document-open"));
  ui_->action_save_playlist->setIcon(IconLoader::Load("document-save"));
  ui_->action_update_library->setIcon(IconLoader::Load("view-refresh"));
  ui_->action_rain->setIcon(IconLoader::Load("weather-showers-scattered"));

  // File view connections
  connect(file_view_, SIGNAL(AddToPlaylist(QList<QUrl>)), SLOT(AddFilesToPlaylist(QList<QUrl>)));
  connect(file_view_, SIGNAL(Load(QList<QUrl>)), SLOT(LoadFilesToPlaylist(QList<QUrl>)));
  connect(file_view_, SIGNAL(DoubleClicked(QList<QUrl>)), SLOT(FilesDoubleClicked(QList<QUrl>)));
  connect(file_view_, SIGNAL(PathChanged(QString)), SLOT(FilePathChanged(QString)));
  connect(file_view_, SIGNAL(CopyToLibrary(QList<QUrl>)), SLOT(CopyFilesToLibrary(QList<QUrl>)));
  connect(file_view_, SIGNAL(MoveToLibrary(QList<QUrl>)), SLOT(MoveFilesToLibrary(QList<QUrl>)));
  connect(file_view_, SIGNAL(CopyToDevice(QList<QUrl>)), SLOT(CopyFilesToDevice(QList<QUrl>)));
  file_view_->SetTaskManager(task_manager_);

  // Action connections
  connect(ui_->action_next_track, SIGNAL(triggered()), player_, SLOT(Next()));
  connect(ui_->action_previous_track, SIGNAL(triggered()), player_, SLOT(Previous()));
  connect(ui_->action_play_pause, SIGNAL(triggered()), player_, SLOT(PlayPause()));
  connect(ui_->action_stop, SIGNAL(triggered()), player_, SLOT(Stop()));
  connect(ui_->action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
  connect(ui_->action_stop_after_this_track, SIGNAL(triggered()), SLOT(StopAfterCurrent()));
  connect(ui_->action_ban, SIGNAL(triggered()), radio_model_->GetLastFMService(), SLOT(Ban()));
  connect(ui_->action_love, SIGNAL(triggered()), SLOT(Love()));
  connect(ui_->action_clear_playlist, SIGNAL(triggered()), playlists_, SLOT(ClearCurrent()));
  connect(ui_->action_remove_from_playlist, SIGNAL(triggered()), SLOT(PlaylistRemoveCurrent()));
  connect(ui_->action_edit_track, SIGNAL(triggered()), SLOT(EditTracks()));
  connect(ui_->action_renumber_tracks, SIGNAL(triggered()), SLOT(RenumberTracks()));
  connect(ui_->action_selection_set_value, SIGNAL(triggered()), SLOT(SelectionSetValue()));
  connect(ui_->action_edit_value, SIGNAL(triggered()), SLOT(EditValue()));
  connect(ui_->action_configure, SIGNAL(triggered()), SLOT(OpenSettingsDialog()));
  connect(ui_->action_about, SIGNAL(triggered()), SLOT(ShowAboutDialog()));
  connect(ui_->action_about_qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  connect(ui_->action_shuffle, SIGNAL(triggered()), playlists_, SLOT(ShuffleCurrent()));
  connect(ui_->action_open_media, SIGNAL(triggered()), SLOT(AddFile()));
  connect(ui_->action_add_file, SIGNAL(triggered()), SLOT(AddFile()));
  connect(ui_->action_add_folder, SIGNAL(triggered()), SLOT(AddFolder()));
  connect(ui_->action_add_stream, SIGNAL(triggered()), SLOT(AddStream()));
  connect(ui_->action_cover_manager, SIGNAL(triggered()), SLOT(ShowCoverManager()));
  connect(ui_->action_equalizer, SIGNAL(triggered()), equalizer_.get(), SLOT(show()));
  connect(ui_->action_transcode, SIGNAL(triggered()), SLOT(ShowTranscodeDialog()));
  connect(ui_->action_jump, SIGNAL(triggered()), ui_->playlist->view(), SLOT(JumpToCurrentlyPlayingTrack()));
  connect(ui_->action_update_library, SIGNAL(triggered()), library_, SLOT(IncrementalScan()));
  connect(ui_->action_rain, SIGNAL(toggled(bool)), player_, SLOT(MakeItRain(bool)));
  connect(ui_->action_hypnotoad, SIGNAL(toggled(bool)), player_, SLOT(AllHail(bool)));
  connect(ui_->action_queue_manager, SIGNAL(triggered()), SLOT(ShowQueueManager()));

  // Give actions to buttons
  ui_->forward_button->setDefaultAction(ui_->action_next_track);
  ui_->back_button->setDefaultAction(ui_->action_previous_track);
  ui_->pause_play_button->setDefaultAction(ui_->action_play_pause);
  ui_->stop_button->setDefaultAction(ui_->action_stop);
  ui_->love_button->setDefaultAction(ui_->action_love);
  ui_->ban_button->setDefaultAction(ui_->action_ban);
  ui_->clear_playlist_button->setDefaultAction(ui_->action_clear_playlist);
  ui_->playlist->SetActions(ui_->action_new_playlist, ui_->action_save_playlist,
                            ui_->action_load_playlist);

#ifdef ENABLE_VISUALISATIONS
  connect(ui_->action_visualisations, SIGNAL(triggered()), SLOT(ShowVisualisations()));
#else
  ui_->action_visualisations->setEnabled(false);
#endif

  // Add the shuffle and repeat action groups to the menu
  ui_->action_shuffle_mode->setMenu(ui_->playlist_sequence->shuffle_menu());
  ui_->action_repeat_mode->setMenu(ui_->playlist_sequence->repeat_menu());

  // Stop actions
  QMenu* stop_menu = new QMenu(this);
  stop_menu->addAction(ui_->action_stop);
  stop_menu->addAction(ui_->action_stop_after_this_track);
  ui_->stop_button->setMenu(stop_menu);

  // Player connections
  connect(ui_->volume, SIGNAL(valueChanged(int)), player_, SLOT(SetVolume(int)));

  connect(player_, SIGNAL(Error(QString)), SLOT(ShowErrorDialog(QString)));
  connect(player_, SIGNAL(Paused()), SLOT(MediaPaused()));
  connect(player_, SIGNAL(Playing()), SLOT(MediaPlaying()));
  connect(player_, SIGNAL(Stopped()), SLOT(MediaStopped()));

  connect(player_, SIGNAL(Paused()), playlists_, SLOT(SetActivePaused()));
  connect(player_, SIGNAL(Playing()), playlists_, SLOT(SetActivePlaying()));
  connect(player_, SIGNAL(Stopped()), playlists_, SLOT(SetActiveStopped()));

  connect(player_, SIGNAL(Paused()), ui_->playlist->view(), SLOT(StopGlowing()));
  connect(player_, SIGNAL(Playing()), ui_->playlist->view(), SLOT(StartGlowing()));
  connect(player_, SIGNAL(Stopped()), ui_->playlist->view(), SLOT(StopGlowing()));
  connect(player_, SIGNAL(Paused()), ui_->playlist, SLOT(ActivePaused()));
  connect(player_, SIGNAL(Playing()), ui_->playlist, SLOT(ActivePlaying()));
  connect(player_, SIGNAL(Stopped()), ui_->playlist, SLOT(ActiveStopped()));

  connect(player_, SIGNAL(Paused()), osd_, SLOT(Paused()));
  connect(player_, SIGNAL(Stopped()), osd_, SLOT(Stopped()));
  connect(player_, SIGNAL(PlaylistFinished()), osd_, SLOT(PlaylistFinished()));
  connect(player_, SIGNAL(VolumeChanged(int)), osd_, SLOT(VolumeChanged(int)));
  connect(player_, SIGNAL(VolumeChanged(int)), ui_->volume, SLOT(setValue(int)));
  connect(player_, SIGNAL(ForceShowOSD(Song)), SLOT(ForceShowOSD(Song)));
  connect(playlists_, SIGNAL(CurrentSongChanged(Song)), osd_, SLOT(SongChanged(Song)));
  connect(playlists_, SIGNAL(CurrentSongChanged(Song)), player_, SLOT(CurrentMetadataChanged(Song)));
  connect(playlists_, SIGNAL(CurrentSongChanged(Song)), lyric_view_, SLOT(SongChanged(Song)));
  connect(playlists_, SIGNAL(PlaylistChanged()), player_, SLOT(PlaylistChanged()));
  connect(playlists_, SIGNAL(EditingFinished(QModelIndex)), SLOT(PlaylistEditFinished(QModelIndex)));
  connect(playlists_, SIGNAL(Error(QString)), SLOT(ShowErrorDialog(QString)));
  connect(playlists_, SIGNAL(SummaryTextChanged(QString)), ui_->playlist_summary, SLOT(setText(QString)));
  connect(playlists_, SIGNAL(PlayRequested(QModelIndex)), SLOT(PlayIndex(QModelIndex)));

  connect(ui_->playlist->view(), SIGNAL(doubleClicked(QModelIndex)), SLOT(PlayIndex(QModelIndex)));
  connect(ui_->playlist->view(), SIGNAL(PlayPauseItem(QModelIndex)), SLOT(PlayIndex(QModelIndex)));
  connect(ui_->playlist->view(), SIGNAL(RightClicked(QPoint,QModelIndex)), SLOT(PlaylistRightClick(QPoint,QModelIndex)));

  connect(ui_->track_slider, SIGNAL(ValueChanged(int)), player_, SLOT(Seek(int)));

  // Database connections
  connect(database_->Worker().get(), SIGNAL(Error(QString)), SLOT(ShowErrorDialog(QString)));

  // Library connections
  connect(library_view_->view(), SIGNAL(doubleClicked(QModelIndex)), SLOT(LibraryItemDoubleClicked(QModelIndex)));
  connect(library_view_->view(), SIGNAL(Load(QModelIndexList)), SLOT(LoadLibraryItemToPlaylist(QModelIndexList)));
  connect(library_view_->view(), SIGNAL(AddToPlaylist(QModelIndexList)), SLOT(AddLibraryItemToPlaylist(QModelIndexList)));
  connect(library_view_->view(), SIGNAL(ShowConfigDialog()), SLOT(ShowLibraryConfig()));
  connect(library_->model(), SIGNAL(TotalSongCountUpdated(int)), library_view_->view(), SLOT(TotalSongCountUpdated(int)));

  connect(task_manager_, SIGNAL(PauseLibraryWatchers()), library_, SLOT(PauseWatcher()));
  connect(task_manager_, SIGNAL(ResumeLibraryWatchers()), library_, SLOT(ResumeWatcher()));

  // Devices connections
  connect(devices_, SIGNAL(Error(QString)), SLOT(ShowErrorDialog(QString)));
  connect(device_view_, SIGNAL(Load(SongList)), SLOT(LoadDeviceSongsToPlaylist(SongList)));
  connect(device_view_, SIGNAL(AddToPlaylist(SongList)), SLOT(AddDeviceSongsToPlaylist(SongList)));
  connect(device_view_, SIGNAL(DoubleClicked(SongList)), SLOT(DeviceSongsDoubleClicked(SongList)));

  // Library filter widget
  QAction* library_config_action = new QAction(
      IconLoader::Load("configure"), tr("Configure library..."), this);
  connect(library_config_action, SIGNAL(triggered()), SLOT(ShowLibraryConfig()));
  library_view_->filter()->SetSettingsGroup(kSettingsGroup);
  library_view_->filter()->SetLibraryModel(library_->model());
  library_view_->filter()->AddMenuAction(library_config_action);

  // Playlist menu
  playlist_play_pause_ = playlist_menu_->addAction(tr("Play"), this, SLOT(PlaylistPlay()));
  playlist_menu_->addAction(ui_->action_stop);
  playlist_stop_after_ = playlist_menu_->addAction(IconLoader::Load("media-playback-stop"), tr("Stop after this track"), this, SLOT(PlaylistStopAfter()));
  playlist_queue_ = playlist_menu_->addAction("", this, SLOT(PlaylistQueue()));
  playlist_queue_->setShortcut(QKeySequence("Ctrl+D"));
  ui_->playlist->addAction(playlist_queue_);
  playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_remove_from_playlist);
  playlist_undoredo_ = playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_edit_track);
  playlist_menu_->addAction(ui_->action_edit_value);
  playlist_menu_->addAction(ui_->action_renumber_tracks);
  playlist_menu_->addAction(ui_->action_selection_set_value);
  playlist_menu_->addSeparator();
  playlist_copy_to_library_ = playlist_menu_->addAction(IconLoader::Load("edit-copy"), tr("Copy to library..."), this, SLOT(PlaylistCopyToLibrary()));
  playlist_move_to_library_ = playlist_menu_->addAction(IconLoader::Load("go-jump"), tr("Move to library..."), this, SLOT(PlaylistMoveToLibrary()));
  playlist_organise_ = playlist_menu_->addAction(IconLoader::Load("edit-copy"), tr("Organise files..."), this, SLOT(PlaylistMoveToLibrary()));
  playlist_copy_to_device_ = playlist_menu_->addAction(IconLoader::Load("multimedia-player-ipod-mini-blue"), tr("Copy to device..."), this, SLOT(PlaylistCopyToDevice()));
  playlist_delete_ = playlist_menu_->addAction(IconLoader::Load("edit-delete"), tr("Delete files..."), this, SLOT(PlaylistDelete()));
  playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_clear_playlist);
  playlist_menu_->addAction(ui_->action_shuffle);

#ifdef Q_OS_DARWIN
  ui_->action_shuffle->setShortcut(QKeySequence());
#endif

  playlist_delete_->setVisible(false); // TODO

  connect(ui_->playlist, SIGNAL(UndoRedoActionsChanged(QAction*,QAction*)),
          SLOT(PlaylistUndoRedoChanged(QAction*,QAction*)));

  playlist_copy_to_device_->setDisabled(devices_->connected_devices_model()->rowCount() == 0);
  connect(devices_->connected_devices_model(), SIGNAL(IsEmptyChanged(bool)),
          playlist_copy_to_device_, SLOT(setDisabled(bool)));

  // Radio connections
  connect(radio_model_, SIGNAL(StreamError(QString)), SLOT(ShowErrorDialog(QString)));
  connect(radio_model_, SIGNAL(AsyncLoadFinished(PlaylistItem::SpecialLoadResult)), player_, SLOT(HandleSpecialLoad(PlaylistItem::SpecialLoadResult)));
  connect(radio_model_, SIGNAL(StreamMetadataFound(QUrl,Song)), playlists_, SLOT(SetActiveStreamMetadata(QUrl,Song)));
  connect(radio_model_, SIGNAL(OpenSettingsAtPage(SettingsDialog::Page)), SLOT(OpenSettingsDialogAtPage(SettingsDialog::Page)));
  connect(radio_model_, SIGNAL(AddItemToPlaylist(RadioItem*)), SLOT(InsertRadioItem(RadioItem*)));
  connect(radio_model_, SIGNAL(AddItemsToPlaylist(PlaylistItemList)), SLOT(InsertRadioItems(PlaylistItemList)));
  connect(radio_model_->GetLastFMService(), SIGNAL(ScrobblingEnabledChanged(bool)), SLOT(ScrobblingEnabledChanged(bool)));
  connect(radio_model_->GetLastFMService(), SIGNAL(ButtonVisibilityChanged(bool)), SLOT(LastFMButtonVisibilityChanged(bool)));
  connect(radio_view_->tree(), SIGNAL(doubleClicked(QModelIndex)), SLOT(RadioDoubleClick(QModelIndex)));
  connect(radio_model_->Service<MagnatuneService>(), SIGNAL(DownloadFinished(QStringList)), osd_, SLOT(MagnatuneDownloadFinished(QStringList)));

  LastFMButtonVisibilityChanged(radio_model_->GetLastFMService()->AreButtonsVisible());

  // Connections to the saved streams service
  connect(RadioModel::Service<SavedRadio>(), SIGNAL(ShowAddStreamDialog()), SLOT(AddStream()));

#ifdef Q_OS_DARWIN
  mac::SetApplicationHandler(this);
#endif
  // Tray icon
  tray_icon_->SetupMenu(ui_->action_previous_track,
                        ui_->action_play_pause,
                        ui_->action_stop,
                        ui_->action_stop_after_this_track,
                        ui_->action_next_track,
                        ui_->action_love,
                        ui_->action_ban,
                        ui_->action_quit);
  connect(tray_icon_, SIGNAL(PlayPause()), player_, SLOT(PlayPause()));
  connect(tray_icon_, SIGNAL(ShowHide()), SLOT(ToggleShowHide()));
  connect(tray_icon_, SIGNAL(ChangeVolume(int)), SLOT(VolumeWheelEvent(int)));
  
#ifdef Q_OS_DARWIN
  // Add check for updates item to application menu.
  QAction* check_updates = ui_->menuTools->addAction(tr("Check for updates..."));
  check_updates->setMenuRole(QAction::ApplicationSpecificRole);
  connect(check_updates, SIGNAL(triggered(bool)), SLOT(CheckForUpdates()));

  // Force this menu to be the app "Preferences".
  ui_->action_configure->setMenuRole(QAction::PreferencesRole);
  // Force this menu to be the app "About".
  ui_->action_about->setMenuRole(QAction::AboutRole);
#endif

  // Global shortcuts
  connect(global_shortcuts_, SIGNAL(Play()), player_, SLOT(Play()));
  connect(global_shortcuts_, SIGNAL(Pause()), player_, SLOT(Pause()));
  connect(global_shortcuts_, SIGNAL(PlayPause()), ui_->action_play_pause, SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(Stop()), ui_->action_stop, SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(StopAfter()), ui_->action_stop_after_this_track, SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(Next()), ui_->action_next_track, SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(Previous()), ui_->action_previous_track, SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(IncVolume()), player_, SLOT(VolumeUp()));
  connect(global_shortcuts_, SIGNAL(DecVolume()), player_, SLOT(VolumeDown()));
  connect(global_shortcuts_, SIGNAL(Mute()), player_, SLOT(Mute()));
  connect(global_shortcuts_, SIGNAL(SeekForward()), player_, SLOT(SeekForward()));
  connect(global_shortcuts_, SIGNAL(SeekBackward()), player_, SLOT(SeekBackward()));
  connect(global_shortcuts_, SIGNAL(ShowHide()), SLOT(ToggleShowHide()));
  connect(global_shortcuts_, SIGNAL(ShowOSD()), player_, SLOT(ShowOSD()));

  // Lyrics
  lyric_view_->set_network(network);

  // Analyzer
  ui_->analyzer->SetEngine(player_->GetEngine());
  ui_->analyzer->SetActions(ui_->action_visualisations);

  // Equalizer
  connect(equalizer_.get(), SIGNAL(ParametersChanged(int,QList<int>)),
          player_->GetEngine(), SLOT(SetEqualizerParameters(int,QList<int>)));
  connect(equalizer_.get(), SIGNAL(EnabledChanged(bool)),
          player_->GetEngine(), SLOT(SetEqualizerEnabled(bool)));
  player_->GetEngine()->SetEqualizerEnabled(equalizer_->is_enabled());
  player_->GetEngine()->SetEqualizerParameters(
      equalizer_->preamp_value(), equalizer_->gain_values());

  // Statusbar widgets
  ui_->playlist_summary->setMinimumWidth(QFontMetrics(font()).width("WW selected of WW tracks - [ WW:WW ]"));
  ui_->status_bar_stack->setCurrentWidget(ui_->playlist_summary_page);
  connect(ui_->multi_loading_indicator, SIGNAL(TaskCountChange(int)), SLOT(TaskCountChanged(int)));

  // Now playing widget
  ui_->now_playing->set_network(network);
  ui_->now_playing->set_ideal_height(ui_->status_bar->sizeHint().height() +
                                     ui_->player_controls->sizeHint().height() +
                                     1); // Don't question the 1
  connect(playlists_, SIGNAL(CurrentSongChanged(Song)), ui_->now_playing, SLOT(NowPlaying(Song)));
  connect(player_, SIGNAL(Stopped()), ui_->now_playing, SLOT(Stopped()));
  connect(ui_->now_playing, SIGNAL(ShowAboveStatusBarChanged(bool)),
          SLOT(NowPlayingWidgetPositionChanged(bool)));
  connect(ui_->action_hypnotoad, SIGNAL(toggled(bool)), ui_->now_playing, SLOT(AllHail(bool)));
  NowPlayingWidgetPositionChanged(ui_->now_playing->show_above_status_bar());

  // Load theme
  StyleSheetLoader* css_loader = new StyleSheetLoader(this);
  css_loader->SetStyleSheet(this, ":mainwindow.css");

  // Load playlists
  playlists_->Init(library_->backend(), playlist_backend_, ui_->playlist_sequence);

  // Load settings
  settings_.beginGroup(kSettingsGroup);

  restoreGeometry(settings_.value("geometry").toByteArray());
  if (!ui_->splitter->restoreState(settings_.value("splitter_state").toByteArray())) {
    ui_->splitter->setSizes(QList<int>() << 200 << width() - 200);
  }
  ui_->tabs->setCurrentIndex(settings_.value("current_tab", 0).toInt());
  file_view_->SetPath(settings_.value("file_path", QDir::homePath()).toString());

  ReloadSettings();

#ifndef Q_OS_DARWIN
  StartupBehaviour behaviour =
      StartupBehaviour(settings_.value("startupbehaviour", Startup_Remember).toInt());
  bool hidden = settings_.value("hidden", false).toBool();

  switch (behaviour) {
    case Startup_AlwaysHide: hide(); break;
    case Startup_AlwaysShow: show(); break;
    case Startup_Remember:   setVisible(!hidden); break;
  }

  // Force the window to show in case somehow the config has tray and window set to hide
  if (hidden && !tray_icon_->IsVisible()) {
    settings_.setValue("hidden", false);
    show();
  }
#else  // Q_OS_DARWIN
  // Always show mainwindow on startup on OS X.
  show();
#endif

  QShortcut* close_window_shortcut = new QShortcut(this);
  close_window_shortcut->setKey(Qt::CTRL + Qt::Key_W);
  connect(close_window_shortcut, SIGNAL(activated()), SLOT(SetHiddenInTray()));

  library_->Init();
  library_->StartThreads();

#ifdef ENABLE_WIIMOTEDEV
// http://code.google.com/p/clementine-player/issues/detail?id=670
// Switched position, mayby something is not ready ?

  wiimotedev_shortcuts_.reset(new WiimotedevShortcuts(osd_, this, player_));
#endif

}

MainWindow::~MainWindow() {
  SaveGeometry();
  delete ui_;

  // It's important that the device manager is deleted before the database.
  // Deleting the database deletes all objects that have been created in its
  // thread, including some device library backends.
  delete devices_; devices_ = NULL;
}

void MainWindow::ReloadSettings() {
#ifndef Q_OS_DARWIN
  bool show_tray = settings_.value("showtray", true).toBool();

  tray_icon_->SetVisible(show_tray);
  if (!show_tray && !isVisible())
    show();
#endif

  QSettings library_settings;
  library_settings.beginGroup(LibraryView::kSettingsGroup);

  autoclear_playlist_ = library_settings.value("autoclear_playlist", false).toBool();
}

void MainWindow::AddFilesToPlaylist(const QList<QUrl>& urls) {
  AddFilesToPlaylist(false, urls);
}

void MainWindow::LoadFilesToPlaylist(const QList<QUrl>& urls) {
  AddFilesToPlaylist(true, urls);
}

void MainWindow::FilesDoubleClicked(const QList<QUrl>& urls) {
  AddFilesToPlaylist(autoclear_playlist_, urls);
}

void MainWindow::AddFilesToPlaylist(bool clear_first, const QList<QUrl>& urls) {
  if (clear_first)
    playlists_->ClearCurrent();

  AddUrls(player_->GetState() != Engine::Playing, urls);
}

void MainWindow::AddUrls(bool play_now, const QList<QUrl> &urls) {
  SongLoaderInserter* inserter =
      new SongLoaderInserter(task_manager_, library_->backend(), this);
  connect(inserter, SIGNAL(Error(QString)), SLOT(ShowErrorDialog(QString)));
  connect(inserter, SIGNAL(PlayRequested(QModelIndex)), SLOT(PlayIndex(QModelIndex)));

  inserter->Load(playlists_->current(), -1, play_now, urls);
}

void MainWindow::AddLibrarySongsToPlaylist(const SongList &songs) {
  AddLibrarySongsToPlaylist(false, songs);
}

void MainWindow::LoadLibrarySongsToPlaylist(const SongList &songs) {
  AddLibrarySongsToPlaylist(true, songs);
}

void MainWindow::LibrarySongsDoubleClicked(const SongList &songs) {
  AddLibrarySongsToPlaylist(autoclear_playlist_, songs);
}

void MainWindow::AddLibrarySongsToPlaylist(bool clear_first, const SongList &songs) {
  if (clear_first)
    playlists_->ClearCurrent();

  QModelIndex first_song = playlists_->current()->InsertLibraryItems(songs);

  if (!playlists_->current()->proxy()->mapFromSource(first_song).isValid()) {
    // The first song doesn't match the filter, so don't play it
    return;
  }

  if (first_song.isValid() && player_->GetState() != Engine::Playing) {
    playlists_->SetActiveToCurrent();
    player_->PlayAt(first_song.row(), Engine::First, true);
  }
}

void MainWindow::AddDeviceSongsToPlaylist(const SongList &songs) {
  AddDeviceSongsToPlaylist(false, songs);
}

void MainWindow::LoadDeviceSongsToPlaylist(const SongList &songs) {
  AddDeviceSongsToPlaylist(true, songs);
}

void MainWindow::DeviceSongsDoubleClicked(const SongList &songs) {
  AddDeviceSongsToPlaylist(autoclear_playlist_, songs);
}

void MainWindow::AddDeviceSongsToPlaylist(bool clear_first, const SongList &songs) {
  if (clear_first)
    playlists_->ClearCurrent();

  QModelIndex first_song = playlists_->current()->InsertSongs(songs);

  if (!playlists_->current()->proxy()->mapFromSource(first_song).isValid()) {
    // The first song doesn't match the filter, so don't play it
    return;
  }

  if (first_song.isValid() && player_->GetState() != Engine::Playing) {
    playlists_->SetActiveToCurrent();
    player_->PlayAt(first_song.row(), Engine::First, true);
  }
}

void MainWindow::MediaStopped() {
  ui_->action_stop->setEnabled(false);
  ui_->action_stop_after_this_track->setEnabled(false);
  ui_->action_play_pause->setIcon(IconLoader::Load("media-playback-start"));
  ui_->action_play_pause->setText(tr("Play"));

  ui_->action_play_pause->setEnabled(true);

  ui_->action_ban->setEnabled(false);
  ui_->action_love->setEnabled(false);

  track_position_timer_->stop();
  ui_->track_slider->SetStopped();
  tray_icon_->SetProgress(0);
  tray_icon_->SetStopped();
}

void MainWindow::MediaPaused() {
  ui_->action_stop->setEnabled(true);
  ui_->action_stop_after_this_track->setEnabled(true);
  ui_->action_play_pause->setIcon(IconLoader::Load("media-playback-start"));
  ui_->action_play_pause->setText(tr("Play"));

  ui_->action_play_pause->setEnabled(true);

  track_position_timer_->stop();

  tray_icon_->SetPaused();
}

void MainWindow::MediaPlaying() {
  ui_->action_stop->setEnabled(true);
  ui_->action_stop_after_this_track->setEnabled(true);
  ui_->action_play_pause->setIcon(IconLoader::Load("media-playback-pause"));
  ui_->action_play_pause->setText(tr("Pause"));

  ui_->action_play_pause->setEnabled(
      ! (player_->GetCurrentItem()->options() & PlaylistItem::PauseDisabled));

  bool is_lastfm = (player_->GetCurrentItem()->options() & PlaylistItem::LastFMControls);
  LastFMService* lastfm = radio_model_->GetLastFMService();

  ui_->action_ban->setEnabled(lastfm->IsScrobblingEnabled() && is_lastfm);
  ui_->action_love->setEnabled(lastfm->IsScrobblingEnabled());

  ui_->track_slider->SetCanSeek(!is_lastfm);

  track_position_timer_->start();
  UpdateTrackPosition();

  tray_icon_->SetPlaying();
}

void MainWindow::ScrobblingEnabledChanged(bool value) {
  if (!player_->GetState() == Engine::Idle)
    return;

  bool is_lastfm = (player_->GetCurrentItem()->options() & PlaylistItem::LastFMControls);
  ui_->action_ban->setEnabled(value && is_lastfm);
  ui_->action_love->setEnabled(value);
}

void MainWindow::LastFMButtonVisibilityChanged(bool value) {
  ui_->action_ban->setVisible(value);
  ui_->action_love->setVisible(value);
  ui_->last_fm_controls->setVisible(value);
}

void MainWindow::resizeEvent(QResizeEvent*) {
  SaveGeometry();
}

void MainWindow::SaveGeometry() {
  settings_.setValue("geometry", saveGeometry());
  settings_.setValue("splitter_state", ui_->splitter->saveState());
  settings_.setValue("current_tab", ui_->tabs->currentIndex());
}

void MainWindow::PlayIndex(const QModelIndex& index) {
  if (!index.isValid())
    return;

  int row = index.row();
  if (index.model() == playlists_->current()->proxy()) {
    // The index was in the proxy model (might've been filtered), so we need
    // to get the actual row in the source model.
    row = playlists_->current()->proxy()->mapToSource(index).row();
  }

  playlists_->SetActiveToCurrent();
  player_->PlayAt(row, Engine::Manual, true);
}

void MainWindow::LoadLibraryItemToPlaylist(const QModelIndexList& indexes) {
  AddLibraryItemToPlaylist(true, indexes);
}

void MainWindow::AddLibraryItemToPlaylist(const QModelIndexList& indexes) {
  AddLibraryItemToPlaylist(false, indexes);
}

void MainWindow::LibraryItemDoubleClicked(const QModelIndex &index) {
  AddLibraryItemToPlaylist(autoclear_playlist_, QModelIndexList() << index);
}

void MainWindow::AddLibraryItemToPlaylist(bool clear_first, const QModelIndexList& indexes) {
  QModelIndexList source_indexes;
  foreach (const QModelIndex& index, indexes) {
    if (index.model() == library_sort_model_)
      source_indexes << library_sort_model_->mapToSource(index);
    else
      source_indexes << index;
  }

  AddLibrarySongsToPlaylist(clear_first, library_->model()->GetChildSongs(source_indexes));
}

void MainWindow::VolumeWheelEvent(int delta) {
  ui_->volume->setValue(ui_->volume->value() + delta / 30);
}

void MainWindow::ToggleShowHide() {
  if (settings_.value("hidden").toBool()) {
    show();
    SetHiddenInTray(false);
  } else if (isActiveWindow()) {
    hide();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    SetHiddenInTray(true);
  } else if (isMinimized()) {
    hide();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    SetHiddenInTray(false);
  } else if (!isVisible())
  {
    show();
    activateWindow();
  } else {
    // Window is not hidden but does not have focus; bring it to front.
    activateWindow();
    raise();
  }
}

void MainWindow::StopAfterCurrent() {
  playlists_->current()->StopAfter(playlists_->current()->current_index());
}

/**
  * Exit if the tray icon is not visible, otherwise ignore and set hidden in tray.
  * On OS X, never quit when the main window is closed. This is equivalent to hiding in the tray.
  */
void MainWindow::closeEvent(QCloseEvent* event) {
#ifndef Q_OS_DARWIN
  if (tray_icon_->IsVisible() && event->spontaneous()) {
    event->ignore();
    SetHiddenInTray(true);
  } else {
    QApplication::quit();
  }
#endif
}

void MainWindow::SetHiddenInTray(bool hidden) {
  settings_.setValue("hidden", hidden);

  // Some window managers don't remember maximized state between calls to
  // hide() and show(), so we have to remember it ourself.
  if (hidden) {
    was_maximized_ = isMaximized();
    hide();
  } else {
    if (was_maximized_)
      showMaximized();
    else
      show();
  }
}

void MainWindow::FilePathChanged(const QString& path) {
  settings_.setValue("file_path", path);
}

void MainWindow::UpdateTrackPosition() {
  // Track position in seconds
  const int position = std::floor(float(player_->GetEngine()->position()) / 1000.0 + 0.5);
  const int length = player_->GetCurrentItem()->Metadata().length();

  if (length <= 0) {
    // Probably a stream that we don't know the length of
    ui_->track_slider->SetStopped();
    tray_icon_->SetProgress(0);
    return;
  }

  // Time to scrobble?
  LastFMService* lastfm = radio_model_->GetLastFMService();

  if (!playlists_->active()->has_scrobbled() &&
      position >= playlists_->active()->scrobble_point()) {
    lastfm->Scrobble();
    playlists_->active()->set_scrobbled(true);
  }

  // Update the slider
  ui_->track_slider->SetValue(position, length);

  // Update the tray icon every 10 seconds
  if (position % 10 == 1) {
    tray_icon_->SetProgress(double(position) / length * 100);
  }
}

void MainWindow::Love() {
  radio_model_->GetLastFMService()->Love();
  ui_->action_love->setEnabled(false);
}

void MainWindow::RadioDoubleClick(const QModelIndex& index) {
  if (autoclear_playlist_)
    playlists_->ClearCurrent();

  scoped_ptr<QMimeData> data(
      radio_model_->merged_model()->mimeData(QModelIndexList() << index));
  if (!data)
    return;

  if (player_->GetState() != Engine::Playing)
    data->setData(Playlist::kPlayNowMimetype, "true");

  int rows = playlists_->current()->rowCount();
  playlists_->current()->dropMimeData(data.get(), Qt::CopyAction, -1, 0, QModelIndex());

  if (player_->GetState() != Engine::Playing && playlists_->current()->rowCount() > rows) {
    QModelIndex first_song = playlists_->current()->index(rows, 0);
    if (!playlists_->current()->proxy()->mapFromSource(first_song).isValid()) {
      // The first song doesn't match the filter, so don't play it
      return;
    }

    playlists_->SetActiveToCurrent();
    player_->PlayAt(first_song.row(), Engine::First, true);
  }
}

void MainWindow::InsertRadioItem(RadioItem* item) {
  QModelIndex first_song = playlists_->current()->InsertRadioStations(
      QList<RadioItem*>() << item);

  if (!playlists_->current()->proxy()->mapFromSource(first_song).isValid()) {
    // The first song doesn't match the filter, so don't play it
    return;
  }

  if (first_song.isValid() && player_->GetState() != Engine::Playing) {
    playlists_->SetActiveToCurrent();
    player_->PlayAt(first_song.row(), Engine::First, true);
  }
}

void MainWindow::InsertRadioItems(const PlaylistItemList& items) {
  QModelIndex first_song = playlists_->current()->InsertItems(items);

  if (!playlists_->current()->proxy()->mapFromSource(first_song).isValid()) {
    // The first song doesn't match the filter, so don't play it
    return;
  }

  if (first_song.isValid() && player_->GetState() != Engine::Playing) {
    playlists_->SetActiveToCurrent();
    player_->PlayAt(first_song.row(), Engine::First, true);
  }
}

void MainWindow::PlaylistRightClick(const QPoint& global_pos, const QModelIndex& index) {
  playlist_menu_index_ = index;

  QModelIndex source_index = playlists_->current()->proxy()->mapToSource(index);

  // Is this song currently playing?
  if (playlists_->current()->current_index() == source_index.row() && player_->GetState() == Engine::Playing) {
    playlist_play_pause_->setText(tr("Pause"));
    playlist_play_pause_->setIcon(IconLoader::Load("media-playback-pause"));
  } else {
    playlist_play_pause_->setText(tr("Play"));
    playlist_play_pause_->setIcon(IconLoader::Load("media-playback-start"));
  }

  // Are we allowed to pause?
  if (index.isValid()) {
    playlist_play_pause_->setEnabled(
        playlists_->current()->current_index() != source_index.row() ||
        ! (playlists_->current()->item_at(source_index.row())->options() & PlaylistItem::PauseDisabled));
  } else {
    playlist_play_pause_->setEnabled(false);
  }

  playlist_stop_after_->setEnabled(index.isValid());

  // Are any of the selected songs editable or queued?
  QModelIndexList selection = ui_->playlist->view()->selectionModel()->selection().indexes();
  int editable = 0;
  int in_queue = 0;
  int not_in_queue = 0;
  foreach (const QModelIndex& index, selection) {
    if (index.column() != 0)
      continue;
    if (playlists_->current()->item_at(index.row())->Metadata().IsEditable()) {
      editable++;
    }

    if (index.data(Playlist::Role_QueuePosition).toInt() == -1)
      not_in_queue ++;
    else
      in_queue ++;
  }
  ui_->action_edit_track->setEnabled(editable);

  bool track_column = (index.column() == Playlist::Column_Track);
  ui_->action_renumber_tracks->setVisible(editable >= 2 && track_column);
  ui_->action_selection_set_value->setVisible(editable >= 2 && !track_column);
  ui_->action_edit_value->setVisible(editable);
  ui_->action_remove_from_playlist->setEnabled(!selection.isEmpty());

  playlist_copy_to_library_->setVisible(false);
  playlist_move_to_library_->setVisible(false);
  playlist_organise_->setVisible(false);
  playlist_delete_->setVisible(false);

  if (in_queue == 1 && not_in_queue == 0)
    playlist_queue_->setText(tr("Dequeue track"));
  else if (in_queue > 1 && not_in_queue == 0)
    playlist_queue_->setText(tr("Dequeue selected tracks"));
  else if (in_queue == 0 && not_in_queue == 1)
    playlist_queue_->setText(tr("Queue track"));
  else if (in_queue == 0 && not_in_queue > 1)
    playlist_queue_->setText(tr("Queue selected tracks"));
  else
    playlist_queue_->setText(tr("Toggle queue status"));

  if (not_in_queue == 0)
    playlist_queue_->setIcon(IconLoader::Load("go-previous"));
  else
    playlist_queue_->setIcon(IconLoader::Load("go-next"));

  if (!index.isValid()) {
    ui_->action_selection_set_value->setVisible(false);
    ui_->action_edit_value->setVisible(false);
  } else {
    Playlist::Column column = (Playlist::Column)index.column();
    bool column_is_editable = Playlist::column_is_editable(column);

    ui_->action_selection_set_value->setVisible(
        ui_->action_selection_set_value->isVisible() && column_is_editable);
    ui_->action_edit_value->setVisible(
        ui_->action_edit_value->isVisible() && column_is_editable);

    QString column_name = Playlist::column_name(column);
    QString column_value = playlists_->current()->data(source_index).toString();
    if (column_value.length() > 25)
      column_value = column_value.left(25) + "...";

    ui_->action_selection_set_value->setText(tr("Set %1 to \"%2\"...")
             .arg(column_name.toLower()).arg(column_value));
    ui_->action_edit_value->setText(tr("Edit tag \"%1\"...").arg(column_name));

    // Is it a library item?
    if (playlists_->current()->item_at(source_index.row())->type() == "Library") {
      playlist_organise_->setVisible(editable);
    } else {
      playlist_copy_to_library_->setVisible(editable);
      playlist_move_to_library_->setVisible(editable);
    }
  }

  playlist_menu_->popup(global_pos);
}

void MainWindow::PlaylistPlay() {
  if (playlists_->current()->current_index() == playlist_menu_index_.row()) {
    player_->PlayPause();
  } else {
    PlayIndex(playlist_menu_index_);
  }
}

void MainWindow::PlaylistStopAfter() {
  playlists_->current()->StopAfter(
      playlists_->current()->proxy()->mapToSource(playlist_menu_index_).row());
}

void MainWindow::EditTracks() {
  SongList songs;
  QList<int> rows;

  foreach (const QModelIndex& index,
           ui_->playlist->view()->selectionModel()->selection().indexes()) {
    if (index.column() != 0)
      continue;
    int row = playlists_->current()->proxy()->mapToSource(index).row();
    Song song = playlists_->current()->item_at(row)->Metadata();

    if (song.IsEditable()) {
      songs << song;
      rows << index.row();
    }
  }

  EnsureEditTagDialogCreated();
  edit_tag_dialog_->SetSongs(songs);
  edit_tag_dialog_->SetTagCompleter(library_->model()->backend());

  if (edit_tag_dialog_->exec() == QDialog::Rejected)
    return;

  playlists_->current()->ReloadItems(rows);
}

void MainWindow::RenumberTracks() {
  QModelIndexList indexes=ui_->playlist->view()->selectionModel()->selection().indexes();
  int track=1;

  // Get the index list in order
  qStableSort(indexes);

  // if first selected song has a track number set, start from that offset
  if (indexes.size()) {
    Song first_song=playlists_->current()->item_at(indexes[0].row())->Metadata();
    if (int first_track = first_song.track())
      track = first_track;
  }

  foreach (const QModelIndex& index, indexes) {
    if (index.column() != 0)
      continue;

    int row = playlists_->current()->proxy()->mapToSource(index).row();
    Song song = playlists_->current()->item_at(row)->Metadata();

    if (song.IsEditable()) {
      song.set_track(track);
      QFuture<bool> future = song.BackgroundSave();
      ModelFutureWatcher<bool>* watcher = new ModelFutureWatcher<bool>(index, this);
      watcher->setFuture(future);
      connect(watcher, SIGNAL(finished()), SLOT(SongSaveComplete()));
    }
    track++;
  }
}

void MainWindow::SongSaveComplete() {
  ModelFutureWatcher<bool>* watcher = static_cast<ModelFutureWatcher<bool>*>(sender());
  watcher->deleteLater();
  if (watcher->index().isValid()) {
    playlists_->current()->item_at(watcher->index().row())->Reload();
  }
}

void MainWindow::SelectionSetValue() {
  Playlist::Column column = (Playlist::Column)playlist_menu_index_.column();
  QVariant column_value = playlists_->current()->data(playlist_menu_index_);

  QModelIndexList indexes=ui_->playlist->view()->selectionModel()->selection().indexes();
  foreach (const QModelIndex& index, indexes) {
    if (index.column() != 0)
      continue;

    int row = playlists_->current()->proxy()->mapToSource(index).row();
    Song song = playlists_->current()->item_at(row)->Metadata();

    if(Playlist::set_column_value(song, column, column_value)) {
      QFuture<bool> future = song.BackgroundSave();
      ModelFutureWatcher<bool>* watcher = new ModelFutureWatcher<bool>(index, this);
      watcher->setFuture(future);
      connect(watcher, SIGNAL(finished()), SLOT(SongSaveComplete()));
    }
  }
}

void MainWindow::EditValue() {
  QModelIndex current = ui_->playlist->view()->currentIndex();
  if (!current.isValid())
    return;

  // Edit the last column that was right-clicked on.  If nothing's ever been
  // right clicked then look for the first editable column.
  int column = playlist_menu_index_.column();
  if (column == -1) {
    for (int i=0 ; i<ui_->playlist->view()->model()->columnCount() ; ++i) {
      if (ui_->playlist->view()->isColumnHidden(i))
        continue;
      if (!Playlist::column_is_editable(Playlist::Column(i)))
        continue;
      column = i;
      break;
    }
  }

  ui_->playlist->view()->edit(current.sibling(current.row(), column));
}

void MainWindow::AddFile() {
  // Last used directory
  QString directory = settings_.value("add_media_path", QDir::currentPath()).toString();

  PlaylistParser parser;

  // Show dialog
  QStringList file_names = QFileDialog::getOpenFileNames(
      this, tr("Add media"), directory,
      QString("%1;;%2;;%3").arg(tr(kMusicFilterSpec), parser.filters(),
                                tr(kAllFilesFilterSpec)));
  if (file_names.isEmpty())
    return;

  // Save last used directory
  settings_.setValue("add_media_path", file_names[0]);

  // Convert to URLs
  QList<QUrl> urls;
  foreach (const QString& path, file_names) {
    urls << QUrl::fromLocalFile(path);
  }
  AddUrls(player_->GetState() != Engine::Playing, urls);
}

void MainWindow::AddFolder() {
  // Last used directory
  QString directory = settings_.value("add_folder_path", QDir::currentPath()).toString();

  // Show dialog
  directory = QFileDialog::getExistingDirectory(this, tr("Add folder"), directory);
  if (directory.isEmpty())
    return;

  // Save last used directory
  settings_.setValue("add_folder_path", directory);

  // Add media
  AddUrls(player_->GetState() != Engine::Playing,
          QList<QUrl>() << QUrl::fromLocalFile(directory));
}

void MainWindow::AddStream() {
  if (!add_stream_dialog_) {
    add_stream_dialog_.reset(new AddStreamDialog);
    connect(add_stream_dialog_.get(), SIGNAL(accepted()), SLOT(AddStreamAccepted()));

    add_stream_dialog_->set_add_on_accept(RadioModel::Service<SavedRadio>());
  }

  add_stream_dialog_->show();
}

void MainWindow::AddStreamAccepted() {
  AddUrls(player_->GetState() != Engine::Playing,
          QList<QUrl>() << add_stream_dialog_->url());
}

void MainWindow::PlaylistRemoveCurrent() {
  ui_->playlist->view()->RemoveSelected();
}

void MainWindow::PlaylistEditFinished(const QModelIndex& index) {
  if (index == playlist_menu_index_)
    SelectionSetValue();
}

void MainWindow::CommandlineOptionsReceived(const QByteArray& serialized_options) {
  if (serialized_options == "wake up!") {
    // Old versions of Clementine sent this - just ignore it
    return;
  }

  CommandlineOptions options;
  options.Load(serialized_options);

  if (options.is_empty()) {
    show();
    activateWindow();
  }
  else
    CommandlineOptionsReceived(options);
}

void MainWindow::CommandlineOptionsReceived(const CommandlineOptions &options) {
  switch (options.player_action()) {
    case CommandlineOptions::Player_Play:
      player_->Play();
      break;
    case CommandlineOptions::Player_PlayPause:
      player_->PlayPause();
      break;
    case CommandlineOptions::Player_Pause:
      player_->Pause();
      break;
    case CommandlineOptions::Player_Stop:
      player_->Stop();
      break;
    case CommandlineOptions::Player_Previous:
      player_->Previous();
      break;
    case CommandlineOptions::Player_Next:
      player_->Next();
      break;

    case CommandlineOptions::Player_None:
      break;
  }

  switch (options.url_list_action()) {
    case CommandlineOptions::UrlList_Load:
      playlists_->ClearCurrent();

      // fallthrough
    case CommandlineOptions::UrlList_Append:
      AddUrls(player_->GetState() != Engine::Playing, options.urls());
      break;
  }

  if (options.set_volume() != -1)
    player_->SetVolume(options.set_volume());

  if (options.volume_modifier() != 0)
    player_->SetVolume(player_->GetVolume() + options.volume_modifier());

  if (options.seek_to() != -1)
    player_->Seek(options.seek_to());
  else if (options.seek_by() != 0)
    player_->Seek(player_->PositionGet() / 1000 + options.seek_by());

  if (options.play_track_at() != -1)
    player_->PlayAt(options.play_track_at(), Engine::Manual, true);

  if (options.show_osd())
    player_->ShowOSD();
}

void MainWindow::ForceShowOSD(const Song &song) {
  osd_->ForceShowNextNotification();
  osd_->SongChanged(song);
}

void MainWindow::Activate() {
  show();
}

bool MainWindow::LoadUrl(const QString& url) {
  if (!QFile::exists(url))
    return false;

  QList<QUrl> urls;
  urls << QUrl::fromLocalFile(url);
  AddUrls(true, urls);  // Always play now as this was a direct request from Finder.
  return true;
}

void MainWindow::CheckForUpdates() {
#ifdef Q_OS_DARWIN
  mac::CheckForUpdates();
#endif
}

void MainWindow::PlaylistUndoRedoChanged(QAction *undo, QAction *redo) {
  playlist_menu_->insertAction(playlist_undoredo_, undo);
  playlist_menu_->insertAction(playlist_undoredo_, redo);
}

void MainWindow::ShowLibraryConfig() {
  EnsureSettingsDialogCreated();
  settings_dialog_->OpenAtPage(SettingsDialog::Page_Library);
}

void MainWindow::TaskCountChanged(int count) {
  if (count == 0) {
    ui_->status_bar_stack->setCurrentWidget(ui_->playlist_summary_page);
  } else {
    ui_->status_bar_stack->setCurrentWidget(ui_->multi_loading_indicator);
  }
}

void MainWindow::NowPlayingWidgetPositionChanged(bool above_status_bar) {
  if (above_status_bar) {
    ui_->status_bar->setParent(ui_->centralWidget);
  } else {
    ui_->status_bar->setParent(ui_->player_controls_container);
  }

  ui_->status_bar->parentWidget()->layout()->addWidget(ui_->status_bar);
  ui_->status_bar->show();
}

void MainWindow::CopyFilesToLibrary(const QList<QUrl> &urls) {
  organise_dialog_->SetDestinationModel(library_->model()->directory_model());
  organise_dialog_->SetUrls(urls);
  organise_dialog_->SetCopy(true);
  organise_dialog_->show();
}

void MainWindow::MoveFilesToLibrary(const QList<QUrl> &urls) {
  organise_dialog_->SetDestinationModel(library_->model()->directory_model());
  organise_dialog_->SetUrls(urls);
  organise_dialog_->SetCopy(false);
  organise_dialog_->show();
}

void MainWindow::CopyFilesToDevice(const QList<QUrl> &urls) {
  organise_dialog_->SetDestinationModel(devices_->connected_devices_model(), true);
  organise_dialog_->SetCopy(true);
  if (organise_dialog_->SetUrls(urls))
    organise_dialog_->show();
  else {
    QMessageBox::warning(this, tr("Error"),
        tr("None of the selected songs were suitable for copying to a device"));
  }
}

void MainWindow::PlaylistCopyToLibrary() {
  PlaylistOrganiseSelected(true);
}

void MainWindow::PlaylistMoveToLibrary() {
  PlaylistOrganiseSelected(false);
}

void MainWindow::PlaylistOrganiseSelected(bool copy) {
  QModelIndexList proxy_indexes = ui_->playlist->view()->selectionModel()->selectedRows();
  SongList songs;

  foreach (const QModelIndex& proxy_index, proxy_indexes) {
    QModelIndex index = playlists_->current()->proxy()->mapToSource(proxy_index);

    songs << playlists_->current()->item_at(index.row())->Metadata();
  }

  organise_dialog_->SetDestinationModel(library_->model()->directory_model());
  organise_dialog_->SetSongs(songs);
  organise_dialog_->SetCopy(copy);
  organise_dialog_->show();
}

void MainWindow::PlaylistDelete() {
  // TODO
}

void MainWindow::PlaylistQueue() {
  QModelIndexList indexes;
  foreach (const QModelIndex& proxy_index,
           ui_->playlist->view()->selectionModel()->selectedRows()) {
    indexes << playlists_->current()->proxy()->mapToSource(proxy_index);
  }

  playlists_->current()->queue()->ToggleTracks(indexes);
}

void MainWindow::PlaylistCopyToDevice() {
  QModelIndexList proxy_indexes = ui_->playlist->view()->selectionModel()->selectedRows();
  SongList songs;

  foreach (const QModelIndex& proxy_index, proxy_indexes) {
    QModelIndex index = playlists_->current()->proxy()->mapToSource(proxy_index);

    songs << playlists_->current()->item_at(index.row())->Metadata();
  }

  organise_dialog_->SetDestinationModel(devices_->connected_devices_model(), true);
  organise_dialog_->SetCopy(true);
  if (organise_dialog_->SetSongs(songs))
    organise_dialog_->show();
  else {
    QMessageBox::warning(this, tr("Error"),
        tr("None of the selected songs were suitable for copying to a device"));
  }
}

void MainWindow::ShowCoverManager() {
  if (!cover_manager_) {
    cover_manager_.reset(new AlbumCoverManager(network_, library_->backend()));
    cover_manager_->Init();

    // Cover manager connections
    connect(cover_manager_.get(), SIGNAL(AddSongsToPlaylist(SongList)), SLOT(AddLibrarySongsToPlaylist(SongList)));
    connect(cover_manager_.get(), SIGNAL(LoadSongsToPlaylist(SongList)), SLOT(LoadLibrarySongsToPlaylist(SongList)));
    connect(cover_manager_.get(), SIGNAL(SongsDoubleClicked(SongList)), SLOT(LibrarySongsDoubleClicked(SongList)));
  }

  cover_manager_->show();
}

void MainWindow::EnsureSettingsDialogCreated() {
  if (settings_dialog_)
    return;

  settings_dialog_.reset(new SettingsDialog);
  settings_dialog_->SetLibraryDirectoryModel(library_->model()->directory_model());

#ifdef HAVE_GSTREAMER
  if (GstEngine* engine = qobject_cast<GstEngine*>(player_->GetEngine())) {
    settings_dialog_->SetGstEngine(engine);
  }
#endif

  settings_dialog_->SetGlobalShortcutManager(global_shortcuts_);
  settings_dialog_->SetLyricFetcher(lyric_view_->fetcher());

  // Settings
  connect(settings_dialog_.get(), SIGNAL(accepted()), SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), library_, SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), player_, SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), osd_, SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), library_view_, SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), player_->GetEngine(), SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), ui_->playlist->view(), SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), lyric_view_->fetcher(), SLOT(ReloadSettings()));
#ifdef ENABLE_WIIMOTEDEV
  connect(settings_dialog_.get(), SIGNAL(accepted()), wiimotedev_shortcuts_.get(), SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(SetWiimotedevInterfaceActived(bool)), wiimotedev_shortcuts_.get(), SLOT(SetWiimotedevInterfaceActived(bool)));
#endif
}

void MainWindow::OpenSettingsDialog() {
  EnsureSettingsDialogCreated();
  settings_dialog_->show();
}

void MainWindow::OpenSettingsDialogAtPage(SettingsDialog::Page page) {
  EnsureSettingsDialogCreated();
  settings_dialog_->OpenAtPage(page);
}

void MainWindow::EnsureEditTagDialogCreated() {
  if (edit_tag_dialog_)
    return;

  edit_tag_dialog_.reset(new EditTagDialog);
}

void MainWindow::ShowAboutDialog() {
  if (!about_dialog_) {
    about_dialog_.reset(new About);
  }

  about_dialog_->show();
}

void MainWindow::ShowTranscodeDialog() {
#ifdef HAVE_GSTREAMER
  if (!transcode_dialog_) {
    transcode_dialog_.reset(new TranscodeDialog);
  }
  transcode_dialog_->show();
#endif
}

void MainWindow::ShowErrorDialog(const QString& message) {
  if (!error_dialog_) {
    error_dialog_.reset(new ErrorDialog);
  }
  error_dialog_->ShowMessage(message);
}

void MainWindow::ShowQueueManager() {
  if (!queue_manager_) {
    queue_manager_.reset(new QueueManager);
    queue_manager_->SetPlaylistManager(playlists_);
  }
  queue_manager_->show();
}

void MainWindow::ShowVisualisations() {
#ifdef ENABLE_VISUALISATIONS
  if (!visualisation_) {
    visualisation_.reset(new VisualisationContainer);

    visualisation_->SetActions(ui_->action_previous_track, ui_->action_play_pause,
                               ui_->action_stop, ui_->action_next_track);
    connect(player_, SIGNAL(Stopped()), visualisation_.get(), SLOT(Stopped()));
    connect(player_, SIGNAL(ForceShowOSD(Song)), visualisation_.get(), SLOT(SongMetadataChanged(Song)));
    connect(playlists_, SIGNAL(CurrentSongChanged(Song)), visualisation_.get(), SLOT(SongMetadataChanged(Song)));

#ifdef HAVE_GSTREAMER
    if (GstEngine* engine = qobject_cast<GstEngine*>(player_->GetEngine()))
    visualisation_->SetEngine(engine);
#endif
  }

  visualisation_->show();
#endif // ENABLE_VISUALISATIONS
}

void MainWindow::AddFancyTab(QWidget* widget, const QIcon& icon, const QString& label) {
  const int i = ui_->tabs->count();
  ui_->tabs->addTab(widget, icon, label);
  ui_->tabs->setTabEnabled(i, true);
}
