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

#include "albumcovermanager.h"
#include "albumcoverfetcher.h"
#include "librarybackend.h"
#include "libraryquery.h"

#include <QSettings>
#include <QPainter>
#include <QMenu>
#include <QActionGroup>
#include <QListWidget>
#include <QCryptographicHash>
#include <QDir>
#include <QEvent>
#include <QScrollBar>
#include <QContextMenuEvent>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

const char* AlbumCoverManager::kSettingsGroup = "CoverManager";

AlbumCoverManager::AlbumCoverManager(QNetworkAccessManager* network, QWidget *parent)
  : QDialog(parent),
    constructed_(false),
    cover_loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    cover_fetcher_(new AlbumCoverFetcher(network, this)),
    artist_icon_(":/artist.png"),
    all_artists_icon_(":/album.png"),
    context_menu_(new QMenu(this)) {
  ui_.setupUi(this);

  // Get a square version of nocover.png
  QImage nocover(":/nocover.png");
  nocover = nocover.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  QImage square_nocover(120, 120, QImage::Format_ARGB32);
  square_nocover.fill(0);
  QPainter p(&square_nocover);
  p.drawImage((120 - nocover.width()) / 2, (120 - nocover.height()) / 2, nocover);
  p.end();
  no_cover_icon_ = QPixmap::fromImage(square_nocover);
}

AlbumCoverManager::~AlbumCoverManager() {
  CancelRequests();
}

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

  ui_.view->setMenu(view_menu);

  // Context menu
  context_menu_->addAction(ui_.action_show_fullsize);
  context_menu_->addAction(ui_.action_fetch);
  context_menu_->addAction(ui_.action_choose_manual);
  context_menu_->addSeparator();
  context_menu_->addAction(ui_.action_unset_cover);
  ui_.albums->installEventFilter(this);

  // Connections
  connect(cover_loader_, SIGNAL(Initialised()), SLOT(CoverLoaderInitialised()));
  connect(ui_.artists, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          SLOT(ArtistChanged(QListWidgetItem*)));
  connect(ui_.filter, SIGNAL(textChanged(QString)), SLOT(UpdateFilter()));
  connect(filter_group, SIGNAL(triggered(QAction*)), SLOT(UpdateFilter()));
  connect(ui_.view, SIGNAL(clicked()), ui_.view, SLOT(showMenu()));
  connect(ui_.fetch, SIGNAL(clicked()), SLOT(FetchAlbumCovers()));
  connect(cover_fetcher_, SIGNAL(AlbumCoverFetched(quint64,QImage)),
          SLOT(AlbumCoverFetched(quint64,QImage)));
  connect(ui_.action_show_fullsize, SIGNAL(triggered()), SLOT(ShowFullsize()));
  connect(ui_.action_fetch, SIGNAL(triggered()), SLOT(FetchSingleCover()));
  connect(ui_.action_choose_manual, SIGNAL(triggered()), SLOT(ChooseManualCover()));
  connect(ui_.action_unset_cover, SIGNAL(triggered()), SLOT(UnsetCover()));

  // Restore settings
  QSettings s;
  s.beginGroup(kSettingsGroup);

  restoreGeometry(s.value("geometry").toByteArray());
  if (!ui_.splitter->restoreState(s.value("splitter_state").toByteArray())) {
    // Sensible default size for the artists view
    ui_.splitter->setSizes(QList<int>() << 200 << width() - 200);
  }

  cover_loader_->Start();
  constructed_ = true;
}

void AlbumCoverManager::CoverLoaderInitialised() {
  connect(cover_loader_->Worker().get(), SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(CoverImageLoaded(quint64,QImage)));
}

void AlbumCoverManager::SetBackend(boost::shared_ptr<LibraryBackendInterface> backend) {
  backend_ = backend;

  if (isVisible())
    Reset();
}

void AlbumCoverManager::showEvent(QShowEvent *) {
  Reset();
}

void AlbumCoverManager::closeEvent(QCloseEvent *) {
  // Save geometry
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("geometry", saveGeometry());
  s.setValue("splitter_state", ui_.splitter->saveState());

  // Cancel any outstanding requests
  CancelRequests();
}

void AlbumCoverManager::CancelRequests() {
  cover_loading_tasks_.clear();
  if (cover_loader_ && cover_loader_->Worker()) {
    cover_loader_->Worker()->Clear();
  }

  cover_fetching_tasks_.clear();
  cover_fetcher_->Clear();
  ui_.fetch->setEnabled(true);
}

void AlbumCoverManager::Reset() {
  if (!backend_)
    return;

  ui_.artists->clear();
  new QListWidgetItem(all_artists_icon_, tr("All artists"), ui_.artists, All_Artists);
  new QListWidgetItem(artist_icon_, tr("Various artists"), ui_.artists, Various_Artists);

  foreach (const QString& artist, backend_->GetAllArtists()) {
    if (artist.isEmpty())
      continue;

    new QListWidgetItem(artist_icon_, artist, ui_.artists, Specific_Artist);
  }
}

void AlbumCoverManager::ArtistChanged(QListWidgetItem* current) {
  if (!backend_ || !cover_loader_->Worker())
    return;
  if (!current)
    return;

  QString artist;
  if (current->type() == Specific_Artist)
    artist = current->text();

  ui_.albums->clear();
  context_menu_items_.clear();
  CancelRequests();

  // Get the list of albums.  How we do it depends on what thing we have
  // selected in the artist list.
  LibraryBackendInterface::AlbumList albums;
  switch (current->type()) {
    case Various_Artists: albums = backend_->GetCompilationAlbums(); break;
    case Specific_Artist: albums = backend_->GetAlbumsByArtist(current->text()); break;
    case All_Artists:
    default:              albums = backend_->GetAllAlbums(); break;
  }

  foreach (const LibraryBackendInterface::Album& info, albums) {
    // Don't show songs without an album, obviously
    if (info.album_name.isEmpty())
      continue;

    QListWidgetItem* item = new QListWidgetItem(no_cover_icon_, info.album_name, ui_.albums);
    item->setData(Role_ArtistName, info.artist);
    item->setData(Role_AlbumName, info.album_name);

    if (!info.art_automatic.isEmpty() || !info.art_manual.isEmpty()) {
      quint64 id = cover_loader_->Worker()->LoadImageAsync(
          info.art_automatic, info.art_manual);
      item->setData(Role_PathAutomatic, info.art_automatic);
      item->setData(Role_PathManual, info.art_manual);
      cover_loading_tasks_[id] = item;
    }
  }

  UpdateFilter();
}

void AlbumCoverManager::CoverImageLoaded(quint64 id, const QImage &image) {
  if (!cover_loading_tasks_.contains(id))
    return;

  QListWidgetItem* item = cover_loading_tasks_.take(id);

  if (image.isNull())
    return;

  item->setIcon(QPixmap::fromImage(image));
  UpdateFilter();
}

void AlbumCoverManager::UpdateFilter() {
  const QString filter = ui_.filter->text().toLower();
  const bool hide_with_covers = filter_without_covers_->isChecked();
  const bool hide_without_covers = filter_with_covers_->isChecked();

  HideCovers hide = Hide_None;
  if (hide_with_covers) {
    hide = Hide_WithCovers;
  } else if (hide_without_covers) {
    hide = Hide_WithoutCovers;
  }

  for (int i = 0; i < ui_.albums->count(); ++i) {
    QListWidgetItem* item = ui_.albums->item(i);
    item->setHidden(ShouldHide(*item, filter, hide));
  }
}

bool AlbumCoverManager::ShouldHide(
    const QListWidgetItem& item, const QString& filter, HideCovers hide) const {
  bool has_cover = item.icon().cacheKey() != no_cover_icon_.cacheKey();
  if (hide == Hide_WithCovers && has_cover) {
    return true;
  } else if (hide == Hide_WithoutCovers && !has_cover) {
    return true;
  }

  if (filter.isEmpty()) {
    return false;
  }

  QStringList query = filter.split(' ');
  foreach (const QString& s, query) {
    if (!item.text().contains(s, Qt::CaseInsensitive)) {
      return true;
    }
  }

  return false;
}

void AlbumCoverManager::FetchAlbumCovers() {
  for (int i=0 ; i<ui_.albums->count() ; ++i) {
    QListWidgetItem* item = ui_.albums->item(i);
    if (item->isHidden())
      continue;
    if (item->icon().cacheKey() != no_cover_icon_.cacheKey())
      continue;

    quint64 id = cover_fetcher_->FetchAlbumCover(
        item->data(Role_ArtistName).toString(), item->data(Role_AlbumName).toString());
    cover_fetching_tasks_[id] = item;
  }

  if (!cover_fetching_tasks_.isEmpty())
    ui_.fetch->setEnabled(false);
}

void AlbumCoverManager::AlbumCoverFetched(quint64 id, const QImage &image) {
  if (!cover_fetching_tasks_.contains(id))
    return;

  QListWidgetItem* item = cover_fetching_tasks_.take(id);
  if (!image.isNull()) {
    const QString artist = item->data(Role_ArtistName).toString();
    const QString album = item->data(Role_AlbumName).toString();

    // Hash the artist and album into a filename for the image
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(artist.toLower().toUtf8().constData());
    hash.addData(album.toLower().toUtf8().constData());

    QString filename = hash.result().toHex() + ".jpg";
    QString path = AlbumCoverLoader::ImageCacheDir() + "/" + filename;

    // Make sure this directory exists first
    QDir dir;
    dir.mkdir(AlbumCoverLoader::ImageCacheDir());

    // Save the image to disk
    image.save(path, "JPG");

    // Save the image in the database
    backend_->UpdateManualAlbumArtAsync(artist, album, path);

    // Update the icon in our list
    quint64 id = cover_loader_->Worker()->LoadImageAsync(QString(), path);
    item->setData(Role_PathManual, path);
    cover_loading_tasks_[id] = item;
  }

  if (cover_fetching_tasks_.isEmpty())
    ui_.fetch->setEnabled(true);
}

bool AlbumCoverManager::event(QEvent* e) {
  if (constructed_) {
    // I think KDE styles override these, and ScrollPerItem really confusing
    // when you have huge items.
    // We seem to have to reset them to sensible values each time the contents
    // of ui_.albums changes.
    ui_.albums->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    ui_.albums->verticalScrollBar()->setSingleStep(20);
  }

  QDialog::event(e);
  return false;
}

bool AlbumCoverManager::eventFilter(QObject *obj, QEvent *event) {
  if (obj == ui_.albums && event->type() == QEvent::ContextMenu) {
    context_menu_items_ = ui_.albums->selectedItems();
    if (context_menu_items_.isEmpty())
      return false;

    bool some_with_covers = false;

    foreach (QListWidgetItem* item, context_menu_items_) {
      if (item->icon().cacheKey() != no_cover_icon_.cacheKey())
        some_with_covers = true;
    }

    ui_.action_show_fullsize->setEnabled(some_with_covers && context_menu_items_.size() == 1);
    ui_.action_choose_manual->setEnabled(context_menu_items_.size() == 1);
    ui_.action_unset_cover->setEnabled(some_with_covers);

    QContextMenuEvent* e = static_cast<QContextMenuEvent*>(event);
    context_menu_->popup(e->globalPos());
    return true;
  }
  return false;
}

void AlbumCoverManager::ShowFullsize() {
  if (context_menu_items_.size() != 1)
    return;
  QListWidgetItem* item = context_menu_items_[0];

  QString title = item->data(Role_AlbumName).toString();
  if (!item->data(Role_ArtistName).toString().isNull())
    title = item->data(Role_ArtistName).toString() + " - " + title;

  QDialog* dialog = new QDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);
  dialog->setWindowTitle(title);

  QLabel* label = new QLabel(dialog);
  label->setPixmap(AlbumCoverLoader::TryLoadPixmap(
      item->data(Role_PathAutomatic).toString(),
      item->data(Role_PathManual).toString()));

  dialog->resize(label->pixmap()->size());
  dialog->show();
}

void AlbumCoverManager::FetchSingleCover() {
  foreach (QListWidgetItem* item, context_menu_items_) {
    quint64 id = cover_fetcher_->FetchAlbumCover(
        item->data(Role_ArtistName).toString(), item->data(Role_AlbumName).toString());
    cover_fetching_tasks_[id] = item;
  }
}

void AlbumCoverManager::ChooseManualCover() {
  if (context_menu_items_.size() != 1)
    return;
  QListWidgetItem* item = context_menu_items_[0];

  QString cover = QFileDialog::getOpenFileName(
      this, tr("Choose manual cover"), item->data(Role_PathAutomatic).toString(),
      tr("Images (*.png *.jpg *.jpeg *.bmp *.gif *.xpm *.pbm *.pgm *.ppm *.xbm *.tiff)") + ";;" +
      tr("All files (*)"));
  if (cover.isNull())
    return;

  // Can we load the image?
  QImage image(cover);
  if (image.isNull())
    return;

  // Update database
  backend_->UpdateManualAlbumArtAsync(item->data(Role_ArtistName).toString(),
                                      item->data(Role_AlbumName).toString(),
                                      cover);

  // Update the icon in our list
  quint64 id = cover_loader_->Worker()->LoadImageAsync(QString(), cover);
  item->setData(Role_PathManual, cover);
  cover_loading_tasks_[id] = item;
}

void AlbumCoverManager::UnsetCover() {
  foreach (QListWidgetItem* item, context_menu_items_) {
    item->setIcon(no_cover_icon_);
    item->setData(Role_PathManual, AlbumCoverLoader::kManuallyUnsetCover);
    backend_->UpdateManualAlbumArtAsync(item->data(Role_ArtistName).toString(),
                                        item->data(Role_AlbumName).toString(),
                                        AlbumCoverLoader::kManuallyUnsetCover);
  }
}
