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

#ifndef NOWPLAYINGWIDGET_H
#define NOWPLAYINGWIDGET_H

#include "core/backgroundthread.h"
#include "core/song.h"

#include <QWidget>

class AlbumCoverLoader;
class NetworkAccessManager;

class QTextDocument;
class QTimeLine;

class NowPlayingWidget : public QWidget {
  Q_OBJECT

public:
  NowPlayingWidget(QWidget* parent = 0);

  static const int kPadding;

  void set_network(NetworkAccessManager* network) { network_ = network; }
  void set_ideal_height(int height);

  QSize sizeHint() const;

public slots:
  void NowPlaying(const Song& metadata);
  void Stopped();

protected:
  void paintEvent(QPaintEvent* e);

private slots:
  void CoverLoaderInitialised();
  void AlbumArtLoaded(quint64 id, const QImage& image);

  void SetVisible(bool visible);
  void SetHeight(int height);

  void FadePreviousTrack(qreal value);

private:
  void DrawContents(QPainter* p);

private:
  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  NetworkAccessManager* network_;

  bool visible_;
  int ideal_height_;
  QTimeLine* show_hide_animation_;
  QTimeLine* fade_animation_;

  QPixmap no_cover_;

  Song metadata_;
  quint64 load_cover_id_;
  QPixmap cover_;
  QTextDocument* details_;

  QPixmap previous_track_;
  qreal previous_track_opacity_;
};

#endif // NOWPLAYINGWIDGET_H
