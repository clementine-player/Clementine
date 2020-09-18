/* This file is part of Clementine.
   Copyright 2013, David Sansome <me@davidsansome.com>

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

#include "favoritewidget.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QSize>
#include <QStyle>
#include <QStylePainter>

#include "core/logging.h"
#include "ui/iconloader.h"

const int FavoriteWidget::kStarSize = 16;

FavoriteWidget::FavoriteWidget(int tab_index, bool favorite, QWidget* parent)
    : QWidget(parent), tab_index_(tab_index), favorite_(favorite) {
  QIcon star_on = IconLoader::Load("star-on", IconLoader::Other);
  on_ = star_on.pixmap(star_on.availableSizes().last());
  QIcon star_off = IconLoader::Load("star-off", IconLoader::Other);
  off_ = star_off.pixmap(star_off.availableSizes().last());
}

void FavoriteWidget::SetFavorite(bool favorite) {
  if (favorite_ != favorite) {
    favorite_ = favorite;
    update();
    emit FavoriteStateChanged(tab_index_, favorite_);
  }
}

QSize FavoriteWidget::sizeHint() const {
  const int frame_width =
      1 + style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  return QSize(kStarSize + frame_width * 2, kStarSize + frame_width * 2);
}

void FavoriteWidget::paintEvent(QPaintEvent* e) {
  QStylePainter p(this);

  if (favorite_) {
    p.drawItemPixmap(rect_, Qt::AlignVCenter, on_);
  } else {
    p.drawItemPixmap(rect_, Qt::AlignVCenter, off_);
  }
}

void FavoriteWidget::resizeEvent(QResizeEvent* e) {
  // ignore text descent when vertically centering
  QFontMetrics fontMetrics = QFontMetrics(this->font());
  rect_ = this->rect();
  rect_.setBottom(rect_.bottom() - fontMetrics.descent());
}

void FavoriteWidget::mouseReleaseEvent(QMouseEvent* e) {
  favorite_ = !favorite_;
  update();
  emit FavoriteStateChanged(tab_index_, favorite_);
}
