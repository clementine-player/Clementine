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

#ifndef SONGKICKCONCERTWIDGET_H
#define SONGKICKCONCERTWIDGET_H

#include <QUrl>
#include <QWidget>

class Ui_SongKickConcertWidget;

class QNetworkAccessManager;
class QNetworkReply;

class SongKickConcertWidget : public QWidget {
  Q_OBJECT

 public:
  SongKickConcertWidget(QWidget* parent = 0);
  ~SongKickConcertWidget();

  static const int kStaticMapWidth;
  static const int kStaticMapHeight;

  void Init(const QString& title, const QString& url, const QString& date,
            const QString& location);
  void SetMap(const QString& lat, const QString& lng,
              const QString& venue_name);

  // QObject
  bool eventFilter(QObject* object, QEvent* event);

 public slots:
  void ReloadSettings();

 private slots:
  void MapLoaded(QNetworkReply* reply);

 private:
  Ui_SongKickConcertWidget* ui_;
  QNetworkAccessManager* network_;

  QUrl map_url_;
};

#endif  // SONGKICKCONCERTWIDGET_H
