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
#include "ui_mainwindow.h"
#include "core/commandlineoptions.h"
#include "core/database.h"
#include "core/globalshortcuts.h"
#include "core/mac_startup.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "core/stylesheetloader.h"
#include "engines/enginebase.h"
#include "library/groupbydialog.h"
#include "library/libraryconfig.h"
#include "library/library.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#include "playlist/playlistview.h"
#include "playlist/songplaylistitem.h"
#include "playlistparsers/playlistparser.h"
#include "radio/lastfmservice.h"
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
#include "ui/globalshortcutsdialog.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"
#include "ui/systemtrayicon.h"
#include "widgets/errordialog.h"
#include "widgets/multiloadingindicator.h"
#include "widgets/osd.h"
#include "widgets/trackslider.h"

#ifdef HAVE_GSTREAMER
# include "engines/gstengine.h"
#endif

#ifdef ENABLE_VISUALISATIONS
# include "visualisations/visualisationcontainer.h"
#endif

#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QUndoStack>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QtDebug>
#include <QCloseEvent>
#include <QSignalMapper>
#include <QFileDialog>
#include <QTimer>
#include <QShortcut>

#include <cmath>

using boost::shared_ptr;
using boost::scoped_ptr;

#ifdef Q_OS_DARWIN
// Non exported mac-specific function.
void qt_mac_set_dock_menu(QMenu*);
#endif

const char* MainWindow::kSettingsGroup = "MainWindow";
const char* MainWindow::kMusicFilterSpec =
    QT_TR_NOOP("Music (*.mp3 *.ogg *.flac *.mpc *.m4a *.aac *.wma)");
const char* MainWindow::kPlaylistFilterSpec =
    QT_TR_NOOP("Playlists (*.m3u *.xspf *.xml)");
const char* MainWindow::kAllFilesFilterSpec =
    QT_TR_NOOP("All Files (*)");

MainWindow::MainWindow(NetworkAccessManager* network, Engine::Type engine, QWidget *parent)
  : QMainWindow(parent),
    ui_(new Ui_MainWindow),
    tray_icon_(new SystemTrayIcon(this)),
    osd_(new OSD(tray_icon_, network, this)),
    track_slider_(new TrackSlider(this)),
    playlist_sequence_(new PlaylistSequence(this)),
    edit_tag_dialog_(new EditTagDialog),
    multi_loading_indicator_(new MultiLoadingIndicator(this)),
    about_dialog_(new About),
    database_(new BackgroundThreadImplementation<Database, Database>(this)),
    radio_model_(NULL),
    playlist_backend_(NULL),
    playlists_(new PlaylistManager(this)),
    playlist_parser_(new PlaylistParser(this)),
    player_(NULL),
    library_(NULL),
    global_shortcuts_(new GlobalShortcuts(this)),
    settings_dialog_(NULL),
    add_stream_dialog_(new AddStreamDialog),
    cover_manager_(NULL),
    equalizer_(new Equalizer),
    transcode_dialog_(new TranscodeDialog),
    global_shortcuts_dialog_(new GlobalShortcutsDialog(global_shortcuts_)),
    error_dialog_(new ErrorDialog),
#ifdef ENABLE_VISUALISATIONS
    visualisation_(new VisualisationContainer),
#endif
    playlist_menu_(new QMenu(this)),
    library_sort_model_(new QSortFilterProxyModel(this)),
    track_position_timer_(new QTimer(this)),
    was_maximized_(false)
{
  // Wait for the database thread to start - lots of stuff depends on it.
  database_->Start(true);

  // Create some objects in the database thread
  playlist_backend_ = database_->CreateInThread<PlaylistBackend>();
  playlist_backend_->SetDatabase(database_->Worker());

  // Create stuff that needs the database
  radio_model_ = new RadioModel(database_, network, this);
  player_ = new Player(playlists_, radio_model_->GetLastFMService(), engine, this);
  library_ = new Library(database_, this);
  cover_manager_.reset(new AlbumCoverManager(network, library_->backend()));
  settings_dialog_.reset(new SettingsDialog); // Needs RadioModel
  radio_model_->SetSettingsDialog(settings_dialog_.get());

  // Initialise the UI
  ui_->setupUi(this);
  tray_icon_->setIcon(windowIcon());
  tray_icon_->setToolTip(QCoreApplication::applicationName());

  ui_->volume->setValue(player_->GetVolume());

  track_position_timer_->setInterval(1000);
  connect(track_position_timer_, SIGNAL(timeout()), SLOT(UpdateTrackPosition()));

  // Start initialising the player
  player_->Init();

#ifdef HAVE_GSTREAMER
  if (GstEngine* engine = qobject_cast<GstEngine*>(player_->GetEngine())) {
    settings_dialog_->SetGstEngine(engine);
#   ifdef ENABLE_VISUALISATIONS
      visualisation_->SetEngine(engine);
#   endif
  }
#endif // HAVE_GSTREAMER

  // Models
  library_sort_model_->setSourceModel(library_->model());
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  ui_->playlist->SetManager(playlists_);

  ui_->library_view->setModel(library_sort_model_);
  ui_->library_view->SetLibrary(library_->model());
  settings_dialog_->SetLibraryDirectoryModel(library_->model()->directory_model());

  ui_->radio_view->SetModel(radio_model_);

  cover_manager_->Init();

  // Icons
  ui_->action_about->setIcon(IconLoader::Load("help-about"));
  ui_->action_add_file->setIcon(IconLoader::Load("document-open"));
  ui_->action_add_folder->setIcon(IconLoader::Load("document-open-folder"));
  ui_->action_add_stream->setIcon(IconLoader::Load("document-open-remote"));
  ui_->action_clear_playlist->setIcon(IconLoader::Load("edit-clear-list"));
  ui_->action_configure->setIcon(IconLoader::Load("configure"));
  ui_->action_configure_global_shortcuts->setIcon(IconLoader::Load("configure-shortcuts"));
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

  // File view connections
  connect(ui_->file_view, SIGNAL(AddToPlaylist(QList<QUrl>)), SLOT(AddFilesToPlaylist(QList<QUrl>)));
  connect(ui_->file_view, SIGNAL(Load(QList<QUrl>)), SLOT(LoadFilesToPlaylist(QList<QUrl>)));
  connect(ui_->file_view, SIGNAL(DoubleClicked(QList<QUrl>)), SLOT(FilesDoubleClicked(QList<QUrl>)));
  connect(ui_->file_view, SIGNAL(PathChanged(QString)), SLOT(FilePathChanged(QString)));

  // Cover manager connections
  connect(cover_manager_.get(), SIGNAL(AddSongsToPlaylist(SongList)), SLOT(AddLibrarySongsToPlaylist(SongList)));
  connect(cover_manager_.get(), SIGNAL(LoadSongsToPlaylist(SongList)), SLOT(LoadLibrarySongsToPlaylist(SongList)));
  connect(cover_manager_.get(), SIGNAL(SongsDoubleClicked(SongList)), SLOT(LibrarySongsDoubleClicked(SongList)));

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
  connect(ui_->action_configure, SIGNAL(triggered()), settings_dialog_.get(), SLOT(show()));
  connect(ui_->action_about, SIGNAL(triggered()), about_dialog_.get(), SLOT(show()));
  connect(ui_->action_shuffle, SIGNAL(triggered()), playlists_, SLOT(ShuffleCurrent()));
  connect(ui_->action_open_media, SIGNAL(triggered()), SLOT(AddFile()));
  connect(ui_->action_add_file, SIGNAL(triggered()), SLOT(AddFile()));
  connect(ui_->action_add_folder, SIGNAL(triggered()), SLOT(AddFolder()));
  connect(ui_->action_add_stream, SIGNAL(triggered()), SLOT(AddStream()));
  connect(ui_->action_cover_manager, SIGNAL(triggered()), cover_manager_.get(), SLOT(show()));
  connect(ui_->action_equalizer, SIGNAL(triggered()), equalizer_.get(), SLOT(show()));
  connect(ui_->action_transcode, SIGNAL(triggered()), transcode_dialog_.get(), SLOT(show()));
  connect(ui_->action_configure_global_shortcuts, SIGNAL(triggered()), global_shortcuts_dialog_.get(), SLOT(show()));
  connect(ui_->action_jump, SIGNAL(triggered()), ui_->playlist->view(), SLOT(JumpToCurrentlyPlayingTrack()));
  connect(ui_->action_update_library, SIGNAL(triggered()), library_, SLOT(IncrementalScan()));

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
  visualisation_->SetActions(ui_->action_previous_track, ui_->action_play_pause,
                             ui_->action_stop, ui_->action_next_track);
  connect(ui_->action_visualisations, SIGNAL(triggered()), visualisation_.get(), SLOT(show()));
  connect(player_, SIGNAL(Stopped()), visualisation_.get(), SLOT(Stopped()));
  connect(player_, SIGNAL(ForceShowOSD(Song)), visualisation_.get(), SLOT(SongMetadataChanged(Song)));
  connect(playlists_, SIGNAL(CurrentSongChanged(Song)), visualisation_.get(), SLOT(SongMetadataChanged(Song)));
#else
  ui_->action_visualisations->setEnabled(false);
#endif

  // Add the shuffle and repeat action groups to the menu
  ui_->action_shuffle_mode->setMenu(playlist_sequence_->shuffle_menu());
  ui_->action_repeat_mode->setMenu(playlist_sequence_->repeat_menu());

  // Stop actions
  QMenu* stop_menu = new QMenu(this);
  stop_menu->addAction(ui_->action_stop);
  stop_menu->addAction(ui_->action_stop_after_this_track);
  ui_->stop_button->setMenu(stop_menu);

  // Player connections
  connect(ui_->volume, SIGNAL(valueChanged(int)), player_, SLOT(SetVolume(int)));

  connect(player_, SIGNAL(Error(QString)), error_dialog_.get(), SLOT(ShowMessage(QString)));
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
  connect(playlists_, SIGNAL(PlaylistChanged()), player_, SLOT(PlaylistChanged()));
  connect(playlists_, SIGNAL(EditingFinished(QModelIndex)), SLOT(PlaylistEditFinished(QModelIndex)));
  connect(playlists_, SIGNAL(Error(QString)), error_dialog_.get(), SLOT(ShowMessage(QString)));

  connect(ui_->playlist->view(), SIGNAL(doubleClicked(QModelIndex)), SLOT(PlayIndex(QModelIndex)));
  connect(ui_->playlist->view(), SIGNAL(PlayPauseItem(QModelIndex)), SLOT(PlayIndex(QModelIndex)));
  connect(ui_->playlist->view(), SIGNAL(RightClicked(QPoint,QModelIndex)), SLOT(PlaylistRightClick(QPoint,QModelIndex)));

  connect(track_slider_, SIGNAL(ValueChanged(int)), player_, SLOT(Seek(int)));

  // Database connections
  connect(database_->Worker().get(), SIGNAL(Error(QString)), error_dialog_.get(), SLOT(ShowMessage(QString)));

  // Library connections
  connect(ui_->library_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(LibraryItemDoubleClicked(QModelIndex)));
  connect(ui_->library_view, SIGNAL(Load(QModelIndexList)), SLOT(LoadLibraryItemToPlaylist(QModelIndexList)));
  connect(ui_->library_view, SIGNAL(AddToPlaylist(QModelIndexList)), SLOT(AddLibraryItemToPlaylist(QModelIndexList)));
  connect(ui_->library_view, SIGNAL(ShowConfigDialog()), SLOT(ShowLibraryConfig()));
  connect(library_->model(), SIGNAL(TotalSongCountUpdated(int)), ui_->library_view, SLOT(TotalSongCountUpdated(int)));
  connect(library_, SIGNAL(ScanStarted()), SLOT(LibraryScanStarted()));
  connect(library_, SIGNAL(ScanFinished()), SLOT(LibraryScanFinished()));

  // Library filter widget
  QAction* library_config_action = new QAction(
      IconLoader::Load("configure"), tr("Configure library..."), this);
  connect(library_config_action, SIGNAL(triggered()), SLOT(ShowLibraryConfig()));
  ui_->library_filter->SetSettingsGroup(kSettingsGroup);
  ui_->library_filter->SetLibraryModel(library_->model());
  ui_->library_filter->AddMenuAction(library_config_action);

  // Playlist menu
  playlist_play_pause_ = playlist_menu_->addAction(tr("Play"), this, SLOT(PlaylistPlay()));
  playlist_menu_->addAction(ui_->action_stop);
  playlist_stop_after_ = playlist_menu_->addAction(IconLoader::Load("media-playback-stop"), tr("Stop after this track"), this, SLOT(PlaylistStopAfter()));
  playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_remove_from_playlist);
  playlist_undoredo_ = playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_edit_track);
  playlist_menu_->addAction(ui_->action_edit_value);
  playlist_menu_->addAction(ui_->action_renumber_tracks);
  playlist_menu_->addAction(ui_->action_selection_set_value);
  playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_clear_playlist);
  playlist_menu_->addAction(ui_->action_shuffle);

  connect(ui_->playlist, SIGNAL(UndoRedoActionsChanged(QAction*,QAction*)),
          SLOT(PlaylistUndoRedoChanged(QAction*,QAction*)));

  // Radio connections
  connect(radio_model_, SIGNAL(TaskStarted(MultiLoadingIndicator::TaskType)), multi_loading_indicator_, SLOT(TaskStarted(MultiLoadingIndicator::TaskType)));
  connect(radio_model_, SIGNAL(TaskFinished(MultiLoadingIndicator::TaskType)), multi_loading_indicator_, SLOT(TaskFinished(MultiLoadingIndicator::TaskType)));
  connect(radio_model_, SIGNAL(StreamError(QString)), error_dialog_.get(), SLOT(ShowMessage(QString)));
  connect(radio_model_, SIGNAL(AsyncLoadFinished(PlaylistItem::SpecialLoadResult)), player_, SLOT(HandleSpecialLoad(PlaylistItem::SpecialLoadResult)));
  connect(radio_model_, SIGNAL(StreamMetadataFound(QUrl,Song)), playlists_, SLOT(SetActiveStreamMetadata(QUrl,Song)));
  connect(radio_model_, SIGNAL(AddItemToPlaylist(RadioItem*)), SLOT(InsertRadioItem(RadioItem*)));
  connect(radio_model_, SIGNAL(AddItemsToPlaylist(PlaylistItemList)), SLOT(InsertRadioItems(PlaylistItemList)));
  connect(radio_model_->GetLastFMService(), SIGNAL(ScrobblingEnabledChanged(bool)), SLOT(ScrobblingEnabledChanged(bool)));
  connect(radio_model_->GetLastFMService(), SIGNAL(ButtonVisibilityChanged(bool)), SLOT(LastFMButtonVisibilityChanged(bool)));
  connect(ui_->radio_view->tree(), SIGNAL(doubleClicked(QModelIndex)), SLOT(RadioDoubleClick(QModelIndex)));

  LastFMButtonVisibilityChanged(radio_model_->GetLastFMService()->AreButtonsVisible());

  // Connections to the saved streams service
  SavedRadio* saved_radio = RadioModel::Service<SavedRadio>();
  add_stream_dialog_->set_add_on_accept(saved_radio);

  connect(saved_radio, SIGNAL(ShowAddStreamDialog()),
          add_stream_dialog_.get(), SLOT(show()));

  // Tray icon
  QMenu* tray_menu = new QMenu(this);
  tray_menu->addAction(ui_->action_previous_track);
  tray_menu->addAction(ui_->action_play_pause);
  tray_menu->addAction(ui_->action_stop);
  tray_menu->addAction(ui_->action_stop_after_this_track);
  tray_menu->addAction(ui_->action_next_track);
  tray_menu->addSeparator();
  tray_menu->addAction(ui_->action_love);
  tray_menu->addAction(ui_->action_ban);
  tray_menu->addSeparator();
  tray_menu->addAction(ui_->action_quit);
  
#ifdef Q_OS_DARWIN
  // Add check for updates item to application menu.
  QAction* check_updates = ui_->menuSettings->addAction(tr("Check for updates..."));
  check_updates->setMenuRole(QAction::ApplicationSpecificRole);
  connect(check_updates, SIGNAL(triggered(bool)), SLOT(CheckForUpdates()));
  // We use the dock instead of the system tray on mac.
  qt_mac_set_dock_menu(tray_menu);

  // Force this menu to be the app "Preferences".
  ui_->action_configure->setMenuRole(QAction::PreferencesRole);
  // Force this menu into the "Clementine" menu.
  ui_->action_configure_global_shortcuts->setMenuRole(QAction::ApplicationSpecificRole);
  // Force this menu to be the app "About".
  ui_->action_about->setMenuRole(QAction::AboutRole);
#else
  tray_icon_->setContextMenu(tray_menu);
  connect(tray_icon_, SIGNAL(WheelEvent(int)), SLOT(VolumeWheelEvent(int)));
  connect(tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(TrayClicked(QSystemTrayIcon::ActivationReason)));
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
  connect(global_shortcuts_dialog_.get(), SIGNAL(accepted()), global_shortcuts_, SLOT(ReloadSettings()));

  // Settings
  connect(settings_dialog_.get(), SIGNAL(accepted()), SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), player_, SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), osd_, SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), ui_->library_view, SLOT(ReloadSettings()));
  connect(settings_dialog_.get(), SIGNAL(accepted()), player_->GetEngine(), SLOT(ReloadSettings()));

  // Add stream dialog
  connect(add_stream_dialog_.get(), SIGNAL(accepted()), SLOT(AddStreamAccepted()));

  // Analyzer
  ui_->analyzer->set_engine(player_->GetEngine());

  // Equalizer
  connect(equalizer_.get(), SIGNAL(ParametersChanged(int,QList<int>)),
          player_->GetEngine(), SLOT(SetEqualizerParameters(int,QList<int>)));
  connect(equalizer_.get(), SIGNAL(EnabledChanged(bool)),
          player_->GetEngine(), SLOT(SetEqualizerEnabled(bool)));
  player_->GetEngine()->SetEqualizerEnabled(equalizer_->is_enabled());
  player_->GetEngine()->SetEqualizerParameters(
      equalizer_->preamp_value(), equalizer_->gain_values());

  // Statusbar widgets
  ui_->statusBar->addPermanentWidget(playlist_sequence_);
  ui_->statusBar->addPermanentWidget(track_slider_);
  ui_->statusBar->addWidget(multi_loading_indicator_);
  multi_loading_indicator_->hide();

  // Load theme
  StyleSheetLoader* css_loader = new StyleSheetLoader(this);
  css_loader->SetStyleSheet(this, ":mainwindow.css");

  // Load playlists
  playlists_->Init(library_->backend(), playlist_backend_, playlist_sequence_);

  // Load settings
  settings_.beginGroup(kSettingsGroup);

  restoreGeometry(settings_.value("geometry").toByteArray());
  if (!ui_->splitter->restoreState(settings_.value("splitter_state").toByteArray())) {
    ui_->splitter->setSizes(QList<int>() << 200 << width() - 200);
  }
  ui_->tabs->setCurrentIndex(settings_.value("current_tab", 0).toInt());
  ui_->file_view->SetPath(settings_.value("file_path", QDir::homePath()).toString());

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
  if (hidden && !tray_icon_->isVisible()) {
    settings_.setValue("hidden", false);
    show();
  }
#else  // Q_OS_DARWIN
  // Always show mainwindow on startup on OS X.
  show();
#endif

  QShortcut* close_window_shortcut = new QShortcut(this);
  close_window_shortcut->setKey(Qt::CTRL + Qt::Key_W);
  connect(close_window_shortcut, SIGNAL(activated()), SLOT(hide()));

  library_->Init();
  library_->StartThreads();
}

MainWindow::~MainWindow() {
  SaveGeometry();

  delete ui_;
}

void MainWindow::ReloadSettings() {
#ifndef Q_OS_DARWIN
  bool show_tray = settings_.value("showtray", true).toBool();

  tray_icon_->setVisible(show_tray);
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

  QModelIndex playlist_index = playlists_->current()->InsertPaths(urls);

  if (playlist_index.isValid() && player_->GetState() != Engine::Playing) {
    playlists_->SetActiveToCurrent();
    player_->PlayAt(playlist_index.row(), Engine::First, true);
  }
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

void MainWindow::MediaStopped() {
  ui_->action_stop->setEnabled(false);
  ui_->action_stop_after_this_track->setEnabled(false);
  ui_->action_play_pause->setIcon(IconLoader::Load("media-playback-start"));
  ui_->action_play_pause->setText(tr("Play"));

  ui_->action_play_pause->setEnabled(true);

  ui_->action_ban->setEnabled(false);
  ui_->action_love->setEnabled(false);

  track_position_timer_->stop();
  track_slider_->SetStopped();
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

  track_slider_->SetCanSeek(!is_lastfm);

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

void MainWindow::TrayClicked(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
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
      } else {
        // Window is not hidden but does not have focus; bring it to front.
        activateWindow();
        raise();
      }
      break;

    case QSystemTrayIcon::MiddleClick:
      player_->PlayPause();
      break;

    default:
      break;
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
  if (tray_icon_->isVisible() && event->spontaneous()) {
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
    track_slider_->SetStopped();
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
  track_slider_->SetValue(position, length);

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

  playlists_->current()->dropMimeData(data.get(), Qt::CopyAction, -1, 0, QModelIndex());

  QModelIndex first_song = playlists_->current()->index(0, 0);
  if (!playlists_->current()->proxy()->mapFromSource(first_song).isValid()) {
    // The first song doesn't match the filter, so don't play it
    return;
  }

  if (first_song.isValid() && player_->GetState() != Engine::Playing) {
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

  if (playlists_->current()->current_index() == index.row() && player_->GetState() == Engine::Playing) {
    playlist_play_pause_->setText(tr("Pause"));
    playlist_play_pause_->setIcon(IconLoader::Load("media-playback-pause"));
  } else {
    playlist_play_pause_->setText(tr("Play"));
    playlist_play_pause_->setIcon(IconLoader::Load("media-playback-start"));
  }

  if (index.isValid()) {
    playlist_play_pause_->setEnabled(
        playlists_->current()->current_index() != index.row() ||
        ! (playlists_->current()->item_at(index.row())->options() & PlaylistItem::PauseDisabled));
  } else {
    playlist_play_pause_->setEnabled(false);
  }

  playlist_stop_after_->setEnabled(index.isValid());

  // Are any of the selected songs editable?
  QModelIndexList selection = ui_->playlist->view()->selectionModel()->selection().indexes();
  int editable = 0;
  foreach (const QModelIndex& index, selection) {
    if (index.column() != 0)
      continue;
    if (playlists_->current()->item_at(index.row())->Metadata().IsEditable()) {
      editable++;
    }
  }
  ui_->action_edit_track->setEnabled(editable);

  bool track_column = (index.column() == Playlist::Column_Track);
  ui_->action_renumber_tracks->setVisible(editable >= 2 && track_column);
  ui_->action_selection_set_value->setVisible(editable >= 2 && !track_column);
  ui_->action_edit_value->setVisible(editable);
  ui_->action_remove_from_playlist->setEnabled(!selection.isEmpty());

  if (!index.isValid()) {
    ui_->action_selection_set_value->setVisible(false);
    ui_->action_edit_value->setVisible(false);
  } else {
    Playlist::Column column = (Playlist::Column)index.column();
    bool editable = Playlist::column_is_editable(column);

    ui_->action_selection_set_value->setVisible(
        ui_->action_selection_set_value->isVisible() && editable);
    ui_->action_edit_value->setVisible(
        ui_->action_edit_value->isVisible() && editable);

    QString column_name = Playlist::column_name(column);
    QString column_value = playlists_->current()->data(index).toString();
    if (column_value.length() > 25)
      column_value = column_value.left(25) + "...";

    ui_->action_selection_set_value->setText(tr("Set %1 to \"%2\"...")
             .arg(column_name.toLower()).arg(column_value));
    ui_->action_edit_value->setText(tr("Edit tag \"%1\"...").arg(column_name));
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
      song.Save();
      playlists_->current()->item_at(row)->Reload();
    }
    track++;
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
      song.Save();
      playlists_->current()->item_at(row)->Reload();
    }
  }
}

void MainWindow::EditValue() {
  ui_->playlist->view()->edit(playlist_menu_index_);
}

void MainWindow::LibraryScanStarted() {
  multi_loading_indicator_->TaskStarted(MultiLoadingIndicator::UpdatingLibrary);
}

void MainWindow::LibraryScanFinished() {
  multi_loading_indicator_->TaskFinished(MultiLoadingIndicator::UpdatingLibrary);
}

void MainWindow::AddFile() {
  // Last used directory
  QString directory = settings_.value("add_media_path", QDir::currentPath()).toString();

  // Show dialog
  QStringList file_names = QFileDialog::getOpenFileNames(
      this, tr("Add media"), directory,
      QString("%1;;%2;;%3").arg(tr(kMusicFilterSpec), tr(kPlaylistFilterSpec),
                                tr(kAllFilesFilterSpec)));
  if (file_names.isEmpty())
    return;

  // Save last used directory
  settings_.setValue("add_media_path", file_names[0]);

  // Add media
  QList<QUrl> urls;
  foreach (const QString& path, file_names) {
    if (playlist_parser_->can_load(path)) {
      playlists_->current()->InsertSongs(playlist_parser_->Load(path));
    } else {
      QUrl url(QUrl::fromLocalFile(path));
      if (url.scheme().isEmpty())
        url.setScheme("file");
      urls << url;
    }
  }
  playlists_->current()->InsertPaths(urls);
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
  QUrl url(QUrl::fromLocalFile(directory));
  if (url.scheme().isEmpty())
    url.setScheme("file");
  playlists_->current()->InsertPaths(QList<QUrl>() << url);
}

void MainWindow::AddStream() {
  add_stream_dialog_->show();
}

void MainWindow::AddStreamAccepted() {
  QList<QUrl> urls;
  urls << add_stream_dialog_->url();

  playlists_->current()->InsertStreamUrls(urls);
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
      playlists_->current()->InsertPaths(options.urls(), -1);
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

bool MainWindow::event(QEvent* event) {
  // ApplicationActivate is received when the dock is clicked on OS X.
  if (event->type() == QEvent::ApplicationActivate) {
    show();
    return true;
  }
  return QMainWindow::event(event);
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
  settings_dialog_->OpenAtPage(SettingsDialog::Page_Library);
}
