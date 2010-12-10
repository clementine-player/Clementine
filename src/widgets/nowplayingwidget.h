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

#ifndef NOWPLAYINGWIDGET_H
#define NOWPLAYINGWIDGET_H

#include "core/backgroundthread.h"
#include "core/song.h"

#include <QWidget>

class AlbumCoverLoader;

class QActionGroup;
class QMenu;
class QMovie;
class QSignalMapper;
class QTextDocument;
class QTimeLine;

class NowPlayingWidget : public QWidget {
  Q_OBJECT

public:
  NowPlayingWidget(QWidget* parent = 0);

  static const char* kSettingsGroup;
  static const int kPadding;
  static const int kGradientHead;
  static const int kGradientTail;
  static const int kMaxCoverSize;
  static const int kBottomOffset;
  static const int kTopBorder;

  // Values are saved in QSettings
  enum Mode {
    SmallSongDetails = 0,
    LargeSongDetails = 1,
  };

  void set_ideal_height(int height);

  bool show_above_status_bar() const;

  QSize sizeHint() const;

signals:
  void ShowAboveStatusBarChanged(bool above);

public slots:
  void NowPlaying(const Song& metadata);
  void Stopped();
  void AllHail(bool hypnotoad);
  void EnableKittens(bool aww);

protected:
  void paintEvent(QPaintEvent* e);
  void resizeEvent(QResizeEvent*);
  void contextMenuEvent(QContextMenuEvent* e);

private slots:
  void SetMode(int mode);
  void ShowAboveStatusBar(bool above);

  void CoverLoaderInitialised();
  void AlbumArtLoaded(quint64 id, const QImage& image);

  void SetVisible(bool visible);
  void SetHeight(int height);

  void FadePreviousTrack(qreal value);

private:
  void CreateModeAction(Mode mode, const QString& text, QActionGroup* group,
                        QSignalMapper* mapper);
  void UpdateDetailsText();
  void UpdateHeight(AlbumCoverLoader* loader);
  void DrawContents(QPainter* p);

private:
  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  BackgroundThread<AlbumCoverLoader>* kitten_loader_;

  Mode mode_;

  QMenu* menu_;
  QAction* above_statusbar_action_;

  bool visible_;
  int small_ideal_height_;
  int cover_height_;
  int total_height_;
  QTimeLine* show_hide_animation_;
  QTimeLine* fade_animation_;

  // Information about the current track
  Song metadata_;
  quint64 load_cover_id_;
  QPixmap cover_;
  QTextDocument* details_;

  // Holds the last track while we're fading to the new track
  QPixmap previous_track_;
  qreal previous_track_opacity_;

  static const char* kHypnotoadPath;
  QMovie* hypnotoad_;

  bool aww_;
};

#endif // NOWPLAYINGWIDGET_H
