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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>
#include <memory>

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
#include <QTimer>
#include <QUndoStack>
#include <QtDebug>

#ifdef Q_OS_WIN32
#include <qtsparkle/Updater>
#endif

#include "core/appearance.h"
#include "core/application.h"
#include "core/backgroundstreams.h"
#include "core/commandlineoptions.h"
#include "core/database.h"
#include "core/deletefiles.h"
#include "core/filesystemmusicstorage.h"
#include "core/globalshortcuts.h"
#include "core/logging.h"
#include "core/mac_startup.h"
#include "core/mergedproxymodel.h"
#include "core/mimedata.h"
#include "core/mpris_common.h"
#include "core/network.h"
#include "core/player.h"
#include "core/songloader.h"
#include "core/stylesheetloader.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "devices/devicemanager.h"
#include "devices/devicestatefiltermodel.h"
#include "devices/deviceview.h"
#include "devices/deviceviewcontainer.h"
#include "engines/enginebase.h"
#include "engines/gstengine.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/globalsearchview.h"
#include "globalsearch/librarysearchprovider.h"
#include "internet/core/internetmodel.h"
#include "internet/core/internetview.h"
#include "internet/core/internetviewcontainer.h"
#include "internet/internetradio/savedradio.h"
#include "internet/magnatune/magnatuneservice.h"
#include "internet/podcasts/podcastservice.h"
#include "library/groupbydialog.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/librarydirectorymodel.h"
#include "library/libraryfilterwidget.h"
#include "library/libraryviewcontainer.h"
#include "musicbrainz/tagfetcher.h"
#include "networkremote/networkremote.h"
#include "playlist/playlist.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlistlistcontainer.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#include "playlist/playlistview.h"
#include "playlist/queue.h"
#include "playlist/queuemanager.h"
#include "playlist/songplaylistitem.h"
#include "playlistparsers/playlistparser.h"
#ifdef HAVE_AUDIOCD
#include "ripper/ripcddialog.h"
#endif
#include "smartplaylists/generator.h"
#include "smartplaylists/generatormimedata.h"
#include "songinfo/artistinfoview.h"
#include "songinfo/songinfoview.h"
#include "songinfo/streamdiscoverer.h"
#include "transcoder/transcodedialog.h"
#include "ui/about.h"
#include "ui/addstreamdialog.h"
#include "ui/albumcovermanager.h"
#include "ui/console.h"
#include "ui/edittagdialog.h"
#include "ui/equalizer.h"
#include "ui/iconloader.h"
#include "ui/organisedialog.h"
#include "ui/organiseerrordialog.h"
#include "ui/qtsystemtrayicon.h"
#include "ui/settingsdialog.h"
#include "ui/streamdetailsdialog.h"
#include "ui/systemtrayicon.h"
#include "ui/trackselectiondialog.h"
#include "ui/windows7thumbbar.h"
#include "version.h"
#include "widgets/errordialog.h"
#include "widgets/fileview.h"
#include "widgets/multiloadingindicator.h"
#include "widgets/osd.h"
#include "widgets/stylehelper.h"
#include "widgets/trackslider.h"

#ifdef Q_OS_DARWIN
#include "ui/macsystemtrayicon.h"
#endif

#ifdef HAVE_LIBLASTFM
#include "internet/lastfm/lastfmservice.h"
#endif

#ifdef HAVE_WIIMOTEDEV
#include "wiimotedev/shortcuts.h"
#endif

#ifdef ENABLE_VISUALISATIONS
#include "visualisations/visualisationcontainer.h"
#endif

#ifdef HAVE_MOODBAR
#include "moodbar/moodbarcontroller.h"
#include "moodbar/moodbarproxystyle.h"
#endif

#include <cmath>

#ifdef Q_OS_DARWIN
// Non exported mac-specific function.
void qt_mac_set_dock_menu(QMenu*);
#endif

const char* MainWindow::kSettingsGroup = "MainWindow";
const char* MainWindow::kAllFilesFilterSpec = QT_TR_NOOP("All Files (*)");

namespace {
const int kTrackSliderUpdateTimeMs = 40;
const int kTrackPositionUpdateTimeMs = 1000;
}

MainWindow::MainWindow(Application* app, SystemTrayIcon* tray_icon, OSD* osd,
                       const CommandlineOptions& options, QWidget* parent)
    : QMainWindow(parent),
      ui_(new Ui_MainWindow),
      thumbbar_(new Windows7ThumbBar(this)),
      app_(app),
      tray_icon_(tray_icon),
      osd_(osd),
      edit_tag_dialog_(std::bind(&MainWindow::CreateEditTagDialog, this)),
      stream_discoverer_(std::bind(&MainWindow::CreateStreamDiscoverer, this)),
      global_shortcuts_(new GlobalShortcuts(this)),
      global_search_view_(new GlobalSearchView(app_, this)),
      library_view_(new LibraryViewContainer(this)),
      file_view_(new FileView(this)),
      playlist_list_(new PlaylistListContainer(this)),
      internet_view_(new InternetViewContainer(this)),
      device_view_container_(new DeviceViewContainer(this)),
      device_view_(device_view_container_->view()),
      song_info_view_(new SongInfoView(this)),
      artist_info_view_(new ArtistInfoView(this)),
      settings_dialog_(std::bind(&MainWindow::CreateSettingsDialog, this)),
      add_stream_dialog_([=]() {
        AddStreamDialog* add_stream_dialog = new AddStreamDialog;
        connect(add_stream_dialog, SIGNAL(accepted()), this,
                SLOT(AddStreamAccepted()));
        add_stream_dialog->set_add_on_accept(
            InternetModel::Service<SavedRadio>());
        return add_stream_dialog;
      }),
      cover_manager_([=]() {
        AlbumCoverManager* cover_manager =
            new AlbumCoverManager(app, app->library_backend());
        cover_manager->Init();

        // Cover manager connections
        connect(cover_manager, SIGNAL(AddToPlaylist(QMimeData*)), this,
                SLOT(AddToPlaylist(QMimeData*)));
        return cover_manager;
      }),
      equalizer_(new Equalizer),
      organise_dialog_([=]() {
        OrganiseDialog* dialog = new OrganiseDialog(app->task_manager());
        dialog->SetDestinationModel(app->library()->model()->directory_model());
        return dialog;
      }),
      queue_manager_([=]() {
        QueueManager* manager = new QueueManager;
        manager->SetPlaylistManager(app->playlist_manager());
        return manager;
      }),
      playlist_menu_(new QMenu(this)),
      playlist_add_to_another_(nullptr),
      playlistitem_actions_separator_(nullptr),
      library_sort_model_(new QSortFilterProxyModel(this)),
      track_position_timer_(new QTimer(this)),
      track_slider_timer_(new QTimer(this)),
      saved_playback_position_(0),
      saved_playback_state_(Engine::Empty),
      doubleclick_addmode_(AddBehaviour_Append),
      doubleclick_playmode_(PlayBehaviour_IfStopped),
      menu_playmode_(PlayBehaviour_IfStopped) {
  qLog(Debug) << "Starting";

  connect(app, SIGNAL(ErrorAdded(QString)), SLOT(ShowErrorDialog(QString)));
  connect(app, SIGNAL(SettingsDialogRequested(SettingsDialog::Page)),
          SLOT(OpenSettingsDialogAtPage(SettingsDialog::Page)));

  // Initialise the UI
  ui_->setupUi(this);
#ifdef Q_OS_DARWIN
  ui_->menu_help->menuAction()->setVisible(false);
#endif

  ui_->multi_loading_indicator->SetTaskManager(app_->task_manager());
  ui_->now_playing->SetApplication(app_);

  int volume = app_->player()->GetVolume();
  ui_->volume->setValue(volume);
  VolumeChanged(volume);

  // Initialise the global search widget
  StyleHelper::setBaseColor(palette().color(QPalette::Highlight).darker());

  // Add global search providers
  app_->global_search()->AddProvider(new LibrarySearchProvider(
      app_->library_backend(), tr("Library"), "library",
      IconLoader::Load("folder-sound", IconLoader::Base), true, app_, this));

  connect(global_search_view_, SIGNAL(AddToPlaylist(QMimeData*)),
          SLOT(AddToPlaylist(QMimeData*)));

  // Add tabs to the fancy tab widget
  ui_->tabs->AddTab(global_search_view_,
                    IconLoader::Load("search", IconLoader::Base),
                    tr("Search", "Global search settings dialog title."));
  ui_->tabs->AddTab(library_view_,
                    IconLoader::Load("folder-sound", IconLoader::Base),
                    tr("Library"));
  ui_->tabs->AddTab(file_view_,
                    IconLoader::Load("document-open", IconLoader::Base),
                    tr("Files"));
  ui_->tabs->AddTab(playlist_list_,
                    IconLoader::Load("view-media-playlist", IconLoader::Base),
                    tr("Playlists"));
  ui_->tabs->AddTab(internet_view_,
                    IconLoader::Load("applications-internet", IconLoader::Base),
                    tr("Internet"));
  ui_->tabs->AddTab(
      device_view_container_,
      IconLoader::Load("multimedia-player-ipod-mini-blue", IconLoader::Base),
      tr("Devices"));
  ui_->tabs->AddSpacer();
  ui_->tabs->AddTab(song_info_view_,
                    IconLoader::Load("view-media-lyrics", IconLoader::Base),
                    tr("Song info"));
  ui_->tabs->AddTab(artist_info_view_,
                    IconLoader::Load("x-clementine-artist", IconLoader::Base),
                    tr("Artist info"));

  // Add the now playing widget to the fancy tab widget
  ui_->tabs->AddBottomWidget(ui_->now_playing);

  ui_->tabs->SetBackgroundPixmap(QPixmap(":/sidebar_background.png"));

  track_position_timer_->setInterval(kTrackPositionUpdateTimeMs);
  connect(track_position_timer_, SIGNAL(timeout()),
          SLOT(UpdateTrackPosition()));
  track_slider_timer_->setInterval(kTrackSliderUpdateTimeMs);
  connect(track_slider_timer_, SIGNAL(timeout()),
          SLOT(UpdateTrackSliderPosition()));

  // Start initialising the player
  qLog(Debug) << "Initialising player";
  app_->player()->Init();
  background_streams_ = new BackgroundStreams(app_->player()->engine(), this);
  background_streams_->LoadStreams();

  // Models
  qLog(Debug) << "Creating models";
  library_sort_model_->setSourceModel(app_->library()->model());
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->setSortLocaleAware(true);
  library_sort_model_->sort(0);

  connect(ui_->playlist, SIGNAL(ViewSelectionModelChanged()),
          SLOT(PlaylistViewSelectionModelChanged()));
  ui_->playlist->SetManager(app_->playlist_manager());
  ui_->playlist->view()->SetApplication(app_);

  library_view_->view()->setModel(library_sort_model_);
  library_view_->view()->SetApplication(app_);
  internet_view_->SetApplication(app_);
  device_view_->SetApplication(app_);
  playlist_list_->SetApplication(app_);

  // Icons
  qLog(Debug) << "Creating UI";
  ui_->action_about->setIcon(IconLoader::Load("help-about", IconLoader::Base));
  ui_->action_about_qt->setIcon(IconLoader::Load("qtlogo", IconLoader::Base));
  ui_->action_add_file->setIcon(
      IconLoader::Load("document-open", IconLoader::Base));
  ui_->action_add_folder->setIcon(
      IconLoader::Load("document-open-folder", IconLoader::Base));
  ui_->action_add_stream->setIcon(
      IconLoader::Load("document-open-remote", IconLoader::Base));
  ui_->action_add_podcast->setIcon(
      IconLoader::Load("podcast", IconLoader::Provider));
  ui_->action_clear_playlist->setIcon(
      IconLoader::Load("edit-clear-list", IconLoader::Base));
  ui_->action_configure->setIcon(
      IconLoader::Load("configure", IconLoader::Base));
  ui_->action_cover_manager->setIcon(
      IconLoader::Load("download", IconLoader::Base));
  ui_->action_edit_track->setIcon(
      IconLoader::Load("edit-rename", IconLoader::Base));
  ui_->action_equalizer->setIcon(
      IconLoader::Load("view-media-equalizer", IconLoader::Base));
  ui_->action_jump->setIcon(IconLoader::Load("go-jump", IconLoader::Base));
  ui_->action_next_track->setIcon(
      IconLoader::Load("media-skip-forward", IconLoader::Base));
  ui_->action_open_media->setIcon(
      IconLoader::Load("document-open", IconLoader::Base));
  ui_->action_open_cd->setIcon(
      IconLoader::Load("media-optical", IconLoader::Base));
  ui_->action_play_pause->setIcon(
      IconLoader::Load("media-playback-start", IconLoader::Base));
  ui_->action_previous_track->setIcon(
      IconLoader::Load("media-skip-backward", IconLoader::Base));
  ui_->action_mute->setIcon(
      IconLoader::Load("audio-volume-muted", IconLoader::Base));
  ui_->action_quit->setIcon(
      IconLoader::Load("application-exit", IconLoader::Base));
  ui_->action_remove_from_playlist->setIcon(
      IconLoader::Load("list-remove", IconLoader::Base));
  ui_->action_repeat_mode->setIcon(
      IconLoader::Load("media-playlist-repeat", IconLoader::Base));
  ui_->action_rip_audio_cd->setIcon(
      IconLoader::Load("media-optical", IconLoader::Base));
  ui_->action_shuffle->setIcon(
      IconLoader::Load("x-clementine-shuffle", IconLoader::Base));
  ui_->action_shuffle_mode->setIcon(
      IconLoader::Load("media-playlist-shuffle", IconLoader::Base));
  ui_->action_stop->setIcon(
      IconLoader::Load("media-playback-stop", IconLoader::Base));
  ui_->action_stop_after_this_track->setIcon(
      IconLoader::Load("media-playback-stop", IconLoader::Base));
  ui_->action_new_playlist->setIcon(
      IconLoader::Load("document-new", IconLoader::Base));
  ui_->action_load_playlist->setIcon(
      IconLoader::Load("document-open", IconLoader::Base));
  ui_->action_save_playlist->setIcon(
      IconLoader::Load("document-save", IconLoader::Base));
  ui_->action_full_library_scan->setIcon(
      IconLoader::Load("view-refresh", IconLoader::Base));
  ui_->action_rain->setIcon(
      IconLoader::Load("weather-showers-scattered", IconLoader::Base));
  ui_->action_hypnotoad->setIcon(
      IconLoader::Load("hypnotoad", IconLoader::Base));
  ui_->action_kittens->setIcon(IconLoader::Load("kittens", IconLoader::Base));
  ui_->action_enterprise->setIcon(
      IconLoader::Load("enterprise", IconLoader::Base));
  ui_->action_love->setIcon(IconLoader::Load("love", IconLoader::Lastfm));

  // File view connections
  connect(file_view_, SIGNAL(AddToPlaylist(QMimeData*)),
          SLOT(AddToPlaylist(QMimeData*)));
  connect(file_view_, SIGNAL(PathChanged(QString)),
          SLOT(FilePathChanged(QString)));
  connect(file_view_, SIGNAL(CopyToLibrary(QList<QUrl>)),
          SLOT(CopyFilesToLibrary(QList<QUrl>)));
  connect(file_view_, SIGNAL(MoveToLibrary(QList<QUrl>)),
          SLOT(MoveFilesToLibrary(QList<QUrl>)));
  connect(file_view_, SIGNAL(EditTags(QList<QUrl>)),
          SLOT(EditFileTags(QList<QUrl>)));
  connect(file_view_, SIGNAL(CopyToDevice(QList<QUrl>)),
          SLOT(CopyFilesToDevice(QList<QUrl>)));
  file_view_->SetTaskManager(app_->task_manager());

  // Action connections
  connect(ui_->action_next_track, SIGNAL(triggered()), app_->player(),
          SLOT(Next()));
  connect(ui_->action_previous_track, SIGNAL(triggered()), app_->player(),
          SLOT(Previous()));
  connect(ui_->action_play_pause, SIGNAL(triggered()), app_->player(),
          SLOT(PlayPause()));
  connect(ui_->action_stop, SIGNAL(triggered()), app_->player(), SLOT(Stop()));
  connect(ui_->action_quit, SIGNAL(triggered()), SLOT(Exit()));
  connect(ui_->action_stop_after_this_track, SIGNAL(triggered()),
          SLOT(StopAfterCurrent()));
  connect(ui_->action_mute, SIGNAL(triggered()), app_->player(), SLOT(Mute()));
#ifdef HAVE_LIBLASTFM
  connect(ui_->action_love, SIGNAL(triggered()), SLOT(Love()));
  connect(ui_->action_toggle_scrobbling, SIGNAL(triggered()), app_->scrobbler(),
          SLOT(ToggleScrobbling()));
#endif

  connect(ui_->action_clear_playlist, SIGNAL(triggered()),
          app_->playlist_manager(), SLOT(ClearCurrent()));
  connect(ui_->action_remove_duplicates, SIGNAL(triggered()),
          app_->playlist_manager(), SLOT(RemoveDuplicatesCurrent()));
  connect(ui_->action_remove_unavailable, SIGNAL(triggered()),
          app_->playlist_manager(), SLOT(RemoveUnavailableCurrent()));
  connect(ui_->action_remove_from_playlist, SIGNAL(triggered()),
          SLOT(PlaylistRemoveCurrent()));
  connect(ui_->action_toggle_show_sidebar, SIGNAL(toggled(bool)),
          ui_->sidebar_layout, SLOT(setShown(bool)));
  connect(ui_->action_edit_track, SIGNAL(triggered()), SLOT(EditTracks()));
  connect(ui_->action_renumber_tracks, SIGNAL(triggered()),
          SLOT(RenumberTracks()));
  connect(ui_->action_selection_set_value, SIGNAL(triggered()),
          SLOT(SelectionSetValue()));
  connect(ui_->action_edit_value, SIGNAL(triggered()), SLOT(EditValue()));
  connect(ui_->action_auto_complete_tags, SIGNAL(triggered()),
          SLOT(AutoCompleteTags()));
  connect(ui_->action_configure, SIGNAL(triggered()),
          SLOT(OpenSettingsDialog()));
  connect(ui_->action_about, SIGNAL(triggered()), SLOT(ShowAboutDialog()));
  connect(ui_->action_about_qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  connect(ui_->action_shuffle, SIGNAL(triggered()), app_->playlist_manager(),
          SLOT(ShuffleCurrent()));
  connect(ui_->action_open_media, SIGNAL(triggered()), SLOT(AddFile()));
  connect(ui_->action_open_cd, SIGNAL(triggered()), SLOT(AddCDTracks()));
#ifdef HAVE_AUDIOCD
  connect(ui_->action_rip_audio_cd, SIGNAL(triggered()),
          SLOT(OpenRipCDDialog()));
#else
  ui_->action_rip_audio_cd->setVisible(false);
#endif
  connect(ui_->action_add_file, SIGNAL(triggered()), SLOT(AddFile()));
  connect(ui_->action_add_folder, SIGNAL(triggered()), SLOT(AddFolder()));
  connect(ui_->action_add_stream, SIGNAL(triggered()), SLOT(AddStream()));
  connect(ui_->action_add_podcast, SIGNAL(triggered()), SLOT(AddPodcast()));
  connect(ui_->action_cover_manager, SIGNAL(triggered()),
          SLOT(ShowCoverManager()));
  connect(ui_->action_equalizer, SIGNAL(triggered()), equalizer_.get(),
          SLOT(show()));
  connect(ui_->action_transcode, SIGNAL(triggered()),
          SLOT(ShowTranscodeDialog()));
  connect(ui_->action_jump, SIGNAL(triggered()), ui_->playlist->view(),
          SLOT(JumpToCurrentlyPlayingTrack()));
  connect(ui_->action_update_library, SIGNAL(triggered()), app_->library(),
          SLOT(IncrementalScan()));
  connect(ui_->action_full_library_scan, SIGNAL(triggered()), app_->library(),
          SLOT(FullScan()));
  connect(ui_->action_queue_manager, SIGNAL(triggered()),
          SLOT(ShowQueueManager()));
  connect(ui_->action_add_files_to_transcoder, SIGNAL(triggered()),
          SLOT(AddFilesToTranscoder()));
  connect(ui_->action_view_stream_details, SIGNAL(triggered()),
          SLOT(DiscoverStreamDetails()));

  background_streams_->AddAction("Rain", ui_->action_rain);
  background_streams_->AddAction("Hypnotoad", ui_->action_hypnotoad);
  background_streams_->AddAction("Make it so!", ui_->action_enterprise);

  // Playlist view actions
  ui_->action_next_playlist->setShortcuts(
      QList<QKeySequence>()
      << QKeySequence::fromString("Ctrl+Tab")
#ifdef Q_OS_DARWIN
      // On OS X "Ctrl+Tab" == Cmd + Tab but this shorcut
      // is already used by default for switching between
      // applications.
      // I would have preferred to use Meta+Tab (which
      // means Ctrl+Tab on OS X), like in Firefox or
      // Chrome, but this doesn't work (probably at Qt bug)
      // and some applications (e.g. Qt creator) uses
      // Alt+Tab too so I believe it's a good shorcut anyway
      << QKeySequence::fromString("Alt+Tab")
#endif  // Q_OS_DARWIN
      << QKeySequence::fromString("Ctrl+PgDown"));
  ui_->action_previous_playlist->setShortcuts(
      QList<QKeySequence>() << QKeySequence::fromString("Ctrl+Shift+Tab")
#ifdef Q_OS_DARWIN
                            << QKeySequence::fromString("Alt+Shift+Tab")
#endif  // Q_OS_DARWIN
                            << QKeySequence::fromString("Ctrl+PgUp"));
  // Actions for switching tabs will be global to the entire window, so adding
  // them here
  addAction(ui_->action_next_playlist);
  addAction(ui_->action_previous_playlist);

  // Give actions to buttons
  ui_->forward_button->setDefaultAction(ui_->action_next_track);
  ui_->back_button->setDefaultAction(ui_->action_previous_track);
  ui_->pause_play_button->setDefaultAction(ui_->action_play_pause);
  ui_->stop_button->setDefaultAction(ui_->action_stop);
  ui_->love_button->setDefaultAction(ui_->action_love);
  ui_->scrobbling_button->setDefaultAction(ui_->action_toggle_scrobbling);
  ui_->clear_playlist_button->setDefaultAction(ui_->action_clear_playlist);
  ui_->playlist->SetActions(
      ui_->action_new_playlist, ui_->action_load_playlist,
      ui_->action_save_playlist,
      ui_->action_next_playlist, /* These two actions aren't associated */
      ui_->action_previous_playlist /* to a button but to the main window */);

#ifdef ENABLE_VISUALISATIONS
  connect(ui_->action_visualisations, SIGNAL(triggered()),
          SLOT(ShowVisualisations()));
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
  connect(ui_->volume, SIGNAL(valueChanged(int)), app_->player(),
          SLOT(SetVolume(int)));

  connect(app_->player(), SIGNAL(Error(QString)),
          SLOT(ShowErrorDialog(QString)));
  connect(app_->player(), SIGNAL(SongChangeRequestProcessed(QUrl, bool)),
          app_->playlist_manager(),
          SLOT(SongChangeRequestProcessed(QUrl, bool)));

  connect(app_->player(), SIGNAL(Paused()), SLOT(MediaPaused()));
  connect(app_->player(), SIGNAL(Playing()), SLOT(MediaPlaying()));
  connect(app_->player(), SIGNAL(Stopped()), SLOT(MediaStopped()));
  connect(app_->player(), SIGNAL(Seeked(qlonglong)), SLOT(Seeked(qlonglong)));
  connect(app_->player(), SIGNAL(TrackSkipped(PlaylistItemPtr)),
          SLOT(TrackSkipped(PlaylistItemPtr)));
  connect(this, SIGNAL(IntroPointReached()), app_->player(),
          SLOT(IntroPointReached()));
  connect(app_->player(), SIGNAL(VolumeChanged(int)), SLOT(VolumeChanged(int)));

  connect(app_->player(), SIGNAL(Paused()), ui_->playlist,
          SLOT(ActivePaused()));
  connect(app_->player(), SIGNAL(Playing()), ui_->playlist,
          SLOT(ActivePlaying()));
  connect(app_->player(), SIGNAL(Stopped()), ui_->playlist,
          SLOT(ActiveStopped()));

  connect(app_->player(), SIGNAL(Paused()), osd_, SLOT(Paused()));
  connect(app_->player(), SIGNAL(Stopped()), osd_, SLOT(Stopped()));
  connect(app_->player(), SIGNAL(PlaylistFinished()), osd_,
          SLOT(PlaylistFinished()));
  connect(app_->player(), SIGNAL(VolumeChanged(int)), osd_,
          SLOT(VolumeChanged(int)));
  connect(app_->player(), SIGNAL(VolumeChanged(int)), ui_->volume,
          SLOT(setValue(int)));
  connect(app_->player(), SIGNAL(ForceShowOSD(Song, bool)),
          SLOT(ForceShowOSD(Song, bool)));
  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          SLOT(SongChanged(Song)));
  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          app_->player(), SLOT(CurrentMetadataChanged(Song)));
  connect(app_->playlist_manager(), SIGNAL(EditingFinished(QModelIndex)),
          SLOT(PlaylistEditFinished(QModelIndex)));
  connect(app_->playlist_manager(), SIGNAL(Error(QString)),
          SLOT(ShowErrorDialog(QString)));
  connect(app_->playlist_manager(), SIGNAL(SummaryTextChanged(QString)),
          ui_->playlist_summary, SLOT(setText(QString)));
  connect(app_->playlist_manager(), SIGNAL(PlayRequested(QModelIndex)),
          SLOT(PlayIndex(QModelIndex)));

  connect(ui_->playlist->view(), SIGNAL(doubleClicked(QModelIndex)),
          SLOT(PlaylistDoubleClick(QModelIndex)));
  connect(ui_->playlist->view(), SIGNAL(PlayItem(QModelIndex)),
          SLOT(PlayIndex(QModelIndex)));
  connect(ui_->playlist->view(), SIGNAL(PlayPause()), app_->player(),
          SLOT(PlayPause()));
  connect(ui_->playlist->view(), SIGNAL(RightClicked(QPoint, QModelIndex)),
          SLOT(PlaylistRightClick(QPoint, QModelIndex)));
  connect(ui_->playlist->view(), SIGNAL(SeekForward()), app_->player(),
          SLOT(SeekForward()));
  connect(ui_->playlist->view(), SIGNAL(SeekBackward()), app_->player(),
          SLOT(SeekBackward()));
  connect(ui_->playlist->view(), SIGNAL(BackgroundPropertyChanged()),
          SLOT(RefreshStyleSheet()));

  connect(ui_->track_slider, SIGNAL(ValueChangedSeconds(int)), app_->player(),
          SLOT(SeekTo(int)));
  connect(ui_->track_slider, SIGNAL(SeekForward()), app_->player(),
          SLOT(SeekForward()));
  connect(ui_->track_slider, SIGNAL(SeekBackward()), app_->player(),
          SLOT(SeekBackward()));

  // Library connections
  connect(library_view_->view(), SIGNAL(AddToPlaylistSignal(QMimeData*)),
          SLOT(AddToPlaylist(QMimeData*)));
  connect(library_view_->view(), SIGNAL(ShowConfigDialog()),
          SLOT(ShowLibraryConfig()));
  connect(app_->library_model(), SIGNAL(TotalSongCountUpdated(int)),
          library_view_->view(), SLOT(TotalSongCountUpdated(int)));
  connect(app_->library_model(), SIGNAL(modelAboutToBeReset()),
          library_view_->view(), SLOT(SaveFocus()));
  connect(app_->library_model(), SIGNAL(modelReset()), library_view_->view(),
          SLOT(RestoreFocus()));

  connect(app_->task_manager(), SIGNAL(PauseLibraryWatchers()), app_->library(),
          SLOT(PauseWatcher()));
  connect(app_->task_manager(), SIGNAL(ResumeLibraryWatchers()),
          app_->library(), SLOT(ResumeWatcher()));

  // Devices connections
  connect(device_view_, SIGNAL(AddToPlaylistSignal(QMimeData*)),
          SLOT(AddToPlaylist(QMimeData*)));

  // Library filter widget
  QActionGroup* library_view_group = new QActionGroup(this);

  library_show_all_ = library_view_group->addAction(tr("Show all songs"));
  library_show_duplicates_ =
      library_view_group->addAction(tr("Show only duplicates"));
  library_show_untagged_ =
      library_view_group->addAction(tr("Show only untagged"));

  library_show_all_->setCheckable(true);
  library_show_duplicates_->setCheckable(true);
  library_show_untagged_->setCheckable(true);
  library_show_all_->setChecked(true);

  connect(library_view_group, SIGNAL(triggered(QAction*)),
          SLOT(ChangeLibraryQueryMode(QAction*)));

  QAction* library_config_action =
      new QAction(IconLoader::Load("configure", IconLoader::Base),
                  tr("Configure library..."), this);
  connect(library_config_action, SIGNAL(triggered()),
          SLOT(ShowLibraryConfig()));
  library_view_->filter()->SetSettingsGroup(kSettingsGroup);
  library_view_->filter()->SetLibraryModel(app_->library()->model());

  QAction* separator = new QAction(this);
  separator->setSeparator(true);

  library_view_->filter()->AddMenuAction(library_show_all_);
  library_view_->filter()->AddMenuAction(library_show_duplicates_);
  library_view_->filter()->AddMenuAction(library_show_untagged_);
  library_view_->filter()->AddMenuAction(separator);
  library_view_->filter()->AddMenuAction(library_config_action);

  // Playlist menu
  playlist_play_pause_ =
      playlist_menu_->addAction(tr("Play"), this, SLOT(PlaylistPlay()));
  playlist_menu_->addAction(ui_->action_stop);
  playlist_stop_after_ = playlist_menu_->addAction(
      IconLoader::Load("media-playback-stop", IconLoader::Base),
      tr("Stop after this track"), this, SLOT(PlaylistStopAfter()));
  playlist_queue_ = playlist_menu_->addAction("", this, SLOT(PlaylistQueue()));
  playlist_queue_->setShortcut(QKeySequence("Ctrl+D"));
  ui_->playlist->addAction(playlist_queue_);
  playlist_skip_ = playlist_menu_->addAction("", this, SLOT(PlaylistSkip()));
  ui_->playlist->addAction(playlist_skip_);

  playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_remove_from_playlist);
  playlist_undoredo_ = playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_edit_track);
  playlist_menu_->addAction(ui_->action_view_stream_details);
  playlist_menu_->addAction(ui_->action_edit_value);
  playlist_menu_->addAction(ui_->action_renumber_tracks);
  playlist_menu_->addAction(ui_->action_selection_set_value);
  playlist_menu_->addAction(ui_->action_auto_complete_tags);
  playlist_menu_->addAction(ui_->action_add_files_to_transcoder);
  playlist_menu_->addSeparator();
  playlist_copy_to_library_ = playlist_menu_->addAction(
      IconLoader::Load("edit-copy", IconLoader::Base), tr("Copy to library..."),
      this, SLOT(PlaylistCopyToLibrary()));
  playlist_move_to_library_ = playlist_menu_->addAction(
      IconLoader::Load("go-jump", IconLoader::Base), tr("Move to library..."),
      this, SLOT(PlaylistMoveToLibrary()));
  playlist_organise_ = playlist_menu_->addAction(
      IconLoader::Load("edit-copy", IconLoader::Base), tr("Organise files..."),
      this, SLOT(PlaylistMoveToLibrary()));
  playlist_copy_to_device_ = playlist_menu_->addAction(
      IconLoader::Load("multimedia-player-ipod-mini-blue", IconLoader::Base),
      tr("Copy to device..."), this, SLOT(PlaylistCopyToDevice()));
  playlist_delete_ = playlist_menu_->addAction(
      IconLoader::Load("edit-delete", IconLoader::Base),
      tr("Delete from disk..."), this, SLOT(PlaylistDelete()));
  playlist_open_in_browser_ = playlist_menu_->addAction(
      IconLoader::Load("document-open-folder", IconLoader::Base),
      tr("Show in file browser..."), this, SLOT(PlaylistOpenInBrowser()));
  playlist_show_in_library_ = playlist_menu_->addAction(
      IconLoader::Load("edit-find", IconLoader::Base), tr("Show in library..."),
      this, SLOT(ShowInLibrary()));
  playlist_menu_->addSeparator();
  playlistitem_actions_separator_ = playlist_menu_->addSeparator();
  playlist_menu_->addAction(ui_->action_clear_playlist);
  playlist_menu_->addAction(ui_->action_shuffle);
  playlist_menu_->addAction(ui_->action_remove_duplicates);
  playlist_menu_->addAction(ui_->action_remove_unavailable);

#ifdef Q_OS_DARWIN
  ui_->action_shuffle->setShortcut(QKeySequence());
#endif

  // We have to add the actions on the playlist menu to this QWidget otherwise
  // their shortcut keys don't work
  addActions(playlist_menu_->actions());

  connect(ui_->playlist, SIGNAL(UndoRedoActionsChanged(QAction*, QAction*)),
          SLOT(PlaylistUndoRedoChanged(QAction*, QAction*)));

  playlist_copy_to_device_->setDisabled(
      app_->device_manager()->connected_devices_model()->rowCount() == 0);
  connect(app_->device_manager()->connected_devices_model(),
          SIGNAL(IsEmptyChanged(bool)), playlist_copy_to_device_,
          SLOT(setDisabled(bool)));

  // Global search shortcut
  QAction* global_search_action = new QAction(this);
  global_search_action->setShortcuts(QList<QKeySequence>()
                                     << QKeySequence("Ctrl+F")
                                     << QKeySequence("Ctrl+L"));
  addAction(global_search_action);
  connect(global_search_action, SIGNAL(triggered()),
          SLOT(FocusGlobalSearchField()));

  // Internet connections
  connect(app_->internet_model(), SIGNAL(StreamError(QString)),
          SLOT(ShowErrorDialog(QString)));
  connect(app_->internet_model(), SIGNAL(StreamMetadataFound(QUrl, Song)),
          app_->playlist_manager(), SLOT(SetActiveStreamMetadata(QUrl, Song)));
  connect(app_->internet_model(), SIGNAL(AddToPlaylist(QMimeData*)),
          SLOT(AddToPlaylist(QMimeData*)));
  connect(app_->internet_model(), SIGNAL(ScrollToIndex(QModelIndex)),
          SLOT(ScrollToInternetIndex(QModelIndex)));
#ifdef HAVE_LIBLASTFM
  connect(app_->scrobbler(), SIGNAL(ButtonVisibilityChanged(bool)),
          SLOT(LastFMButtonVisibilityChanged(bool)));
  connect(app_->scrobbler(), SIGNAL(ScrobbleButtonVisibilityChanged(bool)),
          SLOT(ScrobbleButtonVisibilityChanged(bool)));
  connect(app_->scrobbler(), SIGNAL(ScrobblingEnabledChanged(bool)),
          SLOT(ScrobblingEnabledChanged(bool)));
  connect(app_->scrobbler(), SIGNAL(ScrobbledRadioStream()),
          SLOT(ScrobbledRadioStream()));
#endif
  connect(app_->internet_model()->Service<MagnatuneService>(),
          SIGNAL(DownloadFinished(QStringList)), osd_,
          SLOT(MagnatuneDownloadFinished(QStringList)));
  connect(internet_view_->tree(), SIGNAL(AddToPlaylistSignal(QMimeData*)),
          SLOT(AddToPlaylist(QMimeData*)));

  // Connections to the saved streams service
  connect(InternetModel::Service<SavedRadio>(), SIGNAL(ShowAddStreamDialog()),
          SLOT(AddStream()));

#ifdef Q_OS_DARWIN
  mac::SetApplicationHandler(this);
#endif
  // Tray icon
  tray_icon_->SetupMenu(ui_->action_previous_track, ui_->action_play_pause,
                        ui_->action_stop, ui_->action_stop_after_this_track,
                        ui_->action_next_track, ui_->action_mute,
                        ui_->action_love, ui_->action_quit);
  connect(tray_icon_, SIGNAL(PlayPause()), app_->player(), SLOT(PlayPause()));
  connect(tray_icon_, SIGNAL(SeekForward()), app_->player(),
          SLOT(SeekForward()));
  connect(tray_icon_, SIGNAL(SeekBackward()), app_->player(),
          SLOT(SeekBackward()));
  connect(tray_icon_, SIGNAL(NextTrack()), app_->player(), SLOT(Next()));
  connect(tray_icon_, SIGNAL(PreviousTrack()), app_->player(),
          SLOT(Previous()));
  connect(tray_icon_, SIGNAL(ShowHide()), SLOT(ToggleShowHide()));
  connect(tray_icon_, SIGNAL(ChangeVolume(int)), SLOT(VolumeWheelEvent(int)));

  // Windows 7 thumbbar buttons
  thumbbar_->SetActions(QList<QAction*>()
                        << ui_->action_previous_track << ui_->action_play_pause
                        << ui_->action_stop << ui_->action_next_track
                        << nullptr  // spacer
                        << ui_->action_love);

#if (defined(Q_OS_DARWIN) && defined(HAVE_SPARKLE)) || defined(Q_OS_WIN32)
  // Add check for updates item to application menu.
  QAction* check_updates =
      ui_->menu_tools->addAction(tr("Check for updates..."));
  check_updates->setMenuRole(QAction::ApplicationSpecificRole);
  connect(check_updates, SIGNAL(triggered(bool)), SLOT(CheckForUpdates()));
#endif

#ifdef Q_OS_WIN32
  qLog(Debug) << "Creating sparkle updater";
  qtsparkle::Updater* updater = new qtsparkle::Updater(
      QUrl("https://clementine-data.appspot.com/sparkle-windows"), this);
  updater->SetNetworkAccessManager(new NetworkAccessManager(this));
  updater->SetVersion(CLEMENTINE_VERSION_SPARKLE);
  connect(check_updates, SIGNAL(triggered()), updater, SLOT(CheckNow()));
#endif

  // Global shortcuts
  connect(global_shortcuts_, SIGNAL(Play()), app_->player(), SLOT(Play()));
  connect(global_shortcuts_, SIGNAL(Pause()), app_->player(), SLOT(Pause()));
  connect(global_shortcuts_, SIGNAL(PlayPause()), ui_->action_play_pause,
          SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(Stop()), ui_->action_stop, SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(StopAfter()),
          ui_->action_stop_after_this_track, SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(Next()), ui_->action_next_track,
          SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(Previous()), ui_->action_previous_track,
          SLOT(trigger()));
  connect(global_shortcuts_, SIGNAL(IncVolume()), app_->player(),
          SLOT(VolumeUp()));
  connect(global_shortcuts_, SIGNAL(DecVolume()), app_->player(),
          SLOT(VolumeDown()));
  connect(global_shortcuts_, SIGNAL(Mute()), app_->player(), SLOT(Mute()));
  connect(global_shortcuts_, SIGNAL(SeekForward()), app_->player(),
          SLOT(SeekForward()));
  connect(global_shortcuts_, SIGNAL(SeekBackward()), app_->player(),
          SLOT(SeekBackward()));
  connect(global_shortcuts_, SIGNAL(ShowHide()), SLOT(ToggleShowHide()));
  connect(global_shortcuts_, SIGNAL(ShowOSD()), app_->player(),
          SLOT(ShowOSD()));
  connect(global_shortcuts_, SIGNAL(TogglePrettyOSD()), app_->player(),
          SLOT(TogglePrettyOSD()));
#ifdef HAVE_LIBLASTFM
  connect(global_shortcuts_, SIGNAL(ToggleScrobbling()), app_->scrobbler(),
          SLOT(ToggleScrobbling()));
  connect(global_shortcuts_, SIGNAL(Love()), app_->scrobbler(), SLOT(Love()));
  connect(global_shortcuts_, SIGNAL(Ban()), app_->scrobbler(), SLOT(Ban()));
#endif

  connect(global_shortcuts_, SIGNAL(RateCurrentSong(int)),
          app_->playlist_manager(), SLOT(RateCurrentSong(int)));

  // Fancy tabs
  connect(ui_->tabs, SIGNAL(ModeChanged(FancyTabWidget::Mode)),
          SLOT(SaveGeometry()));
  connect(ui_->tabs, SIGNAL(CurrentChanged(int)), SLOT(SaveGeometry()));

  // Lyrics
  ConnectInfoView(song_info_view_);
  ConnectInfoView(artist_info_view_);

  // Analyzer
  ui_->analyzer->SetEngine(app_->player()->engine());
  ui_->analyzer->SetActions(ui_->action_visualisations);
  connect(ui_->analyzer, SIGNAL(WheelEvent(int)), SLOT(VolumeWheelEvent(int)));

  // Equalizer
  qLog(Debug) << "Creating equalizer";
  connect(equalizer_.get(), SIGNAL(ParametersChanged(int, QList<int>)),
          app_->player()->engine(),
          SLOT(SetEqualizerParameters(int, QList<int>)));
  connect(equalizer_.get(), SIGNAL(EnabledChanged(bool)),
          app_->player()->engine(), SLOT(SetEqualizerEnabled(bool)));
  connect(equalizer_.get(), SIGNAL(StereoBalanceChanged(float)),
          app_->player()->engine(), SLOT(SetStereoBalance(float)));
  app_->player()->engine()->SetEqualizerEnabled(equalizer_->is_enabled());
  app_->player()->engine()->SetEqualizerParameters(equalizer_->preamp_value(),
                                                   equalizer_->gain_values());
  app_->player()->engine()->SetStereoBalance(equalizer_->stereo_balance());

  // Statusbar widgets
  ui_->playlist_summary->setMinimumWidth(
      QFontMetrics(font()).width("WW selected of WW tracks - [ WW:WW ]"));
  ui_->status_bar_stack->setCurrentWidget(ui_->playlist_summary_page);
  connect(ui_->multi_loading_indicator, SIGNAL(TaskCountChange(int)),
          SLOT(TaskCountChanged(int)));

  ui_->track_slider->SetApplication(app);
#ifdef HAVE_MOODBAR
  // Moodbar connections
  connect(app_->moodbar_controller(),
          SIGNAL(CurrentMoodbarDataChanged(QByteArray)),
          ui_->track_slider->moodbar_style(), SLOT(SetMoodbarData(QByteArray)));
#endif

  // Now playing widget
  qLog(Debug) << "Creating now playing widget";
  ui_->now_playing->set_ideal_height(ui_->status_bar->sizeHint().height() +
                                     ui_->player_controls->sizeHint().height());
  connect(app_->player(), SIGNAL(Stopped()), ui_->now_playing, SLOT(Stopped()));
  connect(ui_->now_playing, SIGNAL(ShowAboveStatusBarChanged(bool)),
          SLOT(NowPlayingWidgetPositionChanged(bool)));
  connect(ui_->action_hypnotoad, SIGNAL(toggled(bool)), ui_->now_playing,
          SLOT(AllHail(bool)));
  connect(ui_->action_kittens, SIGNAL(toggled(bool)), ui_->now_playing,
          SLOT(EnableKittens(bool)));
  connect(ui_->action_kittens, SIGNAL(toggled(bool)), app_->network_remote(),
          SLOT(EnableKittens(bool)));
  // Hide the console
  // connect(ui_->action_console, SIGNAL(triggered()), SLOT(ShowConsole()));
  NowPlayingWidgetPositionChanged(ui_->now_playing->show_above_status_bar());

  // Load theme
  // This is tricky: we need to save the default/system palette now, before
  // loading user preferred theme (which will overide it), to be able to restore
  // it later
  const_cast<QPalette&>(Appearance::kDefaultPalette) = QApplication::palette();
  app_->appearance()->LoadUserTheme();
  StyleSheetLoader* css_loader = new StyleSheetLoader(this);
  css_loader->SetStyleSheet(this, ":mainwindow.css");

  // Load playlists
  app_->playlist_manager()->Init(app_->library_backend(),
                                 app_->playlist_backend(),
                                 ui_->playlist_sequence, ui_->playlist);

  // This connection must be done after the playlists have been initialized.
  connect(this, SIGNAL(StopAfterToggled(bool)), osd_,
          SLOT(StopAfterToggle(bool)));

  // We need to connect these global shortcuts here after the playlist have been
  // initialized
  connect(global_shortcuts_, SIGNAL(CycleShuffleMode()),
          app_->playlist_manager()->sequence(), SLOT(CycleShuffleMode()));
  connect(global_shortcuts_, SIGNAL(CycleRepeatMode()),
          app_->playlist_manager()->sequence(), SLOT(CycleRepeatMode()));
  connect(app_->playlist_manager()->sequence(),
          SIGNAL(RepeatModeChanged(PlaylistSequence::RepeatMode)), osd_,
          SLOT(RepeatModeChanged(PlaylistSequence::RepeatMode)));
  connect(app_->playlist_manager()->sequence(),
          SIGNAL(ShuffleModeChanged(PlaylistSequence::ShuffleMode)), osd_,
          SLOT(ShuffleModeChanged(PlaylistSequence::ShuffleMode)));

#ifdef HAVE_LIBLASTFM
  connect(app_->scrobbler(), SIGNAL(ScrobbleSubmitted()),
          SLOT(ScrobbleSubmitted()));
  connect(app_->scrobbler(), SIGNAL(ScrobbleError(int)),
          SLOT(ScrobbleError(int)));

  LastFMButtonVisibilityChanged(app_->scrobbler()->AreButtonsVisible());
  ScrobbleButtonVisibilityChanged(app_->scrobbler()->IsScrobbleButtonVisible());
  ScrobblingEnabledChanged(app_->scrobbler()->IsScrobblingEnabled());
#else
  LastFMButtonVisibilityChanged(false);
  ScrobbleButtonVisibilityChanged(false);
#endif

  // Load settings
  qLog(Debug) << "Loading settings";
  settings_.beginGroup(kSettingsGroup);

  // Set last used geometry to position window on the correct monitor
  // Set window state only if the window was last maximized
  was_maximized_ = settings_.value("maximized", false).toBool();
  restoreGeometry(settings_.value("geometry").toByteArray());
  if (was_maximized_) {
    setWindowState(windowState() | Qt::WindowMaximized);
  }

  if (!ui_->splitter->restoreState(
          settings_.value("splitter_state").toByteArray())) {
    ui_->splitter->setSizes(QList<int>() << 300 << width() - 300);
  }
  ui_->tabs->SetCurrentIndex(
      settings_.value("current_tab", 1 /* Library tab */).toInt());
  FancyTabWidget::Mode default_mode = FancyTabWidget::Mode_LargeSidebar;
  ui_->tabs->SetMode(
      FancyTabWidget::Mode(settings_.value("tab_mode", default_mode).toInt()));
  file_view_->SetPath(
      settings_.value("file_path", QDir::homePath()).toString());

  // Users often collapse one side of the splitter by mistake and don't know
  // how to restore it.  This must be set after the state is restored above.
  ui_->splitter->setChildrenCollapsible(false);

  ReloadSettings();

  // The "GlobalSearchView" requires that "InternetModel" has already been
  // initialised before reload settings.
  app_->global_search()->ReloadSettings();
  global_search_view_->ReloadSettings();

  // Reload pretty OSD to avoid issues with fonts
  osd_->ReloadPrettyOSDSettings();

  // Reload playlist settings, for BG and glowing
  ui_->playlist->view()->ReloadSettings();

#ifndef Q_OS_DARWIN
  StartupBehaviour behaviour = StartupBehaviour(
      settings_.value("startupbehaviour", Startup_Remember).toInt());
  bool hidden = settings_.value("hidden", false).toBool();

  switch (behaviour) {
    case Startup_AlwaysHide:
      hide();
      break;
    case Startup_AlwaysShow:
      show();
      break;
    case Startup_Remember:
      setVisible(!hidden);
      break;
  }

  // Force the window to show in case somehow the config has tray and window set
  // to hide
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

#ifdef HAVE_WIIMOTEDEV
  // http://code.google.com/p/clementine-player/issues/detail?id=670
  // Switched position, mayby something is not ready ?

  wiimotedev_shortcuts_.reset(
      new WiimotedevShortcuts(osd_, this, app_->player()));
#endif

  CheckFullRescanRevisions();

  CommandlineOptionsReceived(options);

  if (!options.contains_play_options()) LoadPlaybackStatus();

  qLog(Debug) << "Started";
}

MainWindow::~MainWindow() {
  SaveGeometry();
  delete ui_;
}

void MainWindow::ReloadSettings() {
#ifndef Q_OS_DARWIN
  bool show_tray = settings_.value("showtray", true).toBool();

  tray_icon_->SetVisible(show_tray);
  if (!show_tray && !isVisible()) show();
#endif

  QSettings s;
  s.beginGroup(kSettingsGroup);

  doubleclick_addmode_ =
      AddBehaviour(s.value("doubleclick_addmode", AddBehaviour_Append).toInt());
  doubleclick_playmode_ = PlayBehaviour(
      s.value("doubleclick_playmode", PlayBehaviour_IfStopped).toInt());
  doubleclick_playlist_addmode_ = PlaylistAddBehaviour(
      s.value("doubleclick_playlist_addmode", PlaylistAddBehaviour_Play)
          .toInt());
  menu_playmode_ =
      PlayBehaviour(s.value("menu_playmode", PlayBehaviour_IfStopped).toInt());

  bool show_sidebar = settings_.value("show_sidebar", true).toBool();
  ui_->sidebar_layout->setShown(show_sidebar);
  ui_->action_toggle_show_sidebar->setChecked(show_sidebar);
}

void MainWindow::ReloadAllSettings() {
  ReloadSettings();

  // Other settings
  app_->ReloadSettings();
  app_->global_search()->ReloadSettings();
  app_->library()->ReloadSettings();
  app_->player()->ReloadSettings();
  osd_->ReloadSettings();
  library_view_->ReloadSettings();
  song_info_view_->ReloadSettings();
  app_->player()->engine()->ReloadSettings();
  ui_->playlist->view()->ReloadSettings();
  app_->internet_model()->ReloadSettings();
#ifdef HAVE_WIIMOTEDEV
  wiimotedev_shortcuts_->ReloadSettings();
#endif
}

void MainWindow::RefreshStyleSheet() { setStyleSheet(styleSheet()); }
void MainWindow::MediaStopped() {
  setWindowTitle(QCoreApplication::applicationName());

  ui_->action_stop->setEnabled(false);
  ui_->action_stop_after_this_track->setEnabled(false);
  ui_->action_play_pause->setIcon(
      IconLoader::Load("media-playback-start", IconLoader::Base));
  ui_->action_play_pause->setText(tr("Play"));

  ui_->action_play_pause->setEnabled(true);

  ui_->action_love->setEnabled(false);
  tray_icon_->LastFMButtonLoveStateChanged(false);

  track_position_timer_->stop();
  track_slider_timer_->stop();
  ui_->track_slider->SetStopped();
  tray_icon_->SetProgress(0);
  tray_icon_->SetStopped();
}

void MainWindow::MediaPaused() {
  ui_->action_stop->setEnabled(true);
  ui_->action_stop_after_this_track->setEnabled(true);
  ui_->action_play_pause->setIcon(
      IconLoader::Load("media-playback-start", IconLoader::Base));
  ui_->action_play_pause->setText(tr("Play"));

  ui_->action_play_pause->setEnabled(true);

  track_position_timer_->stop();
  track_slider_timer_->stop();

  tray_icon_->SetPaused();
}

void MainWindow::MediaPlaying() {
  ui_->action_stop->setEnabled(true);
  ui_->action_stop_after_this_track->setEnabled(true);
  ui_->action_play_pause->setIcon(
      IconLoader::Load("media-playback-pause", IconLoader::Base));
  ui_->action_play_pause->setText(tr("Pause"));

  bool enable_play_pause = !(app_->player()->GetCurrentItem()->options() &
                             PlaylistItem::PauseDisabled);
  ui_->action_play_pause->setEnabled(enable_play_pause);

  bool can_seek = !(app_->player()->GetCurrentItem()->options() &
                    PlaylistItem::SeekDisabled);
  ui_->track_slider->SetCanSeek(can_seek);

#ifdef HAVE_LIBLASTFM
  bool enable_love = app_->scrobbler()->IsScrobblingEnabled();
  ui_->action_love->setEnabled(enable_love);
  tray_icon_->LastFMButtonLoveStateChanged(enable_love);
  tray_icon_->SetPlaying(enable_play_pause, enable_love);
#else
  tray_icon_->SetPlaying(enable_play_pause);
#endif

  track_position_timer_->start();
  track_slider_timer_->start();
  UpdateTrackPosition();
}

void MainWindow::VolumeChanged(int volume) {
  ui_->action_mute->setChecked(!volume);
  tray_icon_->MuteButtonStateChanged(!volume);
}

void MainWindow::SongChanged(const Song& song) {
  setWindowTitle(song.PrettyTitleWithArtist());
  tray_icon_->SetProgress(0);

#ifdef HAVE_LIBLASTFM
  if (ui_->action_toggle_scrobbling->isVisible())
    SetToggleScrobblingIcon(app_->scrobbler()->IsScrobblingEnabled());
#endif
}

void MainWindow::TrackSkipped(PlaylistItemPtr item) {
  // If it was a library item then we have to increment its skipped count in
  // the database.
  if (item && item->IsLocalLibraryItem() && item->Metadata().id() != -1 &&
      app_->playlist_manager()->active()->get_lastfm_status() !=
          Playlist::LastFM_Scrobbled &&
      app_->playlist_manager()->active()->get_lastfm_status() !=
          Playlist::LastFM_Queued) {
    Song song = item->Metadata();
    const qint64 position = app_->player()->engine()->position_nanosec();
    const qint64 length = app_->player()->engine()->length_nanosec();
    const float percentage = (length == 0 ? 1 : float(position) / length);

    const qint64 seconds_left = (length - position) / kNsecPerSec;
    const qint64 seconds_total = length / kNsecPerSec;

    if (((0.05 * seconds_total > 60 && percentage < 0.98) ||
         percentage < 0.95) &&
        seconds_left > 5) {  // Never count the skip if under 5 seconds left
      app_->library_backend()->IncrementSkipCountAsync(song.id(), percentage);
    }
  }
}

#ifdef HAVE_LIBLASTFM
void MainWindow::ScrobblingEnabledChanged(bool value) {
  if (ui_->action_toggle_scrobbling->isVisible())
    SetToggleScrobblingIcon(value);

  if (app_->player()->GetState() != Engine::Idle) {
    return;
  } else {
    // invalidate current song, we will scrobble the next one
    if (app_->playlist_manager()->active()->get_lastfm_status() ==
        Playlist::LastFM_New) {
      app_->playlist_manager()->active()->set_lastfm_status(
          Playlist::LastFM_Seeked);
    }
  }

  ui_->action_love->setEnabled(value);
  tray_icon_->LastFMButtonLoveStateChanged(value);
}
#endif

void MainWindow::LastFMButtonVisibilityChanged(bool value) {
  ui_->action_love->setVisible(value);
  ui_->last_fm_controls->setVisible(value);
  tray_icon_->LastFMButtonVisibilityChanged(value);
}

void MainWindow::ScrobbleButtonVisibilityChanged(bool value) {
  ui_->action_toggle_scrobbling->setVisible(value);
  ui_->scrobbling_button->setVisible(value);

  // when you reshow the buttons
  if (value) {
    // check if the song was scrobbled
    if (app_->playlist_manager()->active()->get_lastfm_status() ==
        Playlist::LastFM_Scrobbled) {
      ui_->action_toggle_scrobbling->setIcon(
          IconLoader::Load("as", IconLoader::Lastfm));
    } else {
#ifdef HAVE_LIBLASTFM
      SetToggleScrobblingIcon(app_->scrobbler()->IsScrobblingEnabled());
#endif
    }
  }
}

void MainWindow::resizeEvent(QResizeEvent*) { SaveGeometry(); }

void MainWindow::SaveGeometry() {
  was_maximized_ = isMaximized();
  settings_.setValue("maximized", was_maximized_);
  // Save the geometry only when mainwindow is not in maximized state
  if (!was_maximized_) {
    settings_.setValue("geometry", saveGeometry());
  }
  settings_.setValue("splitter_state", ui_->splitter->saveState());
  settings_.setValue("current_tab", ui_->tabs->current_index());
  settings_.setValue("tab_mode", ui_->tabs->mode());
}

void MainWindow::SavePlaybackStatus() {
  QSettings settings;
  settings.beginGroup(MainWindow::kSettingsGroup);
  settings.setValue("playback_state", app_->player()->GetState());
  if (app_->player()->GetState() == Engine::Playing ||
      app_->player()->GetState() == Engine::Paused) {
    settings.setValue(
        "playback_position",
        app_->player()->engine()->position_nanosec() / kNsecPerSec);
  } else {
    settings.setValue("playback_position", 0);
  }
}

void MainWindow::LoadPlaybackStatus() {
  QSettings settings;
  settings.beginGroup(MainWindow::kSettingsGroup);
  bool resume_playback =
      settings.value("resume_playback_after_start", false).toBool();
  saved_playback_state_ = static_cast<Engine::State>(
      settings.value("playback_state", Engine::Empty).toInt());
  saved_playback_position_ = settings.value("playback_position", 0).toDouble();
  if (!resume_playback || saved_playback_state_ == Engine::Empty ||
      saved_playback_state_ == Engine::Idle) {
    return;
  }

  connect(app_->playlist_manager()->active(), SIGNAL(RestoreFinished()),
          SLOT(ResumePlayback()));
}

void MainWindow::ResumePlayback() {
  qLog(Debug) << "Resuming playback";

  disconnect(app_->playlist_manager()->active(), SIGNAL(RestoreFinished()),
             this, SLOT(ResumePlayback()));

  if (saved_playback_state_ == Engine::Paused) {
    NewClosure(app_->player(), SIGNAL(Playing()), app_->player(),
               SLOT(PlayPause()));
  }

  app_->player()->Play();

  app_->player()->SeekTo(saved_playback_position_);
}

void MainWindow::PlayIndex(const QModelIndex& index) {
  if (!index.isValid()) return;

  int row = index.row();
  if (index.model() == app_->playlist_manager()->current()->proxy()) {
    // The index was in the proxy model (might've been filtered), so we need
    // to get the actual row in the source model.
    row =
        app_->playlist_manager()->current()->proxy()->mapToSource(index).row();
  }

  app_->playlist_manager()->SetActiveToCurrent();
  app_->player()->PlayAt(row, Engine::Manual, true);
}

void MainWindow::PlaylistDoubleClick(const QModelIndex& index) {
  if (!index.isValid()) return;

  int row = index.row();
  if (index.model() == app_->playlist_manager()->current()->proxy()) {
    // The index was in the proxy model (might've been filtered), so we need
    // to get the actual row in the source model.
    row =
        app_->playlist_manager()->current()->proxy()->mapToSource(index).row();
  }

  QModelIndexList dummyIndexList;

  switch (doubleclick_playlist_addmode_) {
    case PlaylistAddBehaviour_Play:
      app_->playlist_manager()->SetActiveToCurrent();
      app_->player()->PlayAt(row, Engine::Manual, true);
      break;

    case PlaylistAddBehaviour_Enqueue:
      dummyIndexList.append(index);
      app_->playlist_manager()->current()->queue()->ToggleTracks(
          dummyIndexList);
      if (app_->player()->GetState() != Engine::Playing) {
        app_->player()->PlayAt(
            app_->playlist_manager()->current()->queue()->TakeNext(),
            Engine::Manual, true);
      }
      break;
  }
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
  } else if (!isVisible()) {
    show();
    activateWindow();
  } else {
    // Window is not hidden but does not have focus; bring it to front.
    activateWindow();
    raise();
  }
}

void MainWindow::StopAfterCurrent() {
  app_->playlist_manager()->active()->StopAfter(
      app_->playlist_manager()->active()->current_row());
  emit StopAfterToggled(
      app_->playlist_manager()->active()->stop_after_current());
}

void MainWindow::closeEvent(QCloseEvent* event) {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  bool keep_running = s.value("keeprunning", tray_icon_->IsVisible()).toBool();

  if (keep_running && event->spontaneous()) {
    event->ignore();
    SetHiddenInTray(true);
  } else {
    QApplication::quit();
  }
}

void MainWindow::SetHiddenInTray(bool hidden) {
  settings_.setValue("hidden", hidden);

  // Some window managers don't remember maximized state between calls to
  // hide() and show(), so we have to remember it ourself.
  if (hidden) {
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

void MainWindow::Seeked(qlonglong microseconds) {
  const int position = microseconds / kUsecPerSec;
  const int length =
      app_->player()->GetCurrentItem()->Metadata().length_nanosec() /
      kNsecPerSec;
  tray_icon_->SetProgress(double(position) / length * 100);

  // if we seeked, scrobbling is canceled, update the icon
  if (ui_->action_toggle_scrobbling->isVisible()) SetToggleScrobblingIcon(true);
}

void MainWindow::UpdateTrackPosition() {
  // Track position in seconds
  Playlist* playlist = app_->playlist_manager()->active();

  PlaylistItemPtr item(app_->player()->GetCurrentItem());
  const int position = std::floor(
      float(app_->player()->engine()->position_nanosec()) / kNsecPerSec + 0.5);
  const int length = app_->player()->engine()->length_nanosec() / kNsecPerSec;
  const int scrobble_point = playlist->scrobble_point_nanosec() / kNsecPerSec;

  if (length <= 0) {
    // Probably a stream that we don't know the length of
    return;
  }
#ifdef HAVE_LIBLASTFM
  const bool last_fm_enabled = ui_->action_toggle_scrobbling->isVisible() &&
                               app_->scrobbler()->IsScrobblingEnabled() &&
                               app_->scrobbler()->IsAuthenticated();
#endif

  // Time to scrobble?
  if (position >= scrobble_point) {
    if (playlist->get_lastfm_status() == Playlist::LastFM_New) {
#ifdef HAVE_LIBLASTFM
      if (app_->scrobbler()->IsScrobblingEnabled() &&
          app_->scrobbler()->IsAuthenticated()) {
        qLog(Info) << "Scrobbling at" << scrobble_point;
        app_->scrobbler()->Scrobble();
      }
#endif
    }

    // Update the play count for the song if it's from the library
    if (!playlist->have_incremented_playcount() && item->IsLocalLibraryItem() &&
        item->Metadata().id() != -1 &&
        playlist->get_lastfm_status() != Playlist::LastFM_Seeked) {
      app_->library_backend()->IncrementPlayCountAsync(item->Metadata().id());
      playlist->set_have_incremented_playcount();
    }
  }

  // (just after) the scrobble point is a good point to change tracks in intro
  // mode
  if (position >= scrobble_point + 5) {
    if (playlist->sequence()->repeat_mode() == PlaylistSequence::Repeat_Intro) {
      emit IntroPointReached();
    }
  }

  // Update the tray icon every 10 seconds
  if (position % 10 == 0) {
    qLog(Debug) << "position" << position << "scrobble point" << scrobble_point
                << "status" << playlist->get_lastfm_status();
    tray_icon_->SetProgress(double(position) / length * 100);

// if we're waiting for the scrobble point, update the icon
#ifdef HAVE_LIBLASTFM
    if (position < scrobble_point &&
        playlist->get_lastfm_status() == Playlist::LastFM_New &&
        last_fm_enabled) {
      ui_->action_toggle_scrobbling->setIcon(
          CreateOverlayedIcon(position, scrobble_point));
    }
#endif
  }
}

void MainWindow::UpdateTrackSliderPosition() {
  PlaylistItemPtr item(app_->player()->GetCurrentItem());

  const int slider_position = std::floor(
      float(app_->player()->engine()->position_nanosec()) / kNsecPerMsec);
  const int slider_length =
      app_->player()->engine()->length_nanosec() / kNsecPerMsec;

  // Update the slider
  ui_->track_slider->SetValue(slider_position, slider_length);
}

#ifdef HAVE_LIBLASTFM
void MainWindow::ScrobbledRadioStream() {
  ui_->action_love->setEnabled(true);
  tray_icon_->LastFMButtonLoveStateChanged(true);
}

void MainWindow::Love() {
  app_->scrobbler()->Love();
  ui_->action_love->setEnabled(false);
  tray_icon_->LastFMButtonLoveStateChanged(false);
}
#endif

void MainWindow::ApplyAddBehaviour(MainWindow::AddBehaviour b,
                                   MimeData* data) const {
  switch (b) {
    case AddBehaviour_Append:
      data->clear_first_ = false;
      data->enqueue_now_ = false;
      break;

    case AddBehaviour_Enqueue:
      data->clear_first_ = false;
      data->enqueue_now_ = true;
      break;

    case AddBehaviour_Load:
      data->clear_first_ = true;
      data->enqueue_now_ = false;
      break;

    case AddBehaviour_OpenInNew:
      data->open_in_new_playlist_ = true;
      break;
  }
}

void MainWindow::ApplyPlayBehaviour(MainWindow::PlayBehaviour b,
                                    MimeData* data) const {
  switch (b) {
    case PlayBehaviour_Always:
      data->play_now_ = true;
      break;

    case PlayBehaviour_Never:
      data->play_now_ = false;
      break;

    case PlayBehaviour_IfStopped:
      data->play_now_ = !(app_->player()->GetState() == Engine::Playing);
      break;
  }
}

void MainWindow::AddToPlaylist(QMimeData* data) {
  if (!data) return;

  if (MimeData* mime_data = qobject_cast<MimeData*>(data)) {
    // Should we replace the flags with the user's preference?
    if (mime_data->override_user_settings_) {
      // Do nothing
    } else if (mime_data->from_doubleclick_) {
      ApplyAddBehaviour(doubleclick_addmode_, mime_data);
      ApplyPlayBehaviour(doubleclick_playmode_, mime_data);
    } else {
      ApplyPlayBehaviour(menu_playmode_, mime_data);
    }

    // Should we create a new playlist for the songs?
    if (mime_data->open_in_new_playlist_) {
      app_->playlist_manager()->New(mime_data->get_name_for_new_playlist());
    }
  }

  app_->playlist_manager()->current()->dropMimeData(data, Qt::CopyAction, -1, 0,
                                                    QModelIndex());
  delete data;
}

void MainWindow::AddToPlaylist(QAction* action) {
  int destination = action->data().toInt();
  PlaylistItemList items;

  // get the selected playlist items
  for (const QModelIndex& index :
       ui_->playlist->view()->selectionModel()->selection().indexes()) {
    if (index.column() != 0) continue;
    int row =
        app_->playlist_manager()->current()->proxy()->mapToSource(index).row();
    items << app_->playlist_manager()->current()->item_at(row);
  }

  SongList songs;
  for (PlaylistItemPtr item : items) {
    songs << item->Metadata();
  }

  // we're creating a new playlist
  if (destination == -1) {
    // save the current playlist to reactivate it
    int current_id = app_->playlist_manager()->current_id();
    // get the name from selection
    app_->playlist_manager()->New(
        app_->playlist_manager()->GetNameForNewPlaylist(songs));
    if (app_->playlist_manager()->current()->id() != current_id) {
      // I'm sure the new playlist was created and is selected, so I can just
      // insert items
      app_->playlist_manager()->current()->InsertItems(items);
      // set back the current playlist
      app_->playlist_manager()->SetCurrentPlaylist(current_id);
    }
  } else {
    // we're inserting in a existing playlist
    app_->playlist_manager()->playlist(destination)->InsertItems(items);
  }
}

void MainWindow::PlaylistRightClick(const QPoint& global_pos,
                                    const QModelIndex& index) {
  QModelIndex source_index =
      app_->playlist_manager()->current()->proxy()->mapToSource(index);
  playlist_menu_index_ = source_index;

  // Is this song currently playing?
  if (app_->playlist_manager()->current()->current_row() ==
          source_index.row() &&
      app_->player()->GetState() == Engine::Playing) {
    playlist_play_pause_->setText(tr("Pause"));
    playlist_play_pause_->setIcon(
        IconLoader::Load("media-playback-pause", IconLoader::Base));
  } else {
    playlist_play_pause_->setText(tr("Play"));
    playlist_play_pause_->setIcon(
        IconLoader::Load("media-playback-start", IconLoader::Base));
  }

  // Are we allowed to pause?
  if (index.isValid()) {
    playlist_play_pause_->setEnabled(
        app_->playlist_manager()->current()->current_row() !=
            source_index.row() ||
        !(app_->playlist_manager()
              ->current()
              ->item_at(source_index.row())
              ->options() &
          PlaylistItem::PauseDisabled));
  } else {
    playlist_play_pause_->setEnabled(false);
  }

  playlist_stop_after_->setEnabled(index.isValid());

  // Are any of the selected songs editable or queued?
  QModelIndexList selection =
      ui_->playlist->view()->selectionModel()->selection().indexes();
  bool cue_selected = false;
  int editable = 0;
  int streams = 0;
  int in_queue = 0;
  int not_in_queue = 0;
  int in_skipped = 0;
  int not_in_skipped = 0;
  for (const QModelIndex& index : selection) {
    if (index.column() != 0) continue;

    PlaylistItemPtr item =
        app_->playlist_manager()->current()->item_at(index.row());
    if (item->Metadata().has_cue()) {
      cue_selected = true;
    } else if (item->Metadata().IsEditable()) {
      editable++;
    }

    if (item->Metadata().is_stream()) {
      streams++;
    }

    if (index.data(Playlist::Role_QueuePosition).toInt() == -1)
      not_in_queue++;
    else
      in_queue++;

    if (item->GetShouldSkip()) {
      in_skipped++;
    } else {
      not_in_skipped++;
    }
  }

  int all = not_in_queue + in_queue;

  // this is available when we have one or many files and at least one of
  // those is not CUE related
  ui_->action_edit_track->setEnabled(editable);
  ui_->action_edit_track->setVisible(editable);
  ui_->action_auto_complete_tags->setEnabled(editable);
  ui_->action_auto_complete_tags->setVisible(editable);
  // the rest of the read / write actions work only when there are no CUEs
  // involved
  if (cue_selected) editable = 0;

  // no 'show in browser' action if only streams are selected
  playlist_open_in_browser_->setVisible(streams != all);

  // If exactly one stream is selected, enable the 'show details' action.
  ui_->action_view_stream_details->setEnabled(all == 1 && streams == 1);
  ui_->action_view_stream_details->setVisible(all == 1 && streams == 1);

  bool track_column = (index.column() == Playlist::Column_Track);
  ui_->action_renumber_tracks->setVisible(editable >= 2 && track_column);
  ui_->action_selection_set_value->setVisible(editable >= 2 && !track_column);
  ui_->action_edit_value->setVisible(editable);
  ui_->action_remove_from_playlist->setEnabled(!selection.isEmpty());

  playlist_show_in_library_->setVisible(false);
  playlist_copy_to_library_->setVisible(false);
  playlist_move_to_library_->setVisible(false);
  playlist_organise_->setVisible(false);
  playlist_delete_->setVisible(false);
  playlist_copy_to_device_->setVisible(false);

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

  if (in_skipped == 1 && not_in_skipped == 0)
    playlist_skip_->setText(tr("Unskip track"));
  else if (in_skipped > 1 && not_in_skipped == 0)
    playlist_skip_->setText(tr("Unskip selected tracks"));
  else if (in_skipped == 0 && not_in_skipped == 1)
    playlist_skip_->setText(tr("Skip track"));
  else if (in_skipped == 0 && not_in_skipped > 1)
    playlist_skip_->setText(tr("Skip selected tracks"));

  if (not_in_queue == 0)
    playlist_queue_->setIcon(IconLoader::Load("go-previous", IconLoader::Base));
  else
    playlist_queue_->setIcon(IconLoader::Load("go-next", IconLoader::Base));

  if (!index.isValid()) {
    ui_->action_selection_set_value->setVisible(false);
    ui_->action_edit_value->setVisible(false);
  } else {
    Playlist::Column column = (Playlist::Column)index.column();
    bool column_is_editable = Playlist::column_is_editable(column) && editable;

    ui_->action_selection_set_value->setVisible(
        ui_->action_selection_set_value->isVisible() && column_is_editable);
    ui_->action_edit_value->setVisible(ui_->action_edit_value->isVisible() &&
                                       column_is_editable);

    QString column_name = Playlist::column_name(column);
    QString column_value =
        app_->playlist_manager()->current()->data(source_index).toString();
    if (column_value.length() > 25)
      column_value = column_value.left(25) + "...";

    ui_->action_selection_set_value->setText(
        tr("Set %1 to \"%2\"...").arg(column_name.toLower()).arg(column_value));
    ui_->action_edit_value->setText(tr("Edit tag \"%1\"...").arg(column_name));

    // Is it a library item?
    PlaylistItemPtr item =
        app_->playlist_manager()->current()->item_at(source_index.row());
    if (item->IsLocalLibraryItem() && item->Metadata().id() != -1) {
      playlist_organise_->setVisible(editable);
      playlist_show_in_library_->setVisible(editable);
    } else {
      playlist_copy_to_library_->setVisible(editable);
      playlist_move_to_library_->setVisible(editable);
    }

    playlist_delete_->setVisible(editable);
    playlist_copy_to_device_->setVisible(editable);

    // Remove old item actions, if any.
    for (QAction* action : playlistitem_actions_) {
      playlist_menu_->removeAction(action);
    }

    // Get the new item actions, and add them
    playlistitem_actions_ = item->actions();
    playlistitem_actions_separator_->setVisible(
        !playlistitem_actions_.isEmpty());
    playlist_menu_->insertActions(playlistitem_actions_separator_,
                                  playlistitem_actions_);
  }

  // if it isn't the first time we right click, we need to remove the menu
  // previously created
  if (playlist_add_to_another_ != nullptr) {
    playlist_menu_->removeAction(playlist_add_to_another_);
    delete playlist_add_to_another_;
  }

  // create the playlist submenu
  QMenu* add_to_another_menu = new QMenu(tr("Add to another playlist"), this);
  add_to_another_menu->setIcon(IconLoader::Load("list-add", IconLoader::Base));

  for (const PlaylistBackend::Playlist& playlist :
       app_->playlist_backend()->GetAllOpenPlaylists()) {
    // don't add the current playlist
    if (playlist.id != app_->playlist_manager()->current()->id()) {
      QAction* existing_playlist = new QAction(this);
      existing_playlist->setText(playlist.name);
      existing_playlist->setData(playlist.id);
      add_to_another_menu->addAction(existing_playlist);
    }
  }

  add_to_another_menu->addSeparator();
  // add to a new playlist
  QAction* new_playlist = new QAction(this);
  new_playlist->setText(tr("New playlist"));
  new_playlist->setData(-1);  // fake id
  add_to_another_menu->addAction(new_playlist);
  playlist_add_to_another_ = playlist_menu_->insertMenu(
      ui_->action_remove_from_playlist, add_to_another_menu);

  connect(add_to_another_menu, SIGNAL(triggered(QAction*)),
          SLOT(AddToPlaylist(QAction*)));

  playlist_menu_->popup(global_pos);
}

void MainWindow::PlaylistPlay() {
  if (app_->playlist_manager()->current()->current_row() ==
      playlist_menu_index_.row()) {
    app_->player()->PlayPause();
  } else {
    PlayIndex(playlist_menu_index_);
  }
}

void MainWindow::PlaylistStopAfter() {
  app_->playlist_manager()->current()->StopAfter(playlist_menu_index_.row());
}

void MainWindow::EditTracks() {
  SongList songs;
  PlaylistItemList items;

  for (const QModelIndex& index :
       ui_->playlist->view()->selectionModel()->selection().indexes()) {
    if (index.column() != 0) continue;
    int row =
        app_->playlist_manager()->current()->proxy()->mapToSource(index).row();
    PlaylistItemPtr item(app_->playlist_manager()->current()->item_at(row));
    Song song = item->Metadata();

    if (song.IsEditable()) {
      songs << song;
      items << item;
    }
  }

  edit_tag_dialog_->SetSongs(songs, items);
  edit_tag_dialog_->show();
}

void MainWindow::EditTagDialogAccepted() {
  for (PlaylistItemPtr item : edit_tag_dialog_->playlist_items()) {
    item->Reload();
  }

  // This is really lame but we don't know what rows have changed
  ui_->playlist->view()->update();

  app_->playlist_manager()->current()->Save();
}

void MainWindow::DiscoverStreamDetails() {
  int row = playlist_menu_index_.row();
  Song song = app_->playlist_manager()->current()->item_at(row)->Metadata();

  QString url = song.url().toString();
  stream_discoverer_->Discover(url);
}

void MainWindow::ShowStreamDetails(const StreamDetails& details) {
  StreamDetailsDialog stream_details_dialog(this);

  stream_details_dialog.setUrl(details.url);
  stream_details_dialog.setFormat(details.format);
  stream_details_dialog.setBitrate(details.bitrate);
  stream_details_dialog.setChannels(details.channels);
  stream_details_dialog.setDepth(details.depth);
  stream_details_dialog.setSampleRate(details.sample_rate);

  stream_details_dialog.exec();
}

void MainWindow::RenumberTracks() {
  QModelIndexList indexes =
      ui_->playlist->view()->selectionModel()->selection().indexes();
  int track = 1;

  // Get the index list in order
  qStableSort(indexes);

  // if first selected song has a track number set, start from that offset
  if (!indexes.isEmpty()) {
    const Song first_song = app_->playlist_manager()
                                ->current()
                                ->item_at(indexes[0].row())
                                ->Metadata();

    if (first_song.track() > 0) track = first_song.track();
  }

  for (const QModelIndex& index : indexes) {
    if (index.column() != 0) continue;

    const QModelIndex source_index =
        app_->playlist_manager()->current()->proxy()->mapToSource(index);
    int row = source_index.row();
    Song song = app_->playlist_manager()->current()->item_at(row)->Metadata();

    if (song.IsEditable()) {
      song.set_track(track);

      TagReaderReply* reply =
          TagReaderClient::Instance()->SaveFile(song.url().toLocalFile(), song);

      NewClosure(reply, SIGNAL(Finished(bool)), this,
                 SLOT(SongSaveComplete(TagReaderReply*, QPersistentModelIndex)),
                 reply, QPersistentModelIndex(source_index));
    }
    track++;
  }
}

void MainWindow::SongSaveComplete(TagReaderReply* reply,
                                  const QPersistentModelIndex& index) {
  if (reply->is_successful() && index.isValid()) {
    app_->playlist_manager()->current()->ReloadItems(QList<int>()
                                                     << index.row());
  }
  reply->deleteLater();
}

void MainWindow::SelectionSetValue() {
  Playlist::Column column = (Playlist::Column)playlist_menu_index_.column();
  QVariant column_value =
      app_->playlist_manager()->current()->data(playlist_menu_index_);

  QModelIndexList indexes =
      ui_->playlist->view()->selectionModel()->selection().indexes();
  for (const QModelIndex& index : indexes) {
    if (index.column() != 0) continue;

    const QModelIndex source_index =
        app_->playlist_manager()->current()->proxy()->mapToSource(index);
    int row = source_index.row();
    Song song = app_->playlist_manager()->current()->item_at(row)->Metadata();

    if (Playlist::set_column_value(song, column, column_value)) {
      TagReaderReply* reply =
          TagReaderClient::Instance()->SaveFile(song.url().toLocalFile(), song);

      NewClosure(reply, SIGNAL(Finished(bool)), this,
                 SLOT(SongSaveComplete(TagReaderReply*, QPersistentModelIndex)),
                 reply, QPersistentModelIndex(source_index));
    }
  }
}

void MainWindow::EditValue() {
  QModelIndex current = ui_->playlist->view()->currentIndex();
  if (!current.isValid()) return;

  // Edit the last column that was right-clicked on.  If nothing's ever been
  // right clicked then look for the first editable column.
  int column = playlist_menu_index_.column();
  if (column == -1) {
    for (int i = 0; i < ui_->playlist->view()->model()->columnCount(); ++i) {
      if (ui_->playlist->view()->isColumnHidden(i)) continue;
      if (!Playlist::column_is_editable(Playlist::Column(i))) continue;
      column = i;
      break;
    }
  }

  ui_->playlist->view()->edit(current.sibling(current.row(), column));
}

void MainWindow::AddFile() {
  // Last used directory
  QString directory =
      settings_.value("add_media_path", QDir::currentPath()).toString();

  PlaylistParser parser(app_->library_backend());

  // Show dialog
  QStringList file_names = QFileDialog::getOpenFileNames(
      this, tr("Add file"), directory,
      QString("%1 (%2);;%3;;%4")
          .arg(tr("Music"), FileView::kFileFilter, parser.filters(),
               tr(kAllFilesFilterSpec)));
  if (file_names.isEmpty()) return;

  // Save last used directory
  settings_.setValue("add_media_path", file_names[0]);

  // Convert to URLs
  QList<QUrl> urls;
  for (const QString& path : file_names) {
    urls << QUrl::fromLocalFile(QFileInfo(path).canonicalFilePath());
  }

  MimeData* data = new MimeData;
  data->setUrls(urls);
  AddToPlaylist(data);
}

void MainWindow::AddFolder() {
  // Last used directory
  QString directory =
      settings_.value("add_folder_path", QDir::currentPath()).toString();

  // Show dialog
  directory =
      QFileDialog::getExistingDirectory(this, tr("Add folder"), directory);
  if (directory.isEmpty()) return;

  // Save last used directory
  settings_.setValue("add_folder_path", directory);

  // Add media
  MimeData* data = new MimeData;
  data->setUrls(QList<QUrl>() << QUrl::fromLocalFile(
                    QFileInfo(directory).canonicalFilePath()));
  AddToPlaylist(data);
}

void MainWindow::AddStream() { add_stream_dialog_->show(); }

void MainWindow::AddStreamAccepted() {
  MimeData* data = new MimeData;
  data->setUrls(QList<QUrl>() << add_stream_dialog_->url());
  AddToPlaylist(data);
}

void MainWindow::OpenRipCDDialog() {
#ifdef HAVE_AUDIOCD
  if (!rip_cd_dialog_) {
    rip_cd_dialog_.reset(new RipCDDialog);
  }
  if (rip_cd_dialog_->CheckCDIOIsValid()) {
    rip_cd_dialog_->show();
  } else {
    QMessageBox cdio_fail(QMessageBox::Critical, tr("Error"),
                          tr("Failed reading CD drive"));
    cdio_fail.exec();
  }
#endif
}

void MainWindow::AddCDTracks() {
  MimeData* data = new MimeData;
  // We are putting empty data, but we specify cdda mimetype to indicate that
  // we want to load audio cd tracks
  data->open_in_new_playlist_ = true;
  data->setData(Playlist::kCddaMimeType, QByteArray());
  AddToPlaylist(data);
}

void MainWindow::ShowInLibrary() {
  // Show the first valid selected track artist/album in LibraryView
  QModelIndexList proxy_indexes =
      ui_->playlist->view()->selectionModel()->selectedRows();
  SongList songs;

  for (const QModelIndex& proxy_index : proxy_indexes) {
    QModelIndex index =
        app_->playlist_manager()->current()->proxy()->mapToSource(proxy_index);
    if (app_->playlist_manager()
            ->current()
            ->item_at(index.row())
            ->IsLocalLibraryItem()) {
      songs << app_->playlist_manager()
                   ->current()
                   ->item_at(index.row())
                   ->Metadata();
      break;
    }
  }
  QString search;
  if (!songs.isEmpty()) {
    search =
        "artist:" + songs.first().artist() + " album:" + songs.first().album();
  }
  library_view_->filter()->ShowInLibrary(search);
  FocusLibraryTab();
}

void MainWindow::PlaylistRemoveCurrent() {
  ui_->playlist->view()->RemoveSelected();
}

void MainWindow::PlaylistEditFinished(const QModelIndex& index) {
  if (index == playlist_menu_index_) SelectionSetValue();
}

void MainWindow::CommandlineOptionsReceived(
    const QByteArray& serialized_options) {
  if (serialized_options == "wake up!") {
    // Old versions of Clementine sent this - just ignore it
    return;
  }

  CommandlineOptions options;
  options.Load(serialized_options);

  if (options.is_empty()) {
    show();
    activateWindow();
  } else
    CommandlineOptionsReceived(options);
}

void MainWindow::CommandlineOptionsReceived(const CommandlineOptions& options) {
  switch (options.player_action()) {
    case CommandlineOptions::Player_Play:
      if (options.urls().empty()) {
        app_->player()->Play();
      }
      break;
    case CommandlineOptions::Player_PlayPause:
      app_->player()->PlayPause();
      break;
    case CommandlineOptions::Player_Pause:
      app_->player()->Pause();
      break;
    case CommandlineOptions::Player_Stop:
      app_->player()->Stop();
      break;
    case CommandlineOptions::Player_StopAfterCurrent:
      app_->player()->StopAfterCurrent();
      break;
    case CommandlineOptions::Player_Previous:
      app_->player()->Previous();
      break;
    case CommandlineOptions::Player_Next:
      app_->player()->Next();
      break;
    case CommandlineOptions::Player_RestartOrPrevious:
      app_->player()->RestartOrPrevious();
      break;

    case CommandlineOptions::Player_None:
      break;
  }

  if (!options.urls().empty()) {
    MimeData* data = new MimeData;
    data->setUrls(options.urls());
    // Behaviour depends on command line options, so set it here
    data->override_user_settings_ = true;

    if (options.player_action() == CommandlineOptions::Player_Play)
      data->play_now_ = true;
    else
      ApplyPlayBehaviour(doubleclick_playmode_, data);

    switch (options.url_list_action()) {
      case CommandlineOptions::UrlList_Load:
        data->clear_first_ = true;
        break;
      case CommandlineOptions::UrlList_Append:
        // Nothing to do
        break;
      case CommandlineOptions::UrlList_None:
        ApplyAddBehaviour(doubleclick_addmode_, data);
        break;
      case CommandlineOptions::UrlList_CreateNew:
        data->name_for_new_playlist_ = options.playlist_name();
        ApplyAddBehaviour(AddBehaviour_OpenInNew, data);
        break;
    }

    AddToPlaylist(data);
  }

  if (options.set_volume() != -1)
    app_->player()->SetVolume(options.set_volume());

  if (options.volume_modifier() != 0)
    app_->player()->SetVolume(app_->player()->GetVolume() +
                              options.volume_modifier());

  if (options.seek_to() != -1)
    app_->player()->SeekTo(options.seek_to());
  else if (options.seek_by() != 0)
    app_->player()->SeekTo(app_->player()->engine()->position_nanosec() /
                               kNsecPerSec +
                           options.seek_by());

  if (options.play_track_at() != -1)
    app_->player()->PlayAt(options.play_track_at(), Engine::Manual, true);

  if (options.show_osd()) app_->player()->ShowOSD();

  if (options.toggle_pretty_osd()) app_->player()->TogglePrettyOSD();
}

void MainWindow::ForceShowOSD(const Song& song, const bool toggle) {
  if (toggle) {
    osd_->SetPrettyOSDToggleMode(toggle);
  }
  osd_->ReshowCurrentSong();
}

void MainWindow::Activate() { show(); }

bool MainWindow::LoadUrl(const QString& url) {
  if (!QFile::exists(url)) return false;

  MimeData* data = new MimeData;
  data->setUrls(QList<QUrl>() << QUrl::fromLocalFile(url));
  AddToPlaylist(data);

  return true;
}

void MainWindow::CheckForUpdates() {
#if defined(Q_OS_DARWIN)
  mac::CheckForUpdates();
#endif
}

void MainWindow::PlaylistUndoRedoChanged(QAction* undo, QAction* redo) {
  playlist_menu_->insertAction(playlist_undoredo_, undo);
  playlist_menu_->insertAction(playlist_undoredo_, redo);
}

void MainWindow::AddFilesToTranscoder() {
  QStringList filenames;

  for (const QModelIndex& index :
       ui_->playlist->view()->selectionModel()->selection().indexes()) {
    if (index.column() != 0) continue;
    int row =
        app_->playlist_manager()->current()->proxy()->mapToSource(index).row();
    PlaylistItemPtr item(app_->playlist_manager()->current()->item_at(row));
    Song song = item->Metadata();
    filenames << song.url().toLocalFile();
  }

  transcode_dialog_->SetFilenames(filenames);

  ShowTranscodeDialog();
}

void MainWindow::ShowLibraryConfig() {
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

void MainWindow::CopyFilesToLibrary(const QList<QUrl>& urls) {
  organise_dialog_->SetDestinationModel(
      app_->library_model()->directory_model());
  organise_dialog_->SetUrls(urls);
  organise_dialog_->SetCopy(true);
  organise_dialog_->show();
}

void MainWindow::MoveFilesToLibrary(const QList<QUrl>& urls) {
  organise_dialog_->SetDestinationModel(
      app_->library_model()->directory_model());
  organise_dialog_->SetUrls(urls);
  organise_dialog_->SetCopy(false);
  organise_dialog_->show();
}

void MainWindow::CopyFilesToDevice(const QList<QUrl>& urls) {
  organise_dialog_->SetDestinationModel(
      app_->device_manager()->connected_devices_model(), true);
  organise_dialog_->SetCopy(true);
  if (organise_dialog_->SetUrls(urls))
    organise_dialog_->show();
  else {
    QMessageBox::warning(
        this, tr("Error"),
        tr("None of the selected songs were suitable for copying to a device"));
  }
}

void MainWindow::EditFileTags(const QList<QUrl>& urls) {
  SongList songs;
  for (const QUrl& url : urls) {
    Song song;
    song.set_url(url);
    song.set_valid(true);
    song.set_filetype(Song::Type_Mpeg);
    songs << song;
  }

  edit_tag_dialog_->SetSongs(songs);
  edit_tag_dialog_->show();
}

void MainWindow::PlaylistCopyToLibrary() { PlaylistOrganiseSelected(true); }

void MainWindow::PlaylistMoveToLibrary() { PlaylistOrganiseSelected(false); }

void MainWindow::PlaylistOrganiseSelected(bool copy) {
  QModelIndexList proxy_indexes =
      ui_->playlist->view()->selectionModel()->selectedRows();
  SongList songs;

  for (const QModelIndex& proxy_index : proxy_indexes) {
    QModelIndex index =
        app_->playlist_manager()->current()->proxy()->mapToSource(proxy_index);

    songs << app_->playlist_manager()
                 ->current()
                 ->item_at(index.row())
                 ->Metadata();
  }

  organise_dialog_->SetDestinationModel(
      app_->library_model()->directory_model());
  organise_dialog_->SetSongs(songs);
  organise_dialog_->SetCopy(copy);
  organise_dialog_->show();
}

void MainWindow::PlaylistDelete() {
  // Note: copied from LibraryView::Delete

  if (QMessageBox::warning(this, tr("Delete files"),
                           tr("These files will be permanently deleted from "
                              "disk, are you sure you want to continue?"),
                           QMessageBox::Yes,
                           QMessageBox::Cancel) != QMessageBox::Yes)
    return;

  std::shared_ptr<MusicStorage> storage(new FilesystemMusicStorage("/"));

  // Get selected songs
  SongList selected_songs;
  QModelIndexList proxy_indexes =
      ui_->playlist->view()->selectionModel()->selectedRows();
  for (const QModelIndex& proxy_index : proxy_indexes) {
    QModelIndex index =
        app_->playlist_manager()->current()->proxy()->mapToSource(proxy_index);
    selected_songs << app_->playlist_manager()
                          ->current()
                          ->item_at(index.row())
                          ->Metadata();
  }

  ui_->playlist->view()->RemoveSelected();

  DeleteFiles* delete_files = new DeleteFiles(app_->task_manager(), storage);
  connect(delete_files, SIGNAL(Finished(SongList)),
          SLOT(DeleteFinished(SongList)));
  delete_files->Start(selected_songs);
}

void MainWindow::PlaylistOpenInBrowser() {
  QList<QUrl> urls;
  QModelIndexList proxy_indexes =
      ui_->playlist->view()->selectionModel()->selectedRows();

  for (const QModelIndex& proxy_index : proxy_indexes) {
    const QModelIndex index =
        app_->playlist_manager()->current()->proxy()->mapToSource(proxy_index);
    urls << QUrl(index.sibling(index.row(), Playlist::Column_Filename)
                     .data()
                     .toString());
  }

  Utilities::OpenInFileBrowser(urls);
}

void MainWindow::DeleteFinished(const SongList& songs_with_errors) {
  if (songs_with_errors.isEmpty()) return;

  OrganiseErrorDialog* dialog = new OrganiseErrorDialog(this);
  dialog->Show(OrganiseErrorDialog::Type_Delete, songs_with_errors);
  // It deletes itself when the user closes it
}

void MainWindow::PlaylistQueue() {
  QModelIndexList indexes;
  for (const QModelIndex& proxy_index :
       ui_->playlist->view()->selectionModel()->selectedRows()) {
    indexes << app_->playlist_manager()->current()->proxy()->mapToSource(
        proxy_index);
  }

  app_->playlist_manager()->current()->queue()->ToggleTracks(indexes);
}

void MainWindow::PlaylistSkip() {
  QModelIndexList indexes;
  for (const QModelIndex& proxy_index :
       ui_->playlist->view()->selectionModel()->selectedRows()) {
    indexes << app_->playlist_manager()->current()->proxy()->mapToSource(
        proxy_index);
  }

  app_->playlist_manager()->current()->SkipTracks(indexes);
}

void MainWindow::PlaylistCopyToDevice() {
  QModelIndexList proxy_indexes =
      ui_->playlist->view()->selectionModel()->selectedRows();
  SongList songs;

  for (const QModelIndex& proxy_index : proxy_indexes) {
    QModelIndex index =
        app_->playlist_manager()->current()->proxy()->mapToSource(proxy_index);

    songs << app_->playlist_manager()
                 ->current()
                 ->item_at(index.row())
                 ->Metadata();
  }

  organise_dialog_->SetDestinationModel(
      app_->device_manager()->connected_devices_model(), true);
  organise_dialog_->SetCopy(true);
  if (organise_dialog_->SetSongs(songs))
    organise_dialog_->show();
  else {
    QMessageBox::warning(
        this, tr("Error"),
        tr("None of the selected songs were suitable for copying to a device"));
  }
}

void MainWindow::ChangeLibraryQueryMode(QAction* action) {
  if (action == library_show_duplicates_) {
    library_view_->filter()->SetQueryMode(QueryOptions::QueryMode_Duplicates);
  } else if (action == library_show_untagged_) {
    library_view_->filter()->SetQueryMode(QueryOptions::QueryMode_Untagged);
  } else {
    library_view_->filter()->SetQueryMode(QueryOptions::QueryMode_All);
  }
}

void MainWindow::ShowCoverManager() { cover_manager_->show(); }

SettingsDialog* MainWindow::CreateSettingsDialog() {
  SettingsDialog* settings_dialog =
      new SettingsDialog(app_, background_streams_);
  settings_dialog->SetGlobalShortcutManager(global_shortcuts_);
  settings_dialog->SetSongInfoView(song_info_view_);

  // Settings
  connect(settings_dialog, SIGNAL(accepted()), SLOT(ReloadAllSettings()));

#ifdef HAVE_WIIMOTEDEV
  connect(settings_dialog, SIGNAL(SetWiimotedevInterfaceActived(bool)),
          wiimotedev_shortcuts_.get(),
          SLOT(SetWiimotedevInterfaceActived(bool)));
#endif

  // Allows custom notification preview
  connect(settings_dialog,
          SIGNAL(NotificationPreview(OSD::Behaviour, QString, QString)),
          SLOT(HandleNotificationPreview(OSD::Behaviour, QString, QString)));
  return settings_dialog;
}

void MainWindow::OpenSettingsDialog() { settings_dialog_->show(); }

void MainWindow::OpenSettingsDialogAtPage(SettingsDialog::Page page) {
  settings_dialog_->OpenAtPage(page);
}

EditTagDialog* MainWindow::CreateEditTagDialog() {
  EditTagDialog* edit_tag_dialog = new EditTagDialog(app_);
  connect(edit_tag_dialog, SIGNAL(accepted()), SLOT(EditTagDialogAccepted()));
  connect(edit_tag_dialog, SIGNAL(Error(QString)),
          SLOT(ShowErrorDialog(QString)));
  return edit_tag_dialog;
}

StreamDiscoverer* MainWindow::CreateStreamDiscoverer() {
  StreamDiscoverer* discoverer = new StreamDiscoverer();
  connect(discoverer, SIGNAL(DataReady(StreamDetails)),
          SLOT(ShowStreamDetails(StreamDetails)));
  connect(discoverer, SIGNAL(Error(QString)), SLOT(ShowErrorDialog(QString)));
  return discoverer;
}

void MainWindow::ShowAboutDialog() { about_dialog_->show(); }

void MainWindow::ShowTranscodeDialog() { transcode_dialog_->show(); }

void MainWindow::ShowErrorDialog(const QString& message) {
  error_dialog_->ShowMessage(message);
}

void MainWindow::CheckFullRescanRevisions() {
  int from = app_->database()->startup_schema_version();
  int to = app_->database()->current_schema_version();

  // if we're restoring DB from scratch or nothing has
  // changed, do nothing
  if (from == 0 || from == to) {
    return;
  }

  // collect all reasons
  QSet<QString> reasons;
  for (int i = from; i <= to; i++) {
    QString reason = app_->library()->full_rescan_reason(i);

    if (!reason.isEmpty()) {
      reasons.insert(reason);
    }
  }

  // if we have any...
  if (!reasons.isEmpty()) {
    QString message = tr("The version of Clementine you've just updated to "
                         "requires a full library rescan "
                         "because of the new features listed below:") +
                      "<ul>";
    for (const QString& reason : reasons) {
      message += ("<li>" + reason + "</li>");
    }
    message += "</ul>" + tr("Would you like to run a full rescan right now?");

    if (QMessageBox::question(this, tr("Library rescan notice"), message,
                              QMessageBox::Yes,
                              QMessageBox::No) == QMessageBox::Yes) {
      app_->library()->FullScan();
    }
  }
}

void MainWindow::ShowQueueManager() { queue_manager_->show(); }

void MainWindow::ShowVisualisations() {
#ifdef ENABLE_VISUALISATIONS
  if (!visualisation_) {
    visualisation_.reset(new VisualisationContainer);

    visualisation_->SetActions(ui_->action_previous_track,
                               ui_->action_play_pause, ui_->action_stop,
                               ui_->action_next_track);
    connect(app_->player(), SIGNAL(Stopped()), visualisation_.get(),
            SLOT(Stopped()));
    connect(app_->player(), SIGNAL(ForceShowOSD(Song, bool)),
            visualisation_.get(), SLOT(SongMetadataChanged(Song)));
    connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
            visualisation_.get(), SLOT(SongMetadataChanged(Song)));

    visualisation_->SetEngine(
        qobject_cast<GstEngine*>(app_->player()->engine()));
  }

  visualisation_->show();
#endif  // ENABLE_VISUALISATIONS
}

void MainWindow::ConnectInfoView(SongInfoBase* view) {
  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)), view,
          SLOT(SongChanged(Song)));
  connect(app_->player(), SIGNAL(PlaylistFinished()), view,
          SLOT(SongFinished()));
  connect(app_->player(), SIGNAL(Stopped()), view, SLOT(SongFinished()));

  connect(view, SIGNAL(ShowSettingsDialog()), SLOT(ShowSongInfoConfig()));
  connect(view, SIGNAL(DoGlobalSearch(QString)), SLOT(DoGlobalSearch(QString)));
}

void MainWindow::AddSongInfoGenerator(smart_playlists::GeneratorPtr gen) {
  if (!gen) return;
  gen->set_library(app_->library_backend());

  AddToPlaylist(new smart_playlists::GeneratorMimeData(gen));
}

void MainWindow::ShowSongInfoConfig() {
  OpenSettingsDialogAtPage(SettingsDialog::Page_SongInformation);
}

void MainWindow::PlaylistViewSelectionModelChanged() {
  connect(ui_->playlist->view()->selectionModel(),
          SIGNAL(currentChanged(QModelIndex, QModelIndex)),
          SLOT(PlaylistCurrentChanged(QModelIndex)));
}

void MainWindow::PlaylistCurrentChanged(const QModelIndex& proxy_current) {
  const QModelIndex source_current =
      app_->playlist_manager()->current()->proxy()->mapToSource(proxy_current);

  // If the user moves the current index using the keyboard and then presses
  // F2, we don't want that editing the last column that was right clicked on.
  if (source_current != playlist_menu_index_)
    playlist_menu_index_ = QModelIndex();
}

void MainWindow::Raise() {
  show();
  activateWindow();
}

#ifdef Q_OS_WIN32
bool MainWindow::winEvent(MSG* msg, long*) {
  thumbbar_->HandleWinEvent(msg);
  return false;
}
#endif  // Q_OS_WIN32

void MainWindow::Exit() {
  SavePlaybackStatus();
  settings_.setValue("show_sidebar",
                     ui_->action_toggle_show_sidebar->isChecked());

  if (app_->player()->engine()->is_fadeout_enabled()) {
    // To shut down the application when fadeout will be finished
    connect(app_->player()->engine(), SIGNAL(FadeoutFinishedSignal()), qApp,
            SLOT(quit()));
    if (app_->player()->GetState() == Engine::Playing) {
      app_->player()->Stop();
      hide();
      tray_icon_->SetVisible(false);
      return;  // Don't quit the application now: wait for the fadeout finished
               // signal
    }
  }
  qApp->quit();
}

void MainWindow::AutoCompleteTags() {
  // Create the tag fetching stuff if it hasn't been already
  if (!tag_fetcher_) {
    tag_fetcher_.reset(new TagFetcher);
    track_selection_dialog_.reset(new TrackSelectionDialog);
    track_selection_dialog_->set_save_on_close(true);

    connect(tag_fetcher_.get(), SIGNAL(ResultAvailable(Song, SongList)),
            track_selection_dialog_.get(),
            SLOT(FetchTagFinished(Song, SongList)), Qt::QueuedConnection);
    connect(tag_fetcher_.get(), SIGNAL(Progress(Song, QString)),
            track_selection_dialog_.get(),
            SLOT(FetchTagProgress(Song, QString)));
    connect(track_selection_dialog_.get(), SIGNAL(accepted()),
            SLOT(AutoCompleteTagsAccepted()));
    connect(track_selection_dialog_.get(), SIGNAL(finished(int)),
            tag_fetcher_.get(), SLOT(Cancel()));
    connect(track_selection_dialog_.get(), SIGNAL(Error(QString)),
            SLOT(ShowErrorDialog(QString)));
  }

  // Get the selected songs and start fetching tags for them
  SongList songs;
  autocomplete_tag_items_.clear();
  for (const QModelIndex& index :
       ui_->playlist->view()->selectionModel()->selection().indexes()) {
    if (index.column() != 0) continue;
    int row =
        app_->playlist_manager()->current()->proxy()->mapToSource(index).row();
    PlaylistItemPtr item(app_->playlist_manager()->current()->item_at(row));
    Song song = item->Metadata();

    if (song.IsEditable()) {
      songs << song;
      autocomplete_tag_items_ << item;
    }
  }

  track_selection_dialog_->Init(songs);
  tag_fetcher_->StartFetch(songs);

  track_selection_dialog_->show();
}

void MainWindow::AutoCompleteTagsAccepted() {
  for (PlaylistItemPtr item : autocomplete_tag_items_) {
    item->Reload();
  }

  // This is really lame but we don't know what rows have changed
  ui_->playlist->view()->update();
}

QPixmap MainWindow::CreateOverlayedIcon(int position, int scrobble_point) {
  QPixmap normal_icon =
      IconLoader::Load("as_light", IconLoader::Lastfm).pixmap(16);
  QPixmap light_icon = IconLoader::Load("as", IconLoader::Lastfm).pixmap(16);
  QRect rect(normal_icon.rect());

  // calculates the progress
  double perc = 1.0 - ((double)position / (double)scrobble_point);

  QPolygon mask;
  mask << rect.topRight();
  mask << rect.topLeft();
  mask << QPoint(rect.left(), rect.height() * perc);
  mask << QPoint(rect.right(), (rect.height()) * perc);

  QPixmap ret(light_icon);
  QPainter p(&ret);

  // Draw the red icon over the light red one
  p.setClipRegion(mask);
  p.drawPixmap(0, 0, normal_icon);
  p.setClipping(false);

  p.end();

  return ret;
}

void MainWindow::SetToggleScrobblingIcon(bool value) {
  if (!value) {
    ui_->action_toggle_scrobbling->setIcon(
        IconLoader::Load("as_disabled", IconLoader::Lastfm));
  } else {
    ui_->action_toggle_scrobbling->setIcon(
        IconLoader::Load("as_light", IconLoader::Lastfm));
  }
}

#ifdef HAVE_LIBLASTFM
void MainWindow::ScrobbleSubmitted() {
  const bool last_fm_enabled = ui_->action_toggle_scrobbling->isVisible() &&
                               app_->scrobbler()->IsScrobblingEnabled() &&
                               app_->scrobbler()->IsAuthenticated();

  app_->playlist_manager()->active()->set_lastfm_status(
      Playlist::LastFM_Scrobbled);

  // update the button icon
  if (last_fm_enabled)
    ui_->action_toggle_scrobbling->setIcon(
        IconLoader::Load("as", IconLoader::Lastfm));
}

void MainWindow::ScrobbleError(int value) {
  switch (value) {
    case -1:
      // custom error value got from initial validity check
      app_->playlist_manager()->active()->set_lastfm_status(
          Playlist::LastFM_Invalid);
      break;

    case 30:
      // Hack: when offline, liblastfm doesn't inform us, so set the status
      // as queued; in this way we won't try to scrobble again, it will be done
      // automatically
      app_->playlist_manager()->active()->set_lastfm_status(
          Playlist::LastFM_Queued);
      break;

    default:
      if (value > 3) {
        // we're for sure in an error state
        app_->playlist_manager()->active()->set_lastfm_status(
            Playlist::LastFM_Error);
        qLog(Warning) << "Last.fm scrobbling error: " << value;
      }
      break;
  }
}
#endif

void MainWindow::HandleNotificationPreview(OSD::Behaviour type, QString line1,
                                           QString line2) {
  if (!app_->playlist_manager()->current()->GetAllSongs().isEmpty()) {
    // Show a preview notification for the first song in the current playlist
    osd_->ShowPreview(
        type, line1, line2,
        app_->playlist_manager()->current()->GetAllSongs().first());
  } else {
    qLog(Debug) << "The current playlist is empty, showing a fake song";
    // Create a fake song
    Song fake;
    fake.Init("Title", "Artist", "Album", 123);
    fake.set_genre("Classical");
    fake.set_composer("Anonymous");
    fake.set_performer("Anonymous");
    fake.set_lyrics("None");
    fake.set_track(1);
    fake.set_disc(1);
    fake.set_year(2011);

    osd_->ShowPreview(type, line1, line2, fake);
  }
}

void MainWindow::ScrollToInternetIndex(const QModelIndex& index) {
  internet_view_->ScrollToIndex(index);
  ui_->tabs->SetCurrentWidget(internet_view_);
}

void MainWindow::AddPodcast() {
  app_->internet_model()->Service<PodcastService>()->AddPodcast();
}

void MainWindow::FocusLibraryTab() {
  ui_->tabs->SetCurrentWidget(library_view_);
}

void MainWindow::FocusGlobalSearchField() {
  ui_->tabs->SetCurrentWidget(global_search_view_);
  global_search_view_->FocusSearchField();
}

void MainWindow::DoGlobalSearch(const QString& query) {
  FocusGlobalSearchField();
  global_search_view_->StartSearch(query);
}

void MainWindow::ShowConsole() {
  Console* console = new Console(app_, this);
  console->show();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Space) {
    app_->player()->PlayPause();
    event->accept();
  } else if (event->key() == Qt::Key_Left) {
    app_->player()->SeekBackward();
    event->accept();
  } else if (event->key() == Qt::Key_Right) {
    app_->player()->SeekForward();
    event->accept();
  } else {
    QMainWindow::keyPressEvent(event);
  }
}
