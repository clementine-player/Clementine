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
#include "core/logging.h"
#include "covers/albumcoverfetcher.h"
#include "covers/albumcoverloader.h"
#include "widgets/groupediconview.h"

#include <QKeyEvent>
#include <QListWidgetItem>
#include <QPainter>
#include <QStandardItemModel>


AlbumCoverSearcher::AlbumCoverSearcher(const QIcon& no_cover_icon, QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_AlbumCoverSearcher),
    model_(new QStandardItemModel(this)),
    no_cover_icon_(no_cover_icon),
    loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    fetcher_(NULL),
    id_(0)
{
  ui_->setupUi(this);
  ui_->busy->hide();

  ui_->covers->set_header_text(tr("Covers from %1"));
  ui_->covers->setModel(model_);

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

  connect(fetcher_, SIGNAL(SearchFinished(quint64,CoverSearchResults)), SLOT(SearchFinished(quint64,CoverSearchResults)));
}

QImage AlbumCoverSearcher::Exec(const QString& artist, const QString& album) {
  ui_->artist->setText(artist);
  ui_->album->setText(album);
  ui_->artist->setFocus();

  if(!artist.isEmpty() || !album.isEmpty()) {
    Search();
  }

  if (exec() == QDialog::Rejected)
    return QImage();

  QModelIndex selected = ui_->covers->currentIndex();
  if (!selected.isValid() || !selected.data(Role_ImageFetchFinished).toBool())
    return QImage();

  QIcon icon = selected.data(Qt::DecorationRole).value<QIcon>();
  if (icon.cacheKey() == no_cover_icon_.cacheKey())
    return QImage();

  return icon.pixmap(icon.availableSizes()[0]).toImage();
}

void AlbumCoverSearcher::Search() {
  ui_->busy->show();
  ui_->search->setEnabled(false);
  ui_->artist->setEnabled(false);
  ui_->album->setEnabled(false);
  ui_->covers->setEnabled(false);

  model_->clear();
  cover_loading_tasks_.clear();

  id_ = fetcher_->SearchForCovers(ui_->artist->text(), ui_->album->text());
}

void AlbumCoverSearcher::SearchFinished(quint64 id, const CoverSearchResults& results) {
  if (id != id_)
    return;

  ui_->search->setEnabled(true);
  ui_->artist->setEnabled(true);
  ui_->album->setEnabled(true);
  ui_->covers->setEnabled(true);
  id_ = 0;

  foreach (const CoverSearchResult& result, results) {
    if (result.image_url.isEmpty())
      continue;

    quint64 id = loader_->Worker()->LoadImageAsync(result.image_url, QString());

    QStandardItem* item = new QStandardItem;
    item->setIcon(no_cover_icon_);
    item->setText(result.description);
    item->setData(result.image_url, Role_ImageURL);
    item->setData(id, Role_ImageRequestId);
    item->setData(false, Role_ImageFetchFinished);
    item->setData(QVariant(Qt::AlignTop | Qt::AlignHCenter), Qt::TextAlignmentRole);
    item->setData(result.category, GroupedIconView::Role_Group);

    model_->appendRow(item);

    cover_loading_tasks_[id] = item;
  }

  if (cover_loading_tasks_.isEmpty())
    ui_->busy->hide();
}

void AlbumCoverSearcher::ImageLoaded(quint64 id, const QImage& image) {
  if (!cover_loading_tasks_.contains(id))
    return;

  QIcon icon(QPixmap::fromImage(image));

  // Create a pixmap that's padded and exactly the right size for the icon.
  QImage scaled_image(image.scaled(ui_->covers->iconSize(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation));

  QImage padded_image(ui_->covers->iconSize(), QImage::Format_ARGB32_Premultiplied);
  padded_image.fill(0);

  QPainter p(&padded_image);
  p.drawImage((padded_image.width() - scaled_image.width()) / 2,
              (padded_image.height() - scaled_image.height()) / 2,
              scaled_image);
  p.end();

  icon.addPixmap(QPixmap::fromImage(padded_image));

  QStandardItem* item = cover_loading_tasks_.take(id);
  item->setData(true, Role_ImageFetchFinished);
  item->setIcon(icon);
  item->setToolTip(item->text() + " (" + QString::number(image.width()) + "x" +
                                         QString::number(image.height()) + ")");

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
