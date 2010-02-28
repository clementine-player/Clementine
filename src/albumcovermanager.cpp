#include "albumcovermanager.h"
#include "librarybackend.h"
#include "libraryquery.h"

#include <QSettings>
#include <QPainter>
#include <QMenu>
#include <QActionGroup>

const char* AlbumCoverManager::kSettingsGroup = "CoverManager";

AlbumCoverManager::AlbumCoverManager(QWidget *parent)
  : QDialog(parent),
    cover_loader_(new BackgroundThread<AlbumCoverLoader>(this)),
    artist_icon_(":/artist.png"),
    all_artists_icon_(":/album.png")
{
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

  // View menu
  QActionGroup* filter_group = new QActionGroup(this);
  filter_all_ = filter_group->addAction("All albums");
  filter_with_covers_ = filter_group->addAction("Albums with covers");
  filter_without_covers_ = filter_group->addAction("Albums without covers");
  filter_all_->setCheckable(true);
  filter_with_covers_->setCheckable(true);
  filter_without_covers_->setCheckable(true);
  filter_group->setExclusive(true);
  filter_all_->setChecked(true);

  QMenu* view_menu = new QMenu(this);
  view_menu->addActions(filter_group->actions());

  ui_.view->setMenu(view_menu);

  // Connections
  connect(cover_loader_, SIGNAL(Initialised()), SLOT(CoverLoaderInitialised()));
  connect(ui_.artists, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          SLOT(ArtistChanged(QListWidgetItem*)));
  connect(ui_.filter, SIGNAL(textChanged(QString)), SLOT(UpdateFilter()));
  connect(filter_group, SIGNAL(triggered(QAction*)), SLOT(UpdateFilter()));
  connect(ui_.view, SIGNAL(clicked()), ui_.view, SLOT(showMenu()));

  // Restore settings
  QSettings s;
  s.beginGroup(kSettingsGroup);

  restoreGeometry(s.value("geometry").toByteArray());
  if (!ui_.splitter->restoreState(s.value("splitter_state").toByteArray())) {
    // Sensible default size for the artists view
    ui_.splitter->setSizes(QList<int>() << 200 << width() - 200);
  }

  cover_loader_->start();
}

void AlbumCoverManager::CoverLoaderInitialised() {
  connect(cover_loader_->Worker().get(), SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(CoverImageLoaded(quint64,QImage)));
}

void AlbumCoverManager::SetBackend(boost::shared_ptr<LibraryBackend> backend) {
  backend_ = backend;

  if (isVisible())
    Reset();
}

void AlbumCoverManager::showEvent(QShowEvent *) {
  Reset();
}

void AlbumCoverManager::closeEvent(QCloseEvent *) {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("geometry", saveGeometry());
  s.setValue("splitter_state", ui_.splitter->saveState());
}

void AlbumCoverManager::Reset() {
  if (!backend_)
    return;

  ui_.artists->clear();
  new QListWidgetItem(all_artists_icon_, "All artists", ui_.artists, All_Artists);

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
  cover_loading_tasks_.clear();
  cover_loader_->Worker()->Clear();

  foreach (const LibraryBackend::AlbumArtInfo& info, backend_->GetAlbumArtInfo(artist)) {
    QListWidgetItem* item = new QListWidgetItem(no_cover_icon_, info.album_name, ui_.albums);

    if (!info.art_automatic.isEmpty() || !info.art_manual.isEmpty()) {
      quint64 id = cover_loader_->Worker()->LoadImageAsync(
          info.art_automatic, info.art_manual);
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

  for (int i=0 ; i<ui_.albums->count() ; ++i) {
    QListWidgetItem* item = ui_.albums->item(i);
    QString text = item->text();
    bool has_cover = item->icon().cacheKey() != no_cover_icon_.cacheKey();

    item->setHidden((!filter.isEmpty() && !text.toLower().contains(filter)) ||
                    (has_cover && hide_with_covers) ||
                    (!has_cover && hide_without_covers));
  }
}
