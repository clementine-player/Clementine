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
#include "widgets/kcategorizedsortfilterproxymodel.h"

#include <QKeyEvent>
#include <QListWidgetItem>
#include <QPainter>
#include <QStandardItemModel>


const int AlbumCoverCategoryDrawer::kBarThickness = 2;
const int AlbumCoverCategoryDrawer::kBarMarginTop = 3;
const int AlbumCoverCategoryDrawer::kBarMarginBottom = 10;


AlbumCoverCategoryDrawer::AlbumCoverCategoryDrawer(KCategorizedView* view)
    : KCategoryDrawerV3(view),
      total_height_(view->fontMetrics().height() +
        kBarMarginTop + kBarThickness + kBarMarginBottom) {
  setLeftMargin(kBarMarginBottom);
}

int AlbumCoverCategoryDrawer::categoryHeight(const QModelIndex&,
                                             const QStyleOption&) const {
  return total_height_;
}

void AlbumCoverCategoryDrawer::drawCategory(const QModelIndex& index, int,
                                            const QStyleOption& option,
                                            QPainter* painter) const {
  painter->save();

  // Bold font
  QFont font(view()->font());
  font.setBold(true);
  QFontMetrics metrics(font);

  // Draw text
  const QString category = tr("Covers from %1").arg(
      index.data(KCategorizedSortFilterProxyModel::CategoryDisplayRole).toString());
  painter->setFont(font);
  painter->drawText(option.rect, category);

  // Draw a line underneath
  const QPoint start(option.rect.left(),
                     option.rect.top() + metrics.height() + kBarMarginTop);
  const QPoint end(option.rect.right(), start.y());

  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(option.palette.color(QPalette::Disabled, QPalette::Text),
                       kBarThickness, Qt::SolidLine, Qt::RoundCap));
  painter->setOpacity(0.5);
  painter->drawLine(start, end);

  painter->restore();
}


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

  KCategorizedSortFilterProxyModel* proxy = new KCategorizedSortFilterProxyModel(this);
  proxy->setCategorizedModel(true);
  proxy->setSourceModel(model_);
  ui_->covers->setModel(proxy);
  ui_->covers->setCategoryDrawer(new AlbumCoverCategoryDrawer(ui_->covers));

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

QImage AlbumCoverSearcher::Exec(const QString &query) {
  ui_->query->setText(query);
  ui_->query->setFocus();

  if(!query.isEmpty()) {
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
  ui_->query->setEnabled(false);
  ui_->covers->setEnabled(false);

  model_->clear();
  cover_loading_tasks_.clear();

  id_ = fetcher_->SearchForCovers(ui_->query->text());
}

void AlbumCoverSearcher::SearchFinished(quint64 id, const CoverSearchResults& results) {
  if (id != id_)
    return;

  ui_->search->setEnabled(true);
  ui_->query->setEnabled(true);
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
    item->setData(result.category, KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(result.category, KCategorizedSortFilterProxyModel::CategorySortRole);

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

  // Add an icon that's the right size for the view
  icon.addPixmap(QPixmap::fromImage(image.scaled(ui_->covers->iconSize(),
      Qt::KeepAspectRatio, Qt::SmoothTransformation)));

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
