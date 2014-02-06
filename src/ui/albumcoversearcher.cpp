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
#include "core/application.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "covers/albumcoverfetcher.h"
#include "covers/albumcoverloader.h"
#include "widgets/forcescrollperpixel.h"
#include "widgets/groupediconview.h"

#include <QKeyEvent>
#include <QListWidgetItem>
#include <QPainter>
#include <QStandardItemModel>

const int SizeOverlayDelegate::kMargin = 4;
const int SizeOverlayDelegate::kPaddingX = 3;
const int SizeOverlayDelegate::kPaddingY = 1;
const qreal SizeOverlayDelegate::kBorder = 5.0;
const qreal SizeOverlayDelegate::kFontPointSize = 7.5;
const int SizeOverlayDelegate::kBorderAlpha = 200;
const int SizeOverlayDelegate::kBackgroundAlpha = 175;


SizeOverlayDelegate::SizeOverlayDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

void SizeOverlayDelegate::paint(QPainter* painter,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index) const {
  QStyledItemDelegate::paint(painter, option, index);

  if (!index.data(AlbumCoverSearcher::Role_ImageFetchFinished).toBool()) {
    return;
  }

  const QSize size = index.data(AlbumCoverSearcher::Role_ImageSize).toSize();
  const QString text = Utilities::PrettySize(size);

  QFont font(option.font);
  font.setPointSizeF(kFontPointSize);
  font.setBold(true);

  const QFontMetrics metrics(font);

  const int text_width = metrics.width(text);

  const QRect icon_rect(
      option.rect.left(), option.rect.top(),
      option.rect.width(), option.rect.width());

  const QRect background_rect(
      icon_rect.right() - kMargin - text_width - kPaddingX*2,
      icon_rect.bottom() - kMargin - metrics.height() - kPaddingY*2,
      text_width + kPaddingX*2, metrics.height() + kPaddingY*2);
  const QRect text_rect(
      background_rect.left() + kPaddingX, background_rect.top() + kPaddingY,
      text_width, metrics.height());

  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(QColor(0, 0, 0, kBorderAlpha));
  painter->setBrush(QColor(0, 0, 0, kBackgroundAlpha));
  painter->drawRoundedRect(background_rect, kBorder, kBorder);

  painter->setPen(Qt::white);
  painter->setFont(font);
  painter->drawText(text_rect, text);
  painter->restore();
}


AlbumCoverSearcher::AlbumCoverSearcher(const QIcon& no_cover_icon,
                                       Application* app, QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_AlbumCoverSearcher),
    app_(app),
    model_(new QStandardItemModel(this)),
    no_cover_icon_(no_cover_icon),
    fetcher_(nullptr),
    id_(0)
{
  setWindowModality(Qt::WindowModal);
  ui_->setupUi(this);
  ui_->busy->hide();

  ui_->covers->set_header_text(tr("Covers from %1"));
  ui_->covers->AddSortSpec(Role_ImageDimensions, Qt::DescendingOrder);
  ui_->covers->setItemDelegate(new SizeOverlayDelegate(this));
  ui_->covers->setModel(model_);

  options_.scale_output_image_ = false;
  options_.pad_output_image_ = false;

  connect(app_->album_cover_loader(),
          SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(ImageLoaded(quint64,QImage)));

  connect(ui_->search, SIGNAL(clicked()), SLOT(Search()));
  connect(ui_->covers, SIGNAL(doubleClicked(QModelIndex)), SLOT(CoverDoubleClicked(QModelIndex)));

  new ForceScrollPerPixel(ui_->covers, this);

  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setShortcut(QKeySequence::Close);
}

AlbumCoverSearcher::~AlbumCoverSearcher() {
  delete ui_;
}

void AlbumCoverSearcher::Init(AlbumCoverFetcher* fetcher) {
  fetcher_ = fetcher;

  connect(fetcher_, SIGNAL(SearchFinished(quint64,CoverSearchResults,CoverSearchStatistics)),
          SLOT(SearchFinished(quint64,CoverSearchResults)));
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
  model_->clear();
  cover_loading_tasks_.clear();

  if (ui_->album->isEnabled()) {
    id_ = fetcher_->SearchForCovers(ui_->artist->text(), ui_->album->text());
    ui_->search->setText(tr("Abort"));
    ui_->busy->show();
    ui_->artist->setEnabled(false);
    ui_->album->setEnabled(false);
    ui_->covers->setEnabled(false);
  } else {
    fetcher_->Clear();
    ui_->search->setText(tr("Search"));
    ui_->busy->hide();
    ui_->search->setEnabled(true);
    ui_->artist->setEnabled(true);
    ui_->album->setEnabled(true);
    ui_->covers->setEnabled(true);
  }
}

void AlbumCoverSearcher::SearchFinished(quint64 id, const CoverSearchResults& results) {
  if (id != id_)
    return;

  ui_->search->setEnabled(true);
  ui_->artist->setEnabled(true);
  ui_->album->setEnabled(true);
  ui_->covers->setEnabled(true);
  ui_->search->setText(tr("Search"));
  id_ = 0;

  foreach (const CoverSearchResult& result, results) {
    if (result.image_url.isEmpty())
      continue;

    quint64 id = app_->album_cover_loader()->LoadImageAsync(
          options_, result.image_url.toString(), QString());

    QStandardItem* item = new QStandardItem;
    item->setIcon(no_cover_icon_);
    item->setText(result.description);
    item->setData(result.image_url, Role_ImageURL);
    item->setData(id, Role_ImageRequestId);
    item->setData(false, Role_ImageFetchFinished);
    item->setData(QVariant(Qt::AlignTop | Qt::AlignHCenter), Qt::TextAlignmentRole);
    item->setData(result.provider, GroupedIconView::Role_Group);

    model_->appendRow(item);

    cover_loading_tasks_[id] = item;
  }

  if (cover_loading_tasks_.isEmpty())
    ui_->busy->hide();
}

void AlbumCoverSearcher::ImageLoaded(quint64 id, const QImage& image) {
  if (!cover_loading_tasks_.contains(id))
    return;
  QStandardItem* item = cover_loading_tasks_.take(id);

  if (cover_loading_tasks_.isEmpty())
    ui_->busy->hide();

  if (image.isNull()) {
    model_->removeRow(item->row());
    return;
  }

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

  item->setData(true, Role_ImageFetchFinished);
  item->setData(image.width() * image.height(), Role_ImageDimensions);
  item->setData(image.size(), Role_ImageSize);
  item->setIcon(icon);
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
