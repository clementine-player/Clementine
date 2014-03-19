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

#include "busyindicator.h"

#include <QHBoxLayout>
#include <QMovie>

BusyIndicator::BusyIndicator(const QString& text, QWidget* parent)
    : QWidget(parent) {
  Init(text);
}

BusyIndicator::BusyIndicator(QWidget* parent) : QWidget(parent) {
  Init(QString::null);
}

void BusyIndicator::Init(const QString& text) {
  movie_ = new QMovie(":spinner.gif"), label_ = new QLabel;

  QLabel* icon = new QLabel;
  icon->setMovie(movie_);
  icon->setMinimumSize(16, 16);

  label_->setWordWrap(true);
  label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(icon);
  layout->addSpacing(6);
  layout->addWidget(label_);

  set_text(text);
}

BusyIndicator::~BusyIndicator() { delete movie_; }

void BusyIndicator::showEvent(QShowEvent*) { movie_->start(); }

void BusyIndicator::hideEvent(QHideEvent*) { movie_->stop(); }

void BusyIndicator::set_text(const QString& text) {
  label_->setText(text);
  label_->setVisible(!text.isEmpty());
}

QString BusyIndicator::text() const { return label_->text(); }
