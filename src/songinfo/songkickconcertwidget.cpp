/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "songkickconcertwidget.h"

#include <QDate>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QTextDocument>

#include "songinfotextview.h"
#include "ui_songkickconcertwidget.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"

const int SongKickConcertWidget::kStaticMapWidth = 100;
const int SongKickConcertWidget::kStaticMapHeight = 100;

SongKickConcertWidget::SongKickConcertWidget(QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_SongKickConcertWidget),
      network_(new NetworkAccessManager(this)) {
  ui_->setupUi(this);

  // Hide the map by default
  ui_->map->hide();
  ui_->map->setFixedSize(kStaticMapWidth, kStaticMapHeight);
  ui_->map->installEventFilter(this);

  ReloadSettings();
}

SongKickConcertWidget::~SongKickConcertWidget() { delete ui_; }

void SongKickConcertWidget::ReloadSettings() {
  QFont font(SongInfoTextView::Font());
  ui_->title->setFont(font);
  ui_->date->setFont(font);
  ui_->location->setFont(font);
}

void SongKickConcertWidget::Init(const QString& title, const QString& url,
                                 const QString& date, const QString& location) {
  ui_->title->setText(
      QString("<a href=\"%1\">%2</a>").arg(Qt::escape(url), Qt::escape(title)));

  if (!location.isEmpty()) {
    ui_->location->setText(location);
  } else {
    ui_->location->hide();
  }

  if (!date.isEmpty()) {
    QDate parsed_date(QDate::fromString(date, Qt::ISODate));
    QString date_text = Utilities::PrettyFutureDate(parsed_date);

    if (date_text.isEmpty()) {
      date_text = date;
    } else {
      date_text += " (" + date + ")";
    }

    ui_->date->setText(date_text);
  } else {
    ui_->date->hide();
  }
}

void SongKickConcertWidget::SetMap(const QString& lat, const QString& lng,
                                   const QString& venue_name) {
  static const char* kStaticMapUrl =
      "https://maps.googleapis.com/maps/api/staticmap"
      "?key=AIzaSyDDJqmLOeE1mY_EBONhnQmdXbKtasgCtqg"
      "&sensor=false"
      "&size=%1x%2"
      "&zoom=12"
      "&center=%3,%4"
      "&markers=%3,%4";

  ui_->map->show();

  map_url_ = QUrl("https://maps.google.com/");
  map_url_.addQueryItem("ll", QString("%1,%2").arg(lat, lng));
  if (!venue_name.isEmpty()) {
    map_url_.addQueryItem("q", venue_name);
  }

  // Request the static map image
  const QUrl url(QString(kStaticMapUrl).arg(QString::number(kStaticMapWidth),
                                            QString::number(kStaticMapHeight),
                                            lat, lng));
  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this, SLOT(MapLoaded(QNetworkReply*)),
             reply);
}

void SongKickConcertWidget::MapLoaded(QNetworkReply* reply) {
  reply->deleteLater();

  QImage image;
  if (!image.load(reply, "PNG")) {
    qLog(Warning) << "Failed to load static map image" << reply->url();
    return;
  }

  // Scale it if it was the wrong size.
  if (image.width() != kStaticMapWidth || image.height() != kStaticMapHeight) {
    qLog(Warning) << "Scaling static map image" << image.size();
    image = image.scaled(kStaticMapWidth, kStaticMapHeight, Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
  }

  ui_->map->setPixmap(QPixmap::fromImage(image));
}

bool SongKickConcertWidget::eventFilter(QObject* object, QEvent* event) {
  if (object == ui_->map && event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent* e = dynamic_cast<QMouseEvent*>(event);
    if (e->button() == Qt::LeftButton) {
      QDesktopServices::openUrl(map_url_);
      return true;
    }
  }

  return QWidget::eventFilter(object, event);
}
