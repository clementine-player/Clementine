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

#ifndef PRETTYIMAGE_H
#define PRETTYIMAGE_H

#include <QUrl>
#include <QWidget>

class QMenu;
class QNetworkAccessManager;
class QNetworkReply;

class PrettyImage : public QWidget {
  Q_OBJECT

 public:
  PrettyImage(const QUrl& url, QNetworkAccessManager* network,
              QWidget* parent = nullptr);

  static const int kTotalHeight;
  static const int kReflectionHeight;
  static const int kImageHeight;

  static const int kMaxImageWidth;

  static const char* kSettingsGroup;

  QSize sizeHint() const;
  QSize image_size() const;

signals:
  void Loaded();

 public slots:
  void LazyLoad();
  void SaveAs();
  void ShowFullsize();

 protected:
  void contextMenuEvent(QContextMenuEvent*);
  void paintEvent(QPaintEvent*);

 private slots:
  void ImageFetched();
  void ImageScaled();

 private:
  enum State {
    State_WaitingForLazyLoad,
    State_Fetching,
    State_CreatingThumbnail,
    State_Finished,
  };

  void DrawThumbnail(QPainter* p, const QRect& rect);

 private:
  QNetworkAccessManager* network_;
  State state_;
  QUrl url_;

  QImage image_;
  QPixmap thumbnail_;

  QMenu* menu_;
  QString last_save_dir_;
};

#endif  // PRETTYIMAGE_H
