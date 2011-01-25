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

#include "albumcoversearcher.h"
#include "ui_albumcoversearcher.h"
#include "core/albumcoverfetcher.h"
#include "core/albumcoverloader.h"

#include <QKeyEvent>
#include <QListWidgetItem>

AlbumCoverSearcher::AlbumCoverSearcher(const QIcon& no_cover_icon, QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_AlbumCoverSearcher),
    no_cover_icon_(no_cover_icon),
    loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    fetcher_(NULL),
    id_(0)
{
  ui_->setupUi(this);
  ui_->busy->hide();

  loader_->Start(true);
  loader_->Worker()->SetDefaultOutputImage(QImage(":nocover.png"));
  loader_->Worker()->SetScaleOutputImage(false);
  loader_->Worker()->SetPadOutputImage(false);
  connect(loader_->Worker().get(), SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(ImageLoaded(quint64,QImage)));

  connect(ui_->search, SIGNAL(clicked()), SLOT(Search()));
  connect(ui_->covers, SIGNAL(doubleClicked(QModelIndex)), SLOT(CoverDoubleClicked(QModelIndex)));
}

AlbumCoverSearcher::~AlbumCoverSearcher() {
  delete ui_;
}

void AlbumCoverSearcher::Init(AlbumCoverFetcher* fetcher) {
  fetcher_ = fetcher;

  connect(fetcher_, SIGNAL(SearchFinished(quint64,AlbumCoverFetcher::SearchResults)), SLOT(SearchFinished(quint64,AlbumCoverFetcher::SearchResults)));
}

QImage AlbumCoverSearcher::Exec(const QString &query) {
  ui_->query->setText(query);
  ui_->query->setFocus();

  if(!query.isEmpty()) {
    Search();
  }

  if (exec() == QDialog::Rejected)
    return QImage();
  if (!ui_->covers->currentItem())
    return QImage();

  QIcon icon = ui_->covers->currentItem()->icon();
  if (icon.cacheKey() == no_cover_icon_.cacheKey())
    return QImage();

  return icon.pixmap(icon.availableSizes()[0]).toImage();
}

void AlbumCoverSearcher::Search() {
  ui_->busy->show();
  ui_->search->setEnabled(false);
  ui_->query->setEnabled(false);
  ui_->covers->setEnabled(false);

  id_ = fetcher_->SearchForCovers(ui_->query->text());
}

void AlbumCoverSearcher::SearchFinished(quint64 id, const AlbumCoverFetcher::SearchResults &results) {
  if (id != id_)
    return;

  ui_->search->setEnabled(true);
  ui_->query->setEnabled(true);
  ui_->covers->setEnabled(true);
  id_ = 0;

  ui_->covers->clear();
  cover_loading_tasks_.clear();
  foreach (const AlbumCoverFetcher::SearchResult& result, results) {
    if (result.image_url.isEmpty())
      continue;

    quint64 id = loader_->Worker()->LoadImageAsync(result.image_url, QString());

    QListWidgetItem* item = new QListWidgetItem(ui_->covers);
    item->setIcon(no_cover_icon_);
    item->setText(result.artist + " - " + result.album);
    item->setData(Role_ImageURL, result.image_url);
    item->setData(Role_ImageRequestId, id);
    item->setData(Qt::TextAlignmentRole, QVariant(Qt::AlignTop | Qt::AlignHCenter));

    cover_loading_tasks_[id] = item;
  }

  if (cover_loading_tasks_.isEmpty())
    ui_->busy->hide();
}

void AlbumCoverSearcher::ImageLoaded(quint64 id, const QImage& image) {
  if (!cover_loading_tasks_.contains(id))
    return;

  QIcon icon(QPixmap::fromImage(image));

  // Add an icon that's the right size for the view
  icon.addPixmap(QPixmap::fromImage(image.scaled(ui_->covers->iconSize(),
      Qt::KeepAspectRatio, Qt::SmoothTransformation)));

  QListWidgetItem* item = cover_loading_tasks_.take(id);
  item->setIcon(icon);

  if (cover_loading_tasks_.isEmpty())
    ui_->busy->hide();
}

void AlbumCoverSearcher::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Enter ||
      e->key() == Qt::Key_Return) {
    e->ignore();
    return;
  }

  QDialog::keyPressEvent(e);
}

void AlbumCoverSearcher::CoverDoubleClicked(const QModelIndex &index) {
  if (index.isValid())
    accept();
}
