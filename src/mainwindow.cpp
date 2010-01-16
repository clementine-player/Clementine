#include "mainwindow.h"
#include "player.h"
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

#include <cmath>

const int MainWindow::kStateVersion = 1;
const char* MainWindow::kSettingsGroup = "MainWindow";

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    tray_icon_(new SystemTrayIcon(this)),
    osd_(new OSD(tray_icon_, this)),
    track_slider_(new TrackSlider(this)),
    edit_tag_dialog_(new EditTagDialog(this)),
    multi_loading_indicator_(new MultiLoadingIndicator(this)),
    radio_model_(new RadioModel(this)),
    playlist_(new Playlist(this)),
    player_(new Player(playlist_, radio_model_->GetLastFMService(), this)),
    library_(new Library(player_->GetEngine(), this)),
    playlist_menu_(new QMenu(this)),
    library_sort_model_(new QSortFilterProxyModel(this)),
    track_position_timer_(new QTimer(this))
{
  ui_.setupUi(this);
  tray_icon_->setIcon(windowIcon());
  tray_icon_->setToolTip(QCoreApplication::applicationName());
  tray_icon_->show();

  ui_.volume->setValue(player_->GetVolume());

  track_position_timer_->setInterval(1000);
  connect(track_position_timer_, SIGNAL(timeout()), SLOT(UpdateTrackPosition()));

  // Models
  library_sort_model_->setSourceModel(library_);
  library_sort_model_->setSortRole(Library::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  playlist_->Restore();

  playlist_->IgnoreSorting(true);
  ui_.playlist->setModel(playlist_);
  playlist_->IgnoreSorting(false);

  ui_.library_view->setModel(library_sort_model_);
  ui_.library_view->SetLibrary(library_);

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
  connect(ui_.action_clear_playlist, SIGNAL(triggered()), playlist_, SLOT(Clear()));
  connect(ui_.action_edit_track, SIGNAL(triggered()), SLOT(EditTracks()));

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

  connect(player_, SIGNAL(Error(QString)), SLOT(ReportError(QString)));
  connect(player_, SIGNAL(Paused()), SLOT(MediaPaused()));
  connect(player_, SIGNAL(Playing()), SLOT(MediaPlaying()));
  connect(player_, SIGNAL(Stopped()), SLOT(MediaStopped()));

  connect(player_, SIGNAL(Paused()), playlist_, SLOT(Paused()));
  connect(player_, SIGNAL(Playing()), playlist_, SLOT(Playing()));
  connect(player_, SIGNAL(Stopped()), playlist_, SLOT(Stopped()));

  connect(player_, SIGNAL(Paused()), ui_.playlist, SLOT(StopGlowing()));
  connect(player_, SIGNAL(Playing()), ui_.playlist, SLOT(StartGlowing()));
  connect(player_, SIGNAL(Stopped()), ui_.playlist, SLOT(StopGlowing()));

  connect(player_, SIGNAL(Paused()), osd_, SLOT(Paused()));
  connect(player_, SIGNAL(Stopped()), osd_, SLOT(Stopped()));
  connect(player_, SIGNAL(VolumeChanged(int)), osd_, SLOT(VolumeChanged(int)));
  connect(playlist_, SIGNAL(CurrentSongChanged(Song)), osd_, SLOT(SongChanged(Song)));

  connect(ui_.playlist, SIGNAL(doubleClicked(QModelIndex)), SLOT(PlayIndex(QModelIndex)));
  connect(ui_.playlist, SIGNAL(PlayPauseItem(QModelIndex)), SLOT(PlayIndex(QModelIndex)));
  connect(ui_.playlist, SIGNAL(RightClicked(QPoint,QModelIndex)), SLOT(PlaylistRightClick(QPoint,QModelIndex)));

  connect(track_slider_, SIGNAL(ValueChanged(int)), player_, SLOT(Seek(int)));

  // Library connections
  connect(library_, SIGNAL(Error(QString)), SLOT(ReportError(QString)));
  connect(ui_.library_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(LibraryDoubleClick(QModelIndex)));
  connect(ui_.library_view, SIGNAL(ShowConfigDialog()), library_, SLOT(ShowConfig()));
  connect(library_, SIGNAL(TotalSongCountUpdated(int)), ui_.library_view, SLOT(TotalSongCountUpdated(int)));
  connect(library_, SIGNAL(ScanStarted()), SLOT(LibraryScanStarted()));
  connect(library_, SIGNAL(ScanFinished()), SLOT(LibraryScanFinished()));

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
  library_menu->addAction("Configure library...", library_, SLOT(ShowConfig()));
  ui_.library_options->setMenu(library_menu);

  // Playlist menu
  playlist_play_pause_ = playlist_menu_->addAction("Play", this, SLOT(PlaylistPlay()));
  playlist_menu_->addAction(ui_.action_stop);
  playlist_stop_after_ = playlist_menu_->addAction(QIcon(":media-playback-stop.png"), "Stop after this track", this, SLOT(PlaylistStopAfter()));
  playlist_menu_->addAction(ui_.action_edit_track);
  playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_.action_clear_playlist);

  // Radio connections
  connect(radio_model_, SIGNAL(LoadingStarted()), ui_.playlist, SLOT(StartRadioLoading()));
  connect(radio_model_, SIGNAL(LoadingFinished()), ui_.playlist, SLOT(StopRadioLoading()));
  connect(radio_model_, SIGNAL(StreamError(QString)), SLOT(ReportError(QString)));
  connect(radio_model_, SIGNAL(StreamFinished()), player_, SLOT(Next()));
  connect(radio_model_, SIGNAL(StreamReady(QUrl,QUrl)), player_, SLOT(StreamReady(QUrl,QUrl)));
  connect(radio_model_, SIGNAL(StreamMetadataFound(QUrl,Song)), playlist_, SLOT(SetStreamMetadata(QUrl,Song)));
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
  tray_menu->addAction(ui_.action_quit);
  tray_icon_->setContextMenu(tray_menu);

  connect(tray_icon_, SIGNAL(WheelEvent(int)), SLOT(VolumeWheelEvent(int)));
  connect(tray_icon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(TrayClicked(QSystemTrayIcon::ActivationReason)));

  // Global shortcuts
  QxtGlobalShortcut* play_pause = new QxtGlobalShortcut(QKeySequence("Media Play"), this);
  QxtGlobalShortcut* stop = new QxtGlobalShortcut(QKeySequence("Media Stop"), this);
  QxtGlobalShortcut* next = new QxtGlobalShortcut(QKeySequence("Media Next"), this);
  QxtGlobalShortcut* prev = new QxtGlobalShortcut(QKeySequence("Media Previous"), this);
  connect(play_pause, SIGNAL(activated()), ui_.action_play_pause, SLOT(trigger()));
  connect(stop, SIGNAL(activated()), ui_.action_stop, SLOT(trigger()));
  connect(next, SIGNAL(activated()), ui_.action_next_track, SLOT(trigger()));
  connect(prev, SIGNAL(activated()), ui_.action_previous_track, SLOT(trigger()));

  // Analyzer
  ui_.analyzer->set_engine(player_->GetEngine());

  // Statusbar widgets
  ui_.statusBar->addPermanentWidget(track_slider_);
  ui_.statusBar->addWidget(multi_loading_indicator_);
  multi_loading_indicator_->hide();

  // Load theme
  QFile stylesheet(":mainwindow.css");
  if (!stylesheet.open(QIODevice::ReadOnly)) {
    qWarning("Could not open stylesheet");
  } else {
    setStyleSheet(stylesheet.readAll());
  }

  // Load settings
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  restoreGeometry(settings.value("geometry").toByteArray());
  if (!restoreState(settings.value("state").toByteArray(), kStateVersion)) {
    tabifyDockWidget(ui_.files_dock, ui_.radio_dock);
    tabifyDockWidget(ui_.files_dock, ui_.library_dock);
  }

  ui_.file_view->SetPath(settings.value("file_path", QDir::homePath()).toString());

  if (!settings.value("hidden", false).toBool())
    show();

  library_->StartThreads();
}

MainWindow::~MainWindow() {
  SaveGeometry();
}

void MainWindow::QueueFiles(const QList<QUrl>& urls) {
  QModelIndex playlist_index = playlist_->InsertPaths(urls);

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
  ui_.action_play_pause->setText("Play");

  ui_.action_play_pause->setEnabled(true);

  ui_.action_ban->setEnabled(false);
  ui_.action_love->setEnabled(false);

  track_position_timer_->stop();
  track_slider_->SetStopped();
}

void MainWindow::MediaPaused() {
  ui_.action_stop->setEnabled(true);
  ui_.action_stop_after_this_track->setEnabled(true);
  ui_.action_play_pause->setIcon(QIcon(":media-playback-start.png"));
  ui_.action_play_pause->setText("Play");

  ui_.action_play_pause->setEnabled(true);

  track_position_timer_->stop();
}

void MainWindow::MediaPlaying() {
  ui_.action_stop->setEnabled(true);
  ui_.action_stop_after_this_track->setEnabled(true);
  ui_.action_play_pause->setIcon(QIcon(":media-playback-pause.png"));
  ui_.action_play_pause->setText("Pause");

  ui_.action_play_pause->setEnabled(
      ! (playlist_->current_item_options() & PlaylistItem::PauseDisabled));

  bool is_lastfm = (playlist_->current_item_options() & PlaylistItem::LastFMControls);
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

  bool is_lastfm = (playlist_->current_item_options() & PlaylistItem::LastFMControls);
  ui_.action_ban->setEnabled(value && is_lastfm);
  ui_.action_love->setEnabled(value);
}

void MainWindow::resizeEvent(QResizeEvent*) {
  SaveGeometry();
}

void MainWindow::SaveGeometry() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("geometry", saveGeometry());
  settings.setValue("state", saveState(kStateVersion));
}

void MainWindow::PlayIndex(const QModelIndex& index) {
  if (!index.isValid())
    return;

  player_->PlayAt(index.row());
}

void MainWindow::LibraryDoubleClick(const QModelIndex& index) {
  QModelIndex first_song =
      playlist_->InsertSongs(library_->GetChildSongs(
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
  playlist_->StopAfter(playlist_->current_index());
}

void MainWindow::closeEvent(QCloseEvent* event) {
  event->ignore();
  SetHiddenInTray(true);
}

void MainWindow::SetHiddenInTray(bool hidden) {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("hidden", hidden);

  if (hidden)
    hide();
  else
    show();
}

void MainWindow::ClearLibraryFilter() {
  ui_.library_filter->clear();
  ui_.library_filter->setFocus();
}

void MainWindow::FilePathChanged(const QString& path) {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("file_path", path);
}

void MainWindow::UpdateTrackPosition() {
  int position = std::floor(float(player_->GetEngine()->position()) / 1000.0 + 0.5);

  LastFMService* lastfm = radio_model_->GetLastFMService();

  if (!playlist_->has_scrobbled() &&
      position >= playlist_->scrobble_point()) {
    lastfm->Scrobble();
    playlist_->set_scrobbled(true);
  }

  track_slider_->SetValue(position, playlist_->current_item()->Metadata().length());
}

void MainWindow::Love() {
  radio_model_->GetLastFMService()->Love();
  ui_.action_love->setEnabled(false);
}

void MainWindow::RadioDoubleClick(const QModelIndex& index) {
  InsertRadioItem(radio_model_->IndexToItem(index));
}

void MainWindow::InsertRadioItem(RadioItem* item) {
  QModelIndex first_song = playlist_->InsertRadioStations(
      QList<RadioItem*>() << item);

  if (first_song.isValid() && player_->GetState() != Engine::Playing)
    player_->PlayAt(first_song.row());
}

void MainWindow::PlaylistRightClick(const QPoint& global_pos, const QModelIndex& index) {
  playlist_menu_index_ = index;

  if (playlist_->current_index() == index.row()) {
    playlist_play_pause_->setText("Pause");
    playlist_play_pause_->setIcon(QIcon(":media-playback-pause.png"));
  } else {
    playlist_play_pause_->setText("Play");
    playlist_play_pause_->setIcon(QIcon(":media-playback-start.png"));
  }

  playlist_play_pause_->setEnabled(index.isValid());
  playlist_stop_after_->setEnabled(index.isValid());

  // Are any of the selected songs editable?
  bool editable = false;
  foreach (const QModelIndex& index,
           ui_.playlist->selectionModel()->selection().indexes()) {
    if (index.column() != 0)
      continue;
    if (playlist_->item_at(index.row())->Metadata().IsEditable()) {
      editable = true;
      break;
    }
  }
  ui_.action_edit_track->setEnabled(editable);

  playlist_menu_->popup(global_pos);
}

void MainWindow::PlaylistPlay() {
  if (playlist_->current_index() == playlist_menu_index_.row()) {
    player_->PlayPause();
  } else {
    player_->PlayAt(playlist_menu_index_.row());
  }
}

void MainWindow::PlaylistStopAfter() {
  playlist_->StopAfter(playlist_menu_index_.row());
}

void MainWindow::EditTracks() {
  SongList songs;
  QList<int> rows;

  foreach (const QModelIndex& index,
           ui_.playlist->selectionModel()->selection().indexes()) {
    if (index.column() != 0)
      continue;
    Song song = playlist_->item_at(index.row())->Metadata();

    if (song.IsEditable()) {
      songs << song;
      rows << index.row();
    }
  }

  edit_tag_dialog_->SetSongs(songs);
  if (edit_tag_dialog_->exec() == QDialog::Rejected)
    return;

  playlist_->ReloadItems(rows);
}

void MainWindow::LibraryScanStarted() {
  multi_loading_indicator_->TaskStarted("Updating library");
}

void MainWindow::LibraryScanFinished() {
  multi_loading_indicator_->TaskFinished("Updating library");
}
