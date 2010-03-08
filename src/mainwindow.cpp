#include "mainwindow.h"
#include "player.h"
#include "playlistview.h"
#include "playlist.h"
#include "library.h"
#include "libraryconfig.h"
#include "songplaylistitem.h"
#include "systemtrayicon.h"
#include "radiomodel.h"
#include "enginebase.h"
#include "lastfmservice.h"
#include "osd.h"
#include "trackslider.h"
#include "edittagdialog.h"
#include "multiloadingindicator.h"
#include "settingsdialog.h"
#include "shortcutsdialog.h"
#include "libraryconfigdialog.h"
#include "about.h"
#include "addstreamdialog.h"
#include "stylesheetloader.h"
#include "albumcovermanager.h"
#include "m3uparser.h"
#include "playlistmanager.h"
#include "shufflerepeatwidget.h"

#include "qxtglobalshortcut.h"

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

#include <cmath>


const int MainWindow::kStateVersion = 1;
const char* MainWindow::kSettingsGroup = "MainWindow";
const char* MainWindow::kMediaFilterSpec =
    "Music (*.mp3 *.ogg *.flac *.mpc *.m4a *.aac *.wma);;Playlists (*.m3u)";

MainWindow::MainWindow(QNetworkAccessManager* network, QWidget *parent)
  : QMainWindow(parent),
    tray_icon_(new SystemTrayIcon(this)),
    osd_(new OSD(tray_icon_, this)),
    track_slider_(new TrackSlider(this)),
    shuffle_repeat_widget_(new ShuffleRepeatWidget(this)),
    edit_tag_dialog_(new EditTagDialog(this)),
    multi_loading_indicator_(new MultiLoadingIndicator(this)),
    library_config_dialog_(new LibraryConfigDialog(this)),
    about_dialog_(new About(this)),
    radio_model_(new RadioModel(this)),
    current_playlist_(NULL),
    current_playlist_view_(NULL),
    player_(new Player(current_playlist_, radio_model_->GetLastFMService(), this)),
    library_(new Library(player_->GetEngine(), this)),
    settings_dialog_(new SettingsDialog(this)),
    add_stream_dialog_(new AddStreamDialog(this)),
    shortcuts_dialog_(new ShortcutsDialog(this)),
    cover_manager_(new AlbumCoverManager(network, this)),
    playlist_menu_(new QMenu(this)),
    library_sort_model_(new QSortFilterProxyModel(this)),
    track_position_timer_(new QTimer(this)),
    next_playlist_number_(1),
    playlistManager_( new PlaylistManager(this) ) 
{
  ui_.setupUi(this);
  tray_icon_->setIcon(windowIcon());
  tray_icon_->setToolTip(QCoreApplication::applicationName());

  ui_.volume->setValue(player_->GetVolume());

  track_position_timer_->setInterval(1000);
  connect(track_position_timer_, SIGNAL(timeout()), SLOT(UpdateTrackPosition()));

  // Start initialising the player
  multi_loading_indicator_->TaskStarted(MultiLoadingIndicator::LoadingAudioEngine);
  player_->Init();

  // Models
  library_sort_model_->setSourceModel(library_);
  library_sort_model_->setSortRole(Library::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  ui_.library_view->setModel(library_sort_model_);
  ui_.library_view->SetLibrary(library_);
  library_config_dialog_->SetModel(library_->GetDirectoryModel());
  settings_dialog_->SetLibraryDirectoryModel(library_->GetDirectoryModel());

  ui_.radio_view->setModel(radio_model_);

  // File view connections
  connect(ui_.file_view, SIGNAL(Queue(QList<QUrl>)), SLOT(QueueFiles(QList<QUrl>)));
  connect(ui_.file_view, SIGNAL(PathChanged(QString)), SLOT(FilePathChanged(QString)));

  // Action connections
  connect(ui_.action_next_track, SIGNAL(triggered()), player_, SLOT(Next()));
  connect(ui_.action_previous_track, SIGNAL(triggered()), player_, SLOT(Previous()));
  connect(ui_.action_play_pause, SIGNAL(triggered()), player_, SLOT(PlayPause()));
  connect(ui_.action_stop, SIGNAL(triggered()), player_, SLOT(Stop()));
  connect(ui_.action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
  connect(ui_.action_stop_after_this_track, SIGNAL(triggered()), SLOT(StopAfterCurrent()));
  connect(ui_.library_filter, SIGNAL(textChanged(QString)), library_, SLOT(SetFilterText(QString)));
  connect(ui_.action_ban, SIGNAL(triggered()), radio_model_->GetLastFMService(), SLOT(Ban()));
  connect(ui_.action_love, SIGNAL(triggered()), SLOT(Love()));

  connect(ui_.action_edit_track, SIGNAL(triggered()), SLOT(EditTracks()));
  connect(ui_.action_configure, SIGNAL(triggered()), settings_dialog_, SLOT(show()));
  connect(ui_.action_about, SIGNAL(triggered()), about_dialog_, SLOT(show()));

  connect(ui_.action_open_media, SIGNAL(triggered()), SLOT(AddMedia()));
  connect(ui_.action_add_media, SIGNAL(triggered()), SLOT(AddMedia()));
  connect(ui_.action_add_stream, SIGNAL(triggered()), SLOT(AddStream()));
  connect(ui_.action_new_playlist, SIGNAL(triggered()), SLOT(NewPlaylist()));
  connect(ui_.action_hide_tray_icon, SIGNAL(triggered()), SLOT(HideShowTrayIcon()));
  connect(ui_.action_global_shortcuts, SIGNAL(triggered()), shortcuts_dialog_, SLOT(show()));
  connect(ui_.action_cover_manager, SIGNAL(triggered()), cover_manager_, SLOT(show()));

  // Give actions to buttons
  ui_.forward_button->setDefaultAction(ui_.action_next_track);
  ui_.back_button->setDefaultAction(ui_.action_previous_track);
  ui_.pause_play_button->setDefaultAction(ui_.action_play_pause);
  ui_.stop_button->setDefaultAction(ui_.action_stop);
  ui_.love_button->setDefaultAction(ui_.action_love);
  ui_.ban_button->setDefaultAction(ui_.action_ban);
  ui_.clear_playlist_button->setDefaultAction(ui_.action_clear_playlist);

  // Stop actions
  QMenu* stop_menu = new QMenu(this);
  stop_menu->addAction(ui_.action_stop);
  stop_menu->addAction(ui_.action_stop_after_this_track);
  ui_.stop_button->setMenu(stop_menu);

  // Player connections
  connect(ui_.volume, SIGNAL(valueChanged(int)), player_, SLOT(SetVolume(int)));

  connect(player_, SIGNAL(InitFinished()), SLOT(PlayerInitFinished()));
  connect(player_, SIGNAL(Error(QString)), SLOT(ReportError(QString)));
  connect(player_, SIGNAL(Paused()), SLOT(MediaPaused()));
  connect(player_, SIGNAL(Playing()), SLOT(MediaPlaying()));
  connect(player_, SIGNAL(Stopped()), SLOT(MediaStopped()));

  connect(player_, SIGNAL(Paused()), osd_, SLOT(Paused()));
  connect(player_, SIGNAL(Stopped()), osd_, SLOT(Stopped()));
  connect(player_, SIGNAL(VolumeChanged(int)), osd_, SLOT(VolumeChanged(int)));
  
  // PlaylistManager connections
  connect (playlistManager_, SIGNAL(CurrentPlaylistChanged(Playlist*)), this, SLOT(CurrentPlaylistChanged(Playlist*)));

  connect(track_slider_, SIGNAL(ValueChanged(int)), player_, SLOT(Seek(int)));

  // Tab connections
  connect (ui_.tab_widget, SIGNAL(currentChanged(int)), SLOT(CurrentTabChanged(int)));

  // Library connections
  connect(library_, SIGNAL(Error(QString)), SLOT(ReportError(QString)));
  connect(ui_.library_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(LibraryDoubleClick(QModelIndex)));
  connect(ui_.library_view, SIGNAL(ShowConfigDialog()), library_config_dialog_, SLOT(show()));
  connect(library_, SIGNAL(TotalSongCountUpdated(int)), ui_.library_view, SLOT(TotalSongCountUpdated(int)));
  connect(library_, SIGNAL(ScanStarted()), SLOT(LibraryScanStarted()));
  connect(library_, SIGNAL(ScanFinished()), SLOT(LibraryScanFinished()));
  connect(library_, SIGNAL(BackendReady(boost::shared_ptr<LibraryBackend>)),
          cover_manager_, SLOT(SetBackend(boost::shared_ptr<LibraryBackend>)));

  // Age filters
  QActionGroup* filter_age_group = new QActionGroup(this);
  filter_age_group->addAction(ui_.filter_age_all);
  filter_age_group->addAction(ui_.filter_age_today);
  filter_age_group->addAction(ui_.filter_age_week);
  filter_age_group->addAction(ui_.filter_age_month);
  filter_age_group->addAction(ui_.filter_age_three_months);
  filter_age_group->addAction(ui_.filter_age_year);
  filter_age_group->setExclusive(true);

  QSignalMapper* filter_age_mapper = new QSignalMapper(this);
  filter_age_mapper->setMapping(ui_.filter_age_all, -1);
  filter_age_mapper->setMapping(ui_.filter_age_today, 60*60*24);
  filter_age_mapper->setMapping(ui_.filter_age_week, 60*60*24*7);
  filter_age_mapper->setMapping(ui_.filter_age_month, 60*60*24*30);
  filter_age_mapper->setMapping(ui_.filter_age_three_months, 60*60*24*30*3);
  filter_age_mapper->setMapping(ui_.filter_age_year, 60*60*24*365);

  connect(ui_.filter_age_all, SIGNAL(triggered()), filter_age_mapper, SLOT(map()));
  connect(ui_.filter_age_today, SIGNAL(triggered()), filter_age_mapper, SLOT(map()));
  connect(ui_.filter_age_week, SIGNAL(triggered()), filter_age_mapper, SLOT(map()));
  connect(ui_.filter_age_month, SIGNAL(triggered()), filter_age_mapper, SLOT(map()));
  connect(ui_.filter_age_three_months, SIGNAL(triggered()), filter_age_mapper, SLOT(map()));
  connect(ui_.filter_age_year, SIGNAL(triggered()), filter_age_mapper, SLOT(map()));
  connect(filter_age_mapper, SIGNAL(mapped(int)), library_, SLOT(SetFilterAge(int)));
  connect(ui_.library_filter_clear, SIGNAL(clicked()), SLOT(ClearLibraryFilter()));

  // Library config menu
  QMenu* library_menu = new QMenu(this);
  library_menu->addActions(filter_age_group->actions());
  library_menu->addSeparator();
  library_menu->addAction(tr("Configure library..."), library_config_dialog_, SLOT(show()));
  ui_.library_options->setMenu(library_menu);

  // Playlist menu
  playlist_play_pause_ = playlist_menu_->addAction(tr("Play"), this, SLOT(PlaylistPlay()));
  playlist_menu_->addAction(ui_.action_stop);
  playlist_stop_after_ = playlist_menu_->addAction(QIcon(":media-playback-stop.png"), tr("Stop after this track"), this, SLOT(PlaylistStopAfter()));
  playlist_menu_->addAction(ui_.action_edit_track);
  playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_.action_clear_playlist);
  playlist_menu_->addAction(ui_.action_shuffle);

  // Radio connections
  connect(radio_model_, SIGNAL(TaskStarted(MultiLoadingIndicator::TaskType)), multi_loading_indicator_, SLOT(TaskStarted(MultiLoadingIndicator::TaskType)));
  connect(radio_model_, SIGNAL(TaskFinished(MultiLoadingIndicator::TaskType)), multi_loading_indicator_, SLOT(TaskFinished(MultiLoadingIndicator::TaskType)));
  connect(radio_model_, SIGNAL(StreamError(QString)), SLOT(ReportError(QString)));
  connect(radio_model_, SIGNAL(StreamFinished()), player_, SLOT(NextItem()));
  connect(radio_model_, SIGNAL(StreamReady(QUrl,QUrl)), player_, SLOT(StreamReady(QUrl,QUrl)));
  connect(radio_model_, SIGNAL(StreamMetadataFound(QUrl,Song)), current_playlist_, SLOT(SetStreamMetadata(QUrl,Song)));
  connect(radio_model_, SIGNAL(AddItemToPlaylist(RadioItem*)), SLOT(InsertRadioItem(RadioItem*)));
  connect(radio_model_->GetLastFMService(), SIGNAL(ScrobblingEnabledChanged(bool)), SLOT(ScrobblingEnabledChanged(bool)));
  connect(ui_.radio_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(RadioDoubleClick(QModelIndex)));

  // Tray icon
  QMenu* tray_menu = new QMenu(this);
  tray_menu->addAction(ui_.action_previous_track);
  tray_menu->addAction(ui_.action_play_pause);
  tray_menu->addAction(ui_.action_stop);
  tray_menu->addAction(ui_.action_next_track);
  tray_menu->addSeparator();
  tray_menu->addAction(ui_.action_love);
  tray_menu->addAction(ui_.action_ban);
  tray_menu->addSeparator();
  tray_menu->addAction(ui_.action_hide_tray_icon);
  tray_menu->addSeparator();
  tray_menu->addAction(ui_.action_quit);
  tray_icon_->setContextMenu(tray_menu);

  connect(tray_icon_, SIGNAL(WheelEvent(int)), SLOT(VolumeWheelEvent(int)));
  connect(tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(TrayClicked(QSystemTrayIcon::ActivationReason)));

  // Global shortcuts
  // Breaks on OS X and Windows
#ifdef Q_OS_LINUX
  QxtGlobalShortcut* play_pause = new QxtGlobalShortcut(QKeySequence("Media Play"), this);
  QxtGlobalShortcut* stop = new QxtGlobalShortcut(QKeySequence("Media Stop"), this);
  QxtGlobalShortcut* next = new QxtGlobalShortcut(QKeySequence("Media Next"), this);
  QxtGlobalShortcut* prev = new QxtGlobalShortcut(QKeySequence("Media Previous"), this);
  connect(play_pause, SIGNAL(activated()), ui_.action_play_pause, SLOT(trigger()));
  connect(stop, SIGNAL(activated()), ui_.action_stop, SLOT(trigger()));
  connect(next, SIGNAL(activated()), ui_.action_next_track, SLOT(trigger()));
  connect(prev, SIGNAL(activated()), ui_.action_previous_track, SLOT(trigger()));
#endif  // Q_OS_LINUX

  // Settings
  connect(settings_dialog_, SIGNAL(accepted()), player_, SLOT(ReloadSettings()));
  connect(settings_dialog_, SIGNAL(accepted()), osd_, SLOT(ReloadSettings()));

  // Add stream dialog
  connect(add_stream_dialog_, SIGNAL(accepted()), SLOT(AddStreamAccepted()));

  // Analyzer
  ui_.analyzer->set_engine(player_->GetEngine());

  // Statusbar widgets
  ui_.statusBar->addPermanentWidget(shuffle_repeat_widget_);
  ui_.statusBar->addPermanentWidget(track_slider_);
  ui_.statusBar->addWidget(multi_loading_indicator_);
  multi_loading_indicator_->hide();

  // Load theme
  StyleSheetLoader* css_loader = new StyleSheetLoader(this);
  css_loader->SetStyleSheet(this, ":mainwindow.css");

  // Load settings
  settings_.beginGroup(kSettingsGroup);

  restoreGeometry(settings_.value("geometry").toByteArray());
  if (!restoreState(settings_.value("state").toByteArray(), kStateVersion)) {
    tabifyDockWidget(ui_.files_dock, ui_.radio_dock);
    tabifyDockWidget(ui_.files_dock, ui_.library_dock);
  }

  ui_.file_view->SetPath(settings_.value("file_path", QDir::homePath()).toString());

  if (!settings_.value("hidden", false).toBool()) {
    show();
  }

  if (settings_.value("showtray", true).toBool()) {
    tray_icon_->show();
  }
  else {
    ui_.action_hide_tray_icon->setText(tr("&Show tray icon"));
  }

  // Force the window to show in case somehow the config has tray and window set to hide
  // Why doesn't .toBool() work? This might be the case for any combinations with .toBool(); use .toInt()
  if (!settings_.value("hidden", true).toInt() && !settings_.value("showtray", false).toInt()) {
    settings_.setValue("hidden", false);
    show();
  }

  library_->StartThreads();
  
  playlistManager_->SetTabWidget(ui_.tab_widget);
  
  QTimer::singleShot(500,this,SLOT(InitPlaylists())) ; 
}

MainWindow::~MainWindow() {
  SaveGeometry();
  playlistManager_->Save() ; 
  
  delete player_ ; 
}

void MainWindow::HideShowTrayIcon() {
  if (!isHidden() && tray_icon_->isVisible()) {
    tray_icon_->setVisible(false);
    ui_.action_hide_tray_icon->setText(tr("&Show tray icon"));
  }
  else if (!isHidden()) {
    tray_icon_->setVisible(true);
    ui_.action_hide_tray_icon->setText("&Hide tray icon");
  }

  settings_.setValue("showtray", tray_icon_->isVisible());
}

void MainWindow::QueueFiles(const QList<QUrl>& urls) {
  QModelIndex playlist_index = current_playlist_->InsertPaths(urls);

  if (playlist_index.isValid() && player_->GetState() != Engine::Playing)
    player_->PlayAt(playlist_index.row());
}

void MainWindow::ReportError(const QString& message) {
  QMessageBox::warning(this, "Error", message);
}

void MainWindow::MediaStopped() {
  ui_.action_stop->setEnabled(false);
  ui_.action_stop_after_this_track->setEnabled(false);
  ui_.action_play_pause->setIcon(QIcon(":media-playback-start.png"));
  ui_.action_play_pause->setText(tr("Play"));

  ui_.action_play_pause->setEnabled(true);

  ui_.action_ban->setEnabled(false);
  ui_.action_love->setEnabled(false);

  track_position_timer_->stop();
  track_slider_->SetStopped();
  tray_icon_->SetProgress(0);
}

void MainWindow::MediaPaused() {
  ui_.action_stop->setEnabled(true);
  ui_.action_stop_after_this_track->setEnabled(true);
  ui_.action_play_pause->setIcon(QIcon(":media-playback-start.png"));
  ui_.action_play_pause->setText(tr("Play"));

  ui_.action_play_pause->setEnabled(true);

  track_position_timer_->stop();
}

void MainWindow::MediaPlaying() {
  ui_.action_stop->setEnabled(true);
  ui_.action_stop_after_this_track->setEnabled(true);
  ui_.action_play_pause->setIcon(QIcon(":media-playback-pause.png"));
  ui_.action_play_pause->setText(tr("Pause"));

  ui_.action_play_pause->setEnabled(
      ! (player_->GetCurrentItemOptions() & PlaylistItem::PauseDisabled));

  bool is_lastfm = (player_->GetCurrentItemOptions() & PlaylistItem::LastFMControls);
  LastFMService* lastfm = radio_model_->GetLastFMService();

  ui_.action_ban->setEnabled(lastfm->IsScrobblingEnabled() && is_lastfm);
  ui_.action_love->setEnabled(lastfm->IsScrobblingEnabled());

  track_slider_->SetCanSeek(!is_lastfm);

  track_position_timer_->start();
  UpdateTrackPosition();
}

void MainWindow::ScrobblingEnabledChanged(bool value) {
  if (!player_->GetState() == Engine::Idle)
    return;

  bool is_lastfm = (player_->GetCurrentItemOptions() & PlaylistItem::LastFMControls);
  ui_.action_ban->setEnabled(value && is_lastfm);
  ui_.action_love->setEnabled(value);
}

void MainWindow::resizeEvent(QResizeEvent*) {
  SaveGeometry();
}

void MainWindow::SaveGeometry() {
  settings_.setValue("geometry", saveGeometry());
  settings_.setValue("state", saveState(kStateVersion));
}

void MainWindow::PlayIndex(const QModelIndex& index) {
  if (!index.isValid())
    return;

  player_->PlayAt(index.row());
}

void MainWindow::LibraryDoubleClick(const QModelIndex& index) {
  QModelIndex first_song =
      current_playlist_->InsertSongs(library_->GetChildSongs(
          library_sort_model_->mapToSource(index)));

  if (first_song.isValid() && player_->GetState() != Engine::Playing)
    player_->PlayAt(first_song.row());
}

void MainWindow::VolumeWheelEvent(int delta) {
  ui_.volume->setValue(ui_.volume->value() + delta / 20);
}

void MainWindow::TrayClicked(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
      SetHiddenInTray(isVisible());
      break;

    case QSystemTrayIcon::MiddleClick:
      player_->PlayPause();
      break;

    default:
      break;
  }
}

void MainWindow::StopAfterCurrent() {
  current_playlist_->StopAfter(current_playlist_->current_index());
}

/**
  * Exit if the tray icon is not visible, otherwise ignore and set hidden in tray.
  */
void MainWindow::closeEvent(QCloseEvent* event) {
  if (tray_icon_->isVisible()) {
    event->ignore();
    SetHiddenInTray(true);
  }
  else {
    settings_.setValue("showtray", tray_icon_->isVisible());
  }
}

void MainWindow::SetHiddenInTray(bool hidden) {
  settings_.setValue("hidden", hidden);

  if (hidden) {
    hide();
    ui_.action_hide_tray_icon->setEnabled(false); // Disable hiding tray icon if window is hidden
  }
  else {
    show();
    ui_.action_hide_tray_icon->setEnabled(true);
  }
}

void MainWindow::ClearLibraryFilter() {
  ui_.library_filter->clear();
  ui_.library_filter->setFocus();
}

void MainWindow::FilePathChanged(const QString& path) {
  settings_.setValue("file_path", path);
}

void MainWindow::UpdateTrackPosition() {
  // Track position in seconds
  const int position = std::floor(float(player_->GetEngine()->position()) / 1000.0 + 0.5);
  const int length = player_->GetCurrentItem().length();

  if (length <= 0) {
    // Probably a stream that we don't know the length of
    track_slider_->SetStopped();
    tray_icon_->SetProgress(0);
    return;
  }

  // Time to scrobble?
  LastFMService* lastfm = radio_model_->GetLastFMService();

  if (!current_playlist_->has_scrobbled() &&
      position >= current_playlist_->scrobble_point()) {
    lastfm->Scrobble();
    current_playlist_->set_scrobbled(true);
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
  ui_.action_love->setEnabled(false);
}

void MainWindow::RadioDoubleClick(const QModelIndex& index) {
  InsertRadioItem(radio_model_->IndexToItem(index));
}

void MainWindow::InsertRadioItem(RadioItem* item) {
  QModelIndex first_song = current_playlist_->InsertRadioStations(
      QList<RadioItem*>() << item);

  if (first_song.isValid() && player_->GetState() != Engine::Playing)
    player_->PlayAt(first_song.row());
}

void MainWindow::PlaylistRightClick(const QPoint& global_pos, const QModelIndex& index) {
  playlist_menu_index_ = index;

  if ( current_playlist_ == NULL )
    return ; // we don't have any playlist created. 
  if (current_playlist_->current_index() == index.row() && player_->GetState() == Engine::Playing) {
    playlist_play_pause_->setText(tr("Pause"));
    playlist_play_pause_->setIcon(QIcon(":media-playback-pause.png"));
  } else {
    playlist_play_pause_->setText(tr("Play"));
    playlist_play_pause_->setIcon(QIcon(":media-playback-start.png"));
  }

  if (index.isValid()) {
    playlist_play_pause_->setEnabled(
        current_playlist_->current_index() != index.row() ||
        ! (current_playlist_->item_at(index.row())->options() & PlaylistItem::PauseDisabled));
  } else {
    playlist_play_pause_->setEnabled(false);
  }

  playlist_stop_after_->setEnabled(index.isValid());

  // Are any of the selected songs editable?
  bool editable = false;
  foreach (const QModelIndex& index,
           current_playlist_view_->selectionModel()->selection().indexes()) {
    if (index.column() != 0)
      continue;
    if (current_playlist_->item_at(index.row())->Metadata().IsEditable()) {
      editable = true;
      break;
    }
  }
  ui_.action_edit_track->setEnabled(editable);

  playlist_menu_->popup(global_pos);
}

void MainWindow::PlaylistPlay() {
  if (current_playlist_->current_index() == playlist_menu_index_.row()) {
    player_->PlayPause();
  } else {
    player_->PlayAt(playlist_menu_index_.row());
  }
}

void MainWindow::PlaylistStopAfter() {
  current_playlist_->StopAfter(playlist_menu_index_.row());
}

void MainWindow::EditTracks() {
  SongList songs;
  QList<int> rows;

  foreach (const QModelIndex& index,
           current_playlist_view_->selectionModel()->selection().indexes()) {
    if (index.column() != 0)
      continue;
    Song song = current_playlist_->item_at(index.row())->Metadata();

    if (song.IsEditable()) {
      songs << song;
      rows << index.row();
    }
  }

  edit_tag_dialog_->SetSongs(songs);
  if (edit_tag_dialog_->exec() == QDialog::Rejected)
    return;

  current_playlist_->ReloadItems(rows);
}

void MainWindow::LibraryScanStarted() {
  multi_loading_indicator_->TaskStarted(MultiLoadingIndicator::UpdatingLibrary);
}

void MainWindow::LibraryScanFinished() {
  multi_loading_indicator_->TaskFinished(MultiLoadingIndicator::UpdatingLibrary);
}

void MainWindow::PlayerInitFinished() {
  multi_loading_indicator_->TaskFinished(MultiLoadingIndicator::LoadingAudioEngine);
}

void MainWindow::AddMedia() {
  // Last used directory
  QString directory = settings_.value("add_media_path", QDir::currentPath()).toString();

  // Show dialog
  QStringList file_names = QFileDialog::getOpenFileNames(
      this, "Add media", directory, kMediaFilterSpec);
  if (file_names.isEmpty())
    return;

  // Save last used directory
  settings_.setValue("add_media_path", file_names[0]);

  // Add media
  QList<QUrl> urls;
  foreach (const QString& path, file_names) {
    if (path.endsWith(".m3u")) {
      QFile file(path);
      QFileInfo info(file);
      file.open(QIODevice::ReadOnly);
      M3UParser parser(&file, info.dir());
      const SongList& songs = parser.Parse();
      current_playlist_->InsertSongs(songs);
    } else {
      QUrl url(path);
      if (url.scheme().isEmpty())
        url.setScheme("file");
      urls << url;
    }
  }
  current_playlist_->InsertPaths(urls);
}

void MainWindow::AddStream() {
  add_stream_dialog_->show();
}

void MainWindow::AddStreamAccepted() {
  QList<QUrl> urls;
  urls << add_stream_dialog_->url();

  current_playlist_->InsertStreamUrls(urls);
}
void MainWindow::NewPlaylist(){
  playlistManager_->addPlaylist();
}
void MainWindow::SetCurrentPlaylist(PlaylistView* pCurrent){
    // tab widget ;     
    //disconnects!!
    if ( current_playlist_ != NULL && current_playlist_view_ != NULL ) {
      disconnect(current_playlist_, SIGNAL(CurrentSongChanged(Song)), osd_, SLOT(SongChanged(Song)));
      disconnect(current_playlist_view_, SIGNAL(doubleClicked(QModelIndex)),this, SLOT(PlayIndex(QModelIndex)));
      disconnect(current_playlist_view_, SIGNAL(PlayPauseItem(QModelIndex)),this, SLOT(PlayIndex(QModelIndex)));
      disconnect(current_playlist_view_, SIGNAL(RightClicked(QPoint,QModelIndex)),this, SLOT(PlaylistRightClick(QPoint,QModelIndex)));
      disconnect(ui_.action_clear_playlist, SIGNAL(triggered()), current_playlist_, SLOT(Clear()));
      disconnect(ui_.action_shuffle, SIGNAL(triggered()), current_playlist_, SLOT(Shuffle()));
      disconnect(player_, SIGNAL(Paused()), current_playlist_, SLOT(Paused()));
      disconnect(player_, SIGNAL(Playing()), current_playlist_, SLOT(Playing()));
      disconnect(player_, SIGNAL(Stopped()), current_playlist_, SLOT(Stopped()));

      disconnect(player_, SIGNAL(Paused()), current_playlist_view_, SLOT(StopGlowing()));
      disconnect(player_, SIGNAL(Playing()), current_playlist_view_, SLOT(StartGlowing()));
      disconnect(player_, SIGNAL(Stopped()), current_playlist_view_, SLOT(StopGlowing()));
      
      disconnect(radio_model_, SIGNAL(StreamMetadataFound(QUrl,Song)), current_playlist_, SLOT(SetStreamMetadata(QUrl,Song)));
    }
    
    // repin pointers
    
    ui_.tab_widget->setCurrentWidget(pCurrent);
    current_playlist_view_ = pCurrent ; 
    current_playlist_ = qobject_cast< Playlist* >( pCurrent->model() );
    player_->SetCurrentPlaylist(current_playlist_);
    
    
    // connects !! :)
    
    connect(current_playlist_, SIGNAL(CurrentSongChanged(Song)), osd_, SLOT(SongChanged(Song)));
    connect(current_playlist_, SIGNAL(CurrentSongChanged(Song)), player_, SLOT(CurrentMetadataChanged(Song)));
    connect(current_playlist_view_, SIGNAL(doubleClicked(QModelIndex)), SLOT(PlayIndex(QModelIndex)));
    connect(current_playlist_view_, SIGNAL(PlayPauseItem(QModelIndex)), SLOT(PlayIndex(QModelIndex)));
    connect(current_playlist_view_, SIGNAL(RightClicked(QPoint,QModelIndex)), SLOT(PlaylistRightClick(QPoint,QModelIndex)));
    connect(ui_.action_clear_playlist, SIGNAL(triggered()), current_playlist_, SLOT(Clear()));
    connect(ui_.action_shuffle, SIGNAL(triggered()), current_playlist_, SLOT(Shuffle()));
    connect(player_, SIGNAL(Paused()), current_playlist_, SLOT(Paused()));
    connect(player_, SIGNAL(Playing()), current_playlist_, SLOT(Playing()));
    connect(player_, SIGNAL(Stopped()), current_playlist_, SLOT(Stopped()));

    connect(player_, SIGNAL(Paused()), current_playlist_view_, SLOT(StopGlowing()));
    connect(player_, SIGNAL(Playing()), current_playlist_view_, SLOT(StartGlowing()));
    connect(player_, SIGNAL(Stopped()), current_playlist_view_, SLOT(StopGlowing()));
    
    connect(radio_model_, SIGNAL(StreamMetadataFound(QUrl,Song)), current_playlist_, SLOT(SetStreamMetadata(QUrl,Song)));
}

void MainWindow::CurrentTabChanged(int index ){
    PlaylistView *pCurrent = qobject_cast< PlaylistView* >( ui_.tab_widget->currentWidget() ); 
    SetCurrentPlaylist(pCurrent);
}
void MainWindow::CurrentPlaylistChanged(Playlist* pPlaylist){

}
void MainWindow::InitPlaylists(){
  bool bRestored = playlistManager_->Restore() ; 
  if ( !bRestored ) 
    playlistManager_->addPlaylist();
}
