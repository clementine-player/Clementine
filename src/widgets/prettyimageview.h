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

#ifndef PRETTYIMAGEVIEW_H
#define PRETTYIMAGEVIEW_H

#include <QMap>
#include <QUrl>
#include <QWidget>

class NetworkAccessManager;

class QNetworkReply;
class QTimeLine;

class PrettyImageView : public QWidget {
  Q_OBJECT

public:
  PrettyImageView(NetworkAccessManager* network, QWidget* parent = 0);

  static const int kTotalHeight;
  static const int kImageHeight;
  static const int kBorderHeight;
  static const int kArrowWidth;

  static const int kBaseAnimationDuration;
  static const int kArrowAnimationDuration;

public slots:
  void Clear();
  void AddImage(const QUrl& url);

protected:
  void paintEvent(QPaintEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);

private:
  struct Image {
    Image(const QUrl& url) : loading_(false), url_(url) {}

    void SetImage(const QImage& image);

    bool loading_;
    QUrl url_;
    QImage image_;
    QPixmap thumbnail_;
  };

  QRect left() const;
  QRect right() const;
  QPolygon arrow(const QRect& rect, int direction) const;

  void SetTimeLineActive(QTimeLine* timeline, bool active);

  void DrawImage(QPainter* p, const QRect& rect, const Image& image);
  void DrawThumbnail(QPainter* p, const QRect& rect, const Image& image);

private slots:
  void ImageFetched(quint64 id, QNetworkReply* reply);

private:
  NetworkAccessManager* network_;

  QMap<quint64, int> image_requests_;
  quint64 next_image_request_id_;

  QList<Image> images_;
  int current_index_;

  QTimeLine* base_timeline_;
  QTimeLine* left_timeline_;
  QTimeLine* right_timeline_;
};

#endif // PRETTYIMAGEVIEW_H
