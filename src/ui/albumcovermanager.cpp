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

#include "albumcovermanager.h"
#include "albumcoversearcher.h"
#include "iconloader.h"
#include "ui_albumcovermanager.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "covers/albumcoverexporter.h"
#include "covers/albumcoverfetcher.h"
#include "covers/albumcoverloader.h"
#include "covers/coverproviders.h"
#include "covers/coversearchstatisticsdialog.h"
#include "library/librarybackend.h"
#include "library/libraryquery.h"
#include "library/sqlrow.h"
#include "playlist/songmimedata.h"
#include "widgets/forcescrollperpixel.h"
#include "ui/albumcoverchoicecontroller.h"
#include "ui/albumcoverexport.h"
#include "ui/iconloader.h"

#include <algorithm>

#include <QActionGroup>
#include <QPushButton>
#include <QContextMenuEvent>
#include <QEvent>
#include <QFileDialog>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QProgressBar>
#include <QSettings>
#include <QShortcut>
#include <QTimer>

const char* AlbumCoverManager::kSettingsGroup = "CoverManager";

AlbumCoverManager::AlbumCoverManager(Application* app,
                                     LibraryBackend* library_backend,
                                     QWidget* parent,
                                     QNetworkAccessManager* network)
    : QMainWindow(parent),
      ui_(new Ui_CoverManager),
      app_(app),
      album_cover_choice_controller_(new AlbumCoverChoiceController(this)),
      cover_fetcher_(
          new AlbumCoverFetcher(app_->cover_providers(), this, network)),
      cover_searcher_(nullptr),
      cover_export_(nullptr),
      cover_exporter_(new AlbumCoverExporter(this)),
      artist_icon_(IconLoader::Load("x-clementine-artist", IconLoader::Base)),
      all_artists_icon_(
          IconLoader::Load("x-clementine-album", IconLoader::Base)),
      no_cover_icon_(IconLoader::Load("nocover", IconLoader::Other)),
      no_cover_image_(GenerateNoCoverImage(no_cover_icon_)),
      no_cover_item_icon_(QPixmap::fromImage(no_cover_image_)),
      context_menu_(new QMenu(this)),
      progress_bar_(new QProgressBar(this)),
      abort_progress_(new QPushButton(this)),
      jobs_(0),
      library_backend_(library_backend) {
  ui_->setupUi(this);
  ui_->albums->set_cover_manager(this);

  // Icons
  ui_->action_fetch->setIcon(IconLoader::Load("download", IconLoader::Base));
  ui_->export_covers->setIcon(
      IconLoader::Load("document-save", IconLoader::Base));
  ui_->view->setIcon(IconLoader::Load("view-choose", IconLoader::Base));
  ui_->fetch->setIcon(IconLoader::Load("download", IconLoader::Base));
  ui_->action_add_to_playlist->setIcon(
      IconLoader::Load("media-playback-start", IconLoader::Base));
  ui_->action_load->setIcon(
      IconLoader::Load("media-playback-start", IconLoader::Base));

  album_cover_choice_controller_->SetApplication(app_);

  cover_searcher_ = new AlbumCoverSearcher(no_cover_item_icon_, app_, this);
  cover_export_ = new AlbumCoverExport(this);

  // Set up the status bar
  statusBar()->addPermanentWidget(progress_bar_);
  statusBar()->addPermanentWidget(abort_progress_);
  progress_bar_->hide();
  abort_progress_->hide();
  abort_progress_->setText(tr("Abort"));
  connect(abort_progress_, SIGNAL(clicked()), this, SLOT(CancelRequests()));

  ui_->albums->setAttribute(Qt::WA_MacShowFocusRect, false);
  ui_->artists->setAttribute(Qt::WA_MacShowFocusRect, false);

  QShortcut* close = new QShortcut(QKeySequence::Close, this);
  connect(close, SIGNAL(activated()), SLOT(close()));

  EnableCoversButtons();
}

AlbumCoverManager::~AlbumCoverManager() {
  CancelRequests();

  delete ui_;
}

LibraryBackend* AlbumCoverManager::backend() const { return library_backend_; }

void AlbumCoverManager::Init() {
  // View menu
  QActionGroup* filter_group = new QActionGroup(this);
  filter_all_ = filter_group->addAction(tr("All albums"));
  filter_with_covers_ = filter_group->addAction(tr("Albums with covers"));
  filter_without_covers_ = filter_group->addAction(tr("Albums without covers"));
  filter_all_->setCheckable(true);
  filter_with_covers_->setCheckable(true);
  filter_without_covers_->setCheckable(true);
  filter_group->setExclusive(true);
  filter_all_->setChecked(true);

  QMenu* view_menu = new QMenu(this);
  view_menu->addActions(filter_group->actions());

  ui_->view->setMenu(view_menu);

  // Context menu

  QList<QAction*> actions = album_cover_choice_controller_->GetAllActions();

  connect(album_cover_choice_controller_->cover_from_file_action(),
          SIGNAL(triggered()), this, SLOT(LoadCoverFromFile()));
  connect(album_cover_choice_controller_->cover_to_file_action(),
          SIGNAL(triggered()), this, SLOT(SaveCoverToFile()));
  connect(album_cover_choice_controller_->cover_from_url_action(),
          SIGNAL(triggered()), this, SLOT(LoadCoverFromURL()));
  connect(album_cover_choice_controller_->search_for_cover_action(),
          SIGNAL(triggered()), this, SLOT(SearchForCover()));
  connect(album_cover_choice_controller_->unset_cover_action(),
          SIGNAL(triggered()), this, SLOT(UnsetCover()));
  connect(album_cover_choice_controller_->show_cover_action(),
          SIGNAL(triggered()), this, SLOT(ShowCover()));

  connect(cover_exporter_, SIGNAL(AlbumCoversExportUpdate(int, int, int)),
          SLOT(UpdateExportStatus(int, int, int)));

  context_menu_->addActions(actions);
  context_menu_->addSeparator();
  context_menu_->addAction(ui_->action_load);
  context_menu_->addAction(ui_->action_add_to_playlist);

  ui_->albums->installEventFilter(this);

  // Connections
  connect(ui_->artists,
          SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
          SLOT(ArtistChanged(QListWidgetItem*)));
  connect(ui_->filter, SIGNAL(textChanged(QString)), SLOT(UpdateFilter()));
  connect(filter_group, SIGNAL(triggered(QAction*)), SLOT(UpdateFilter()));
  connect(ui_->view, SIGNAL(clicked()), ui_->view, SLOT(showMenu()));
  connect(ui_->fetch, SIGNAL(clicked()), SLOT(FetchAlbumCovers()));
  connect(ui_->export_covers, SIGNAL(clicked()), SLOT(ExportCovers()));
  connect(cover_fetcher_,
          SIGNAL(AlbumCoverFetched(quint64, QImage, CoverSearchStatistics)),
          SLOT(AlbumCoverFetched(quint64, QImage, CoverSearchStatistics)));
  connect(ui_->action_fetch, SIGNAL(triggered()), SLOT(FetchSingleCover()));
  connect(ui_->albums, SIGNAL(doubleClicked(QModelIndex)),
          SLOT(AlbumDoubleClicked(QModelIndex)));
  connect(ui_->action_add_to_playlist, SIGNAL(triggered()),
          SLOT(AddSelectedToPlaylist()));
  connect(ui_->action_load, SIGNAL(triggered()),
          SLOT(LoadSelectedToPlaylist()));

  // Restore settings
  QSettings s;
  s.beginGroup(kSettingsGroup);

  restoreGeometry(s.value("geometry").toByteArray());
  if (!ui_->splitter->restoreState(s.value("splitter_state").toByteArray())) {
    // Sensible default size for the artists view
    ui_->splitter->setSizes(QList<int>() << 200 << width() - 200);
  }

  connect(app_->album_cover_loader(), SIGNAL(ImageLoaded(quint64, QImage)),
          SLOT(CoverImageLoaded(quint64, QImage)));

  cover_searcher_->Init(cover_fetcher_);

  new ForceScrollPerPixel(ui_->albums, this);
}

void AlbumCoverManager::showEvent(QShowEvent*) { Reset(); }

void AlbumCoverManager::closeEvent(QCloseEvent* e) {
  if (!cover_fetching_tasks_.isEmpty()) {
    std::unique_ptr<QMessageBox> message_box(new QMessageBox(
        QMessageBox::Question, tr("Really cancel?"),
        tr("Closing this window will stop searching for album covers."),
        QMessageBox::Abort, this));
    message_box->addButton(tr("Don't stop!"), QMessageBox::AcceptRole);

    if (message_box->exec() != QMessageBox::Abort) {
      e->ignore();
      return;
    }
  }

  // Save geometry
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("geometry", saveGeometry());
  s.setValue("splitter_state", ui_->splitter->saveState());

  // Cancel any outstanding requests
  CancelRequests();
}

void AlbumCoverManager::CancelRequests() {
  app_->album_cover_loader()->CancelTasks(
      QSet<quint64>::fromList(cover_loading_tasks_.keys()));
  cover_loading_tasks_.clear();

  cover_exporter_->Cancel();

  cover_fetching_tasks_.clear();
  cover_fetcher_->Clear();
  progress_bar_->hide();
  abort_progress_->hide();
  statusBar()->clearMessage();
  EnableCoversButtons();
}

static bool CompareNocase(const QString& left, const QString& right) {
  return QString::localeAwareCompare(left, right) < 0;
}

static bool CompareAlbumNameNocase(const LibraryBackend::Album& left,
                                   const LibraryBackend::Album& right) {
  return CompareNocase(left.album_name, right.album_name);
}

void AlbumCoverManager::Reset() {
  EnableCoversButtons();

  ui_->artists->clear();
  new QListWidgetItem(all_artists_icon_, tr("All artists"), ui_->artists,
                      All_Artists);
  new QListWidgetItem(artist_icon_, tr("Various artists"), ui_->artists,
                      Various_Artists);

  QStringList artists(library_backend_->GetAllArtistsWithAlbums());
  std::stable_sort(artists.begin(), artists.end(), CompareNocase);

  for (const QString& artist : artists) {
    if (artist.isEmpty()) continue;

    new QListWidgetItem(artist_icon_, artist, ui_->artists, Specific_Artist);
  }
}

void AlbumCoverManager::EnableCoversButtons() {
  ui_->fetch->setEnabled(app_->cover_providers()->HasAnyProviders());
  ui_->export_covers->setEnabled(true);
}

void AlbumCoverManager::DisableCoversButtons() {
  ui_->fetch->setEnabled(false);
  ui_->export_covers->setEnabled(false);
}

void AlbumCoverManager::ArtistChanged(QListWidgetItem* current) {
  if (!current) return;

  QString artist;
  if (current->type() == Specific_Artist) artist = current->text();

  ui_->albums->clear();
  context_menu_items_.clear();
  CancelRequests();

  // Get the list of albums.  How we do it depends on what thing we have
  // selected in the artist list.
  LibraryBackend::AlbumList albums;
  switch (current->type()) {
    case Various_Artists:
      albums = library_backend_->GetCompilationAlbums();
      break;
    case Specific_Artist:
      albums = library_backend_->GetAlbumsByArtist(current->text());
      albums += library_backend_->GetAlbumsByAlbumArtist(current->text());
      break;
    case All_Artists:
    default:
      albums = library_backend_->GetAllAlbums();
      break;
  }

  // Sort by album name.  The list is already sorted by sqlite but it was done
  // case sensitively.
  std::stable_sort(albums.begin(), albums.end(), CompareAlbumNameNocase);

  for (const LibraryBackend::Album& info : albums) {
    // Don't show songs without an album, obviously
    if (info.album_name.isEmpty()) continue;

    QListWidgetItem* item =
        new QListWidgetItem(no_cover_item_icon_, info.album_name, ui_->albums);

    item->setData(Role_ArtistName, info.artist);
    item->setData(Role_AlbumArtistName, info.album_artist);
    item->setData(Role_AlbumName, info.album_name);
    item->setData(Role_FirstUrl, info.first_url);
    item->setData(Qt::TextAlignmentRole,
                  QVariant(Qt::AlignTop | Qt::AlignHCenter));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                   Qt::ItemIsDragEnabled);

    QString effective_artist = EffectiveAlbumArtistName(*item);
    if (!artist.isEmpty()) {
      item->setToolTip(effective_artist + " - " + info.album_name);
    } else {
      item->setToolTip(info.album_name);
    }

    if (!info.art_automatic.isEmpty() || !info.art_manual.isEmpty()) {
      quint64 id = app_->album_cover_loader()->LoadImageAsync(
          cover_loader_options_, info.art_automatic, info.art_manual,
          info.first_url.toLocalFile());
      item->setData(Role_PathAutomatic, info.art_automatic);
      item->setData(Role_PathManual, info.art_manual);
      cover_loading_tasks_[id] = item;
    }
  }

  UpdateFilter();
}

void AlbumCoverManager::CoverImageLoaded(quint64 id, const QImage& image) {
  if (!cover_loading_tasks_.contains(id)) return;

  QListWidgetItem* item = cover_loading_tasks_.take(id);

  if (image.isNull()) return;

  item->setIcon(QPixmap::fromImage(image));
  UpdateFilter();
}

void AlbumCoverManager::UpdateFilter() {
  const QString filter = ui_->filter->text().toLower();
  const bool hide_with_covers = filter_without_covers_->isChecked();
  const bool hide_without_covers = filter_with_covers_->isChecked();

  HideCovers hide = Hide_None;
  if (hide_with_covers) {
    hide = Hide_WithCovers;
  } else if (hide_without_covers) {
    hide = Hide_WithoutCovers;
  }

  qint32 total_count = 0;
  qint32 without_cover = 0;

  for (int i = 0; i < ui_->albums->count(); ++i) {
    QListWidgetItem* item = ui_->albums->item(i);
    bool should_hide = ShouldHide(*item, filter, hide);
    item->setHidden(should_hide);

    if (!should_hide) {
      total_count++;
      if (!ItemHasCover(*item)) {
        without_cover++;
      }
    }
  }

  ui_->total_albums->setText(QString::number(total_count));
  ui_->without_cover->setText(QString::number(without_cover));
}

bool AlbumCoverManager::ShouldHide(const QListWidgetItem& item,
                                   const QString& filter,
                                   HideCovers hide) const {
  bool has_cover = ItemHasCover(item);
  if (hide == Hide_WithCovers && has_cover) {
    return true;
  } else if (hide == Hide_WithoutCovers && !has_cover) {
    return true;
  }

  if (filter.isEmpty()) {
    return false;
  }

  QStringList query = filter.split(' ');
  for (const QString& s : query) {
    bool in_text = item.text().contains(s, Qt::CaseInsensitive);
    bool in_artist =
        item.data(Role_ArtistName).toString().contains(s, Qt::CaseInsensitive);
    bool in_albumartist = item.data(Role_AlbumArtistName).toString().contains(
        s, Qt::CaseInsensitive);
    if (!in_text && !in_artist && !in_albumartist) {
      return true;
    }
  }

  return false;
}

void AlbumCoverManager::FetchAlbumCovers() {
  for (int i = 0; i < ui_->albums->count(); ++i) {
    QListWidgetItem* item = ui_->albums->item(i);
    if (item->isHidden()) continue;
    if (ItemHasCover(*item)) continue;

    quint64 id = cover_fetcher_->FetchAlbumCover(
        EffectiveAlbumArtistName(*item), item->data(Role_AlbumName).toString());
    cover_fetching_tasks_[id] = item;
    jobs_++;
  }

  if (!cover_fetching_tasks_.isEmpty()) ui_->fetch->setEnabled(false);

  progress_bar_->setMaximum(jobs_);
  progress_bar_->show();
  abort_progress_->show();
  fetch_statistics_ = CoverSearchStatistics();
  UpdateStatusText();
}

void AlbumCoverManager::AlbumCoverFetched(
    quint64 id, const QImage& image, const CoverSearchStatistics& statistics) {
  if (!cover_fetching_tasks_.contains(id)) return;

  QListWidgetItem* item = cover_fetching_tasks_.take(id);
  if (!image.isNull()) {
    SaveAndSetCover(item, image);
  }

  if (cover_fetching_tasks_.isEmpty()) {
    EnableCoversButtons();
  }

  fetch_statistics_ += statistics;
  UpdateStatusText();
}

void AlbumCoverManager::UpdateStatusText() {
  QString message = tr("Got %1 covers out of %2 (%3 failed)")
                        .arg(fetch_statistics_.chosen_images_)
                        .arg(jobs_)
                        .arg(fetch_statistics_.missing_images_);

  if (fetch_statistics_.bytes_transferred_) {
    message += ", " +
               tr("%1 transferred").arg(
                   Utilities::PrettySize(fetch_statistics_.bytes_transferred_));
  }

  statusBar()->showMessage(message);
  progress_bar_->setValue(fetch_statistics_.chosen_images_ +
                          fetch_statistics_.missing_images_);

  if (cover_fetching_tasks_.isEmpty()) {
    QTimer::singleShot(2000, statusBar(), SLOT(clearMessage()));
    progress_bar_->hide();
    abort_progress_->hide();

    CoverSearchStatisticsDialog* dialog = new CoverSearchStatisticsDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->Show(fetch_statistics_);

    jobs_ = 0;
  }
}

bool AlbumCoverManager::eventFilter(QObject* obj, QEvent* event) {
  if (obj == ui_->albums && event->type() == QEvent::ContextMenu) {
    context_menu_items_ = ui_->albums->selectedItems();
    if (context_menu_items_.isEmpty()) return false;

    bool some_with_covers = false;

    for (QListWidgetItem* item : context_menu_items_) {
      if (ItemHasCover(*item)) some_with_covers = true;
    }

    album_cover_choice_controller_->cover_from_file_action()->setEnabled(
        context_menu_items_.size() == 1);
    album_cover_choice_controller_->cover_from_url_action()->setEnabled(
        context_menu_items_.size() == 1);
    album_cover_choice_controller_->show_cover_action()->setEnabled(
        some_with_covers && context_menu_items_.size() == 1);
    album_cover_choice_controller_->unset_cover_action()->setEnabled(
        some_with_covers);
    album_cover_choice_controller_->search_for_cover_action()->setEnabled(
        app_->cover_providers()->HasAnyProviders());

    QContextMenuEvent* e = static_cast<QContextMenuEvent*>(event);
    context_menu_->popup(e->globalPos());
    return true;
  }
  return QMainWindow::eventFilter(obj, event);
}

Song AlbumCoverManager::GetSingleSelectionAsSong() {
  return context_menu_items_.size() != 1 ? Song()
                                         : ItemAsSong(context_menu_items_[0]);
}

Song AlbumCoverManager::GetFirstSelectedAsSong() {
  return context_menu_items_.isEmpty() ? Song()
                                       : ItemAsSong(context_menu_items_[0]);
}

Song AlbumCoverManager::ItemAsSong(QListWidgetItem* item) {
  Song result;

  QString title = item->data(Role_AlbumName).toString();
  QString artist_name = EffectiveAlbumArtistName(*item);
  if (!artist_name.isEmpty()) {
    result.set_title(artist_name + " - " + title);
  } else {
    result.set_title(title);
  }

  result.set_artist(item->data(Role_ArtistName).toString());
  result.set_albumartist(item->data(Role_AlbumArtistName).toString());
  result.set_album(item->data(Role_AlbumName).toString());

  result.set_url(item->data(Role_FirstUrl).toUrl());

  result.set_art_automatic(item->data(Role_PathAutomatic).toString());
  result.set_art_manual(item->data(Role_PathManual).toString());

  // force validity
  result.set_valid(true);
  result.set_id(0);

  return result;
}

void AlbumCoverManager::ShowCover() {
  Song song = GetSingleSelectionAsSong();
  if (!song.is_valid()) return;

  album_cover_choice_controller_->ShowCover(song);
}

void AlbumCoverManager::FetchSingleCover() {
  for (QListWidgetItem* item : context_menu_items_) {
    quint64 id = cover_fetcher_->FetchAlbumCover(
        EffectiveAlbumArtistName(*item), item->data(Role_AlbumName).toString());
    cover_fetching_tasks_[id] = item;
    jobs_++;
  }

  progress_bar_->setMaximum(jobs_);
  progress_bar_->show();
  abort_progress_->show();
  UpdateStatusText();
}

void AlbumCoverManager::UpdateCoverInList(QListWidgetItem* item,
                                          const QString& cover) {
  quint64 id = app_->album_cover_loader()->LoadImageAsync(cover_loader_options_,
                                                          QString(), cover);
  item->setData(Role_PathManual, cover);
  cover_loading_tasks_[id] = item;
}

void AlbumCoverManager::LoadCoverFromFile() {
  Song song = GetSingleSelectionAsSong();
  if (!song.is_valid()) return;

  QListWidgetItem* item = context_menu_items_[0];

  QString cover = album_cover_choice_controller_->LoadCoverFromFile(&song);

  if (!cover.isEmpty()) {
    UpdateCoverInList(item, cover);
  }
}

void AlbumCoverManager::SaveCoverToFile() {
  Song song = GetSingleSelectionAsSong();
  if (!song.is_valid()) return;

  QImage image;

  // load the image from disk
  if (song.has_manually_unset_cover()) {
    image = no_cover_image_;
  } else {
    if (!song.art_manual().isEmpty() && QFile::exists(song.art_manual())) {
      image = QImage(song.art_manual());
    } else if (!song.art_automatic().isEmpty() &&
               QFile::exists(song.art_automatic())) {
      image = QImage(song.art_automatic());
    } else {
      image = no_cover_image_;
    }
  }

  album_cover_choice_controller_->SaveCoverToFile(song, image);
}

void AlbumCoverManager::LoadCoverFromURL() {
  Song song = GetSingleSelectionAsSong();
  if (!song.is_valid()) return;

  QListWidgetItem* item = context_menu_items_[0];

  QString cover = album_cover_choice_controller_->LoadCoverFromURL(&song);

  if (!cover.isEmpty()) {
    UpdateCoverInList(item, cover);
  }
}

void AlbumCoverManager::SearchForCover() {
  Song song = GetFirstSelectedAsSong();
  if (!song.is_valid()) return;

  QListWidgetItem* item = context_menu_items_[0];

  QString cover = album_cover_choice_controller_->SearchForCover(&song);
  if (cover.isEmpty()) return;

  // force the found cover on all of the selected items
  for (QListWidgetItem* current : context_menu_items_) {
    // don't save the first one twice
    if (current != item) {
      Song current_song = ItemAsSong(current);
      album_cover_choice_controller_->SaveCover(&current_song, cover);
    }

    UpdateCoverInList(current, cover);
  }
}

void AlbumCoverManager::UnsetCover() {
  Song song = GetFirstSelectedAsSong();
  if (!song.is_valid()) return;

  QListWidgetItem* item = context_menu_items_[0];

  QString cover = album_cover_choice_controller_->UnsetCover(&song);

  // force the 'none' cover on all of the selected items
  for (QListWidgetItem* current : context_menu_items_) {
    current->setIcon(no_cover_item_icon_);
    current->setData(Role_PathManual, cover);

    // don't save the first one twice
    if (current != item) {
      Song current_song = ItemAsSong(current);
      album_cover_choice_controller_->SaveCover(&current_song, cover);
    }
  }
}

SongList AlbumCoverManager::GetSongsInAlbum(const QModelIndex& index) const {
  SongList ret;

  LibraryQuery q;
  q.SetColumnSpec("ROWID," + Song::kColumnSpec);
  q.AddWhere("album", index.data(Role_AlbumName).toString());
  q.SetOrderBy("disc, track, title");

  QString artist = index.data(Role_ArtistName).toString();
  QString albumartist = index.data(Role_AlbumArtistName).toString();

  if (!albumartist.isEmpty()) {
    q.AddWhere("albumartist", albumartist);
  } else if (!artist.isEmpty()) {
    q.AddWhere("artist", artist);
  }

  q.AddCompilationRequirement(artist.isEmpty() && albumartist.isEmpty());

  if (!library_backend_->ExecQuery(&q)) return ret;

  while (q.Next()) {
    Song song;
    song.InitFromQuery(q, true);
    ret << song;
  }
  return ret;
}

SongList AlbumCoverManager::GetSongsInAlbums(
    const QModelIndexList& indexes) const {
  SongList ret;
  for (const QModelIndex& index : indexes) {
    ret << GetSongsInAlbum(index);
  }
  return ret;
}

SongMimeData* AlbumCoverManager::GetMimeDataForAlbums(
    const QModelIndexList& indexes) const {
  SongList songs = GetSongsInAlbums(indexes);
  if (songs.isEmpty()) return nullptr;

  SongMimeData* data = new SongMimeData;
  data->backend = library_backend_;
  data->songs = songs;
  return data;
}

void AlbumCoverManager::AlbumDoubleClicked(const QModelIndex& index) {
  SongMimeData* data = GetMimeDataForAlbums(QModelIndexList() << index);
  if (data) {
    data->from_doubleclick_ = true;
    emit AddToPlaylist(data);
  }
}

void AlbumCoverManager::AddSelectedToPlaylist() {
  emit AddToPlaylist(
      GetMimeDataForAlbums(ui_->albums->selectionModel()->selectedIndexes()));
}

void AlbumCoverManager::LoadSelectedToPlaylist() {
  SongMimeData* data =
      GetMimeDataForAlbums(ui_->albums->selectionModel()->selectedIndexes());
  if (data) {
    data->clear_first_ = true;
    emit AddToPlaylist(data);
  }
}

void AlbumCoverManager::SaveAndSetCover(QListWidgetItem* item,
                                        const QImage& image) {
  const QString artist = item->data(Role_ArtistName).toString();
  const QString albumartist = item->data(Role_AlbumArtistName).toString();
  const QString album = item->data(Role_AlbumName).toString();

  QString path =
      album_cover_choice_controller_->SaveCoverInCache(artist, album, image);

  // Save the image in the database
  library_backend_->UpdateManualAlbumArtAsync(artist, albumartist, album, path);

  // Update the icon in our list
  quint64 id = app_->album_cover_loader()->LoadImageAsync(cover_loader_options_,
                                                          QString(), path);
  item->setData(Role_PathManual, path);
  cover_loading_tasks_[id] = item;
}

void AlbumCoverManager::ExportCovers() {
  AlbumCoverExport::DialogResult result = cover_export_->Exec();

  if (result.cancelled_) {
    return;
  }

  DisableCoversButtons();

  cover_exporter_->SetDialogResult(result);

  for (int i = 0; i < ui_->albums->count(); ++i) {
    QListWidgetItem* item = ui_->albums->item(i);

    // skip hidden and coverless albums
    if (item->isHidden() || !ItemHasCover(*item)) {
      continue;
    }

    cover_exporter_->AddExportRequest(ItemAsSong(item));
  }

  if (cover_exporter_->request_count() > 0) {
    progress_bar_->setMaximum(cover_exporter_->request_count());
    progress_bar_->show();
    abort_progress_->show();

    cover_exporter_->StartExporting();
  } else {
    QMessageBox msg;
    msg.setWindowTitle(tr("Export finished"));
    msg.setText(tr("No covers to export."));
    msg.exec();
  }
}

void AlbumCoverManager::UpdateExportStatus(int exported, int skipped, int max) {
  progress_bar_->setValue(exported);

  QString message = tr("Exported %1 covers out of %2 (%3 skipped)")
                        .arg(exported)
                        .arg(max)
                        .arg(skipped);
  statusBar()->showMessage(message);

  // end of the current process
  if (exported + skipped >= max) {
    QTimer::singleShot(2000, statusBar(), SLOT(clearMessage()));

    progress_bar_->hide();
    abort_progress_->hide();
    EnableCoversButtons();

    QMessageBox msg;
    msg.setWindowTitle(tr("Export finished"));
    msg.setText(message);
    msg.exec();
  }
}

QString AlbumCoverManager::EffectiveAlbumArtistName(
    const QListWidgetItem& item) const {
  QString albumartist = item.data(Role_AlbumArtistName).toString();
  if (!albumartist.isEmpty()) {
    return albumartist;
  }
  return item.data(Role_ArtistName).toString();
}

QImage AlbumCoverManager::GenerateNoCoverImage(
    const QIcon& no_cover_icon) const {
  // Get a square version of nocover.png with some transparency:
  QImage nocover =
      no_cover_icon.pixmap(no_cover_icon.availableSizes().last()).toImage();
  nocover =
      nocover.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  QImage square_nocover(120, 120, QImage::Format_ARGB32);
  square_nocover.fill(0);
  QPainter p(&square_nocover);
  p.setOpacity(0.4);
  p.drawImage((120 - nocover.width()) / 2, (120 - nocover.height()) / 2,
              nocover);
  p.end();

  return square_nocover;
}

bool AlbumCoverManager::ItemHasCover(const QListWidgetItem& item) const {
  return item.icon().cacheKey() != no_cover_item_icon_.cacheKey();
}
