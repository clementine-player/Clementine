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

#include "widgetfadehelper.h"

#include <QPainter>
#include <QTimeLine>
#include <QtDebug>

WidgetFadeHelper::WidgetFadeHelper(QWidget* parent, int msec)
  : QWidget(parent),
    parent_(parent),
    timeline_(new QTimeLine(msec, this))
{
  setAttribute(Qt::WA_TransparentForMouseEvents);

  connect(timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
  connect(timeline_, SIGNAL(finished()), SLOT(hide()));

  hide();
}

void WidgetFadeHelper::Start() {
  // Take a "screenshot" of the window
  pixmap_ = QPixmap::grabWidget(parent_);

  // Cover the parent
  resize(parent_->size());
  raise();
  show();

  // Start the timeline
  timeline_->start();
}

void WidgetFadeHelper::paintEvent(QPaintEvent* ) {
  QPainter p(this);

  p.setOpacity(1.0 - timeline_->currentValue());
  p.drawPixmap(0, 0, pixmap_);
}
