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

#include "globalsearchtooltip.h"
#include "tooltipresultwidget.h"
#include "core/logging.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QLayoutItem>
#include <QPainter>
#include <QVBoxLayout>

GlobalSearchTooltip::GlobalSearchTooltip(QObject* event_target)
  : QWidget(NULL),
    event_target_(event_target)
{
  setWindowFlags(Qt::Popup);
  setFocusPolicy(Qt::NoFocus);
  setAttribute(Qt::WA_OpaquePaintEvent);
}

void GlobalSearchTooltip::SetResults(const SearchProvider::ResultList& results) {
  results_ = results;

  qDeleteAll(widgets_);
  widgets_.clear();

  // Using a QVBoxLayout here made some weird flickering that I couldn't figure
  // out how to fix, so do layout manually.
  int y = 0;
  int w = 0;

  foreach (const SearchProvider::Result& result, results) {
    QWidget* widget = new TooltipResultWidget(result, this);
    widget->move(0, y);
    widget->show();
    widgets_ << widget;

    QSize size_hint(widget->sizeHint());
    y += size_hint.height();
    w = qMax(w, size_hint.width());
  }

  resize(w, y);
}

void GlobalSearchTooltip::ShowAt(const QPoint& pointing_to) {
  move(pointing_to);

  if (!isVisible())
    show();
}

void GlobalSearchTooltip::keyPressEvent(QKeyEvent* e) {
  // Copy the event to send to the target
  QKeyEvent e2(e->type(), e->key(), e->modifiers(), e->text(),
               e->isAutoRepeat(), e->count());

  qApp->sendEvent(event_target_, &e2);

  e->accept();
}

void GlobalSearchTooltip::paintEvent(QPaintEvent*) {
  QPainter p(this);

  p.fillRect(rect(), palette().base());
}
